/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.alexaautoclientservice;

import static com.amazon.alexaautoclientservice.util.FileUtil.isDefaultImplementationEnabled;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.ContentProviderClient;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.amazon.aace.aasb.AASBStream;
import com.amazon.aace.core.Engine;
import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.core.config.EngineConfiguration;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsipc.AACSReceiver;
import com.amazon.aacsipc.AACSReceiver.FetchStreamCallback;
import com.amazon.aacsipc.AACSReceiver.MessageReceivedCallback;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexaautoclientservice.aacs_extra.AACSContext;
import com.amazon.alexaautoclientservice.aacs_extra.AACSModuleFactoryInterface;
import com.amazon.alexaautoclientservice.constants.LVCServiceConstants;
import com.amazon.alexaautoclientservice.modules.externalMediaPlayer.MACCPlayer;
import com.amazon.alexaautoclientservice.modules.locationProvider.LocationProviderHandler;
import com.amazon.alexaautoclientservice.modules.networkInfoProvider.NetworkInfoProviderHandler;
import com.amazon.alexaautoclientservice.modules.propertyManager.PropertyManagerHandler;
import com.amazon.alexaautoclientservice.receiver.LVCReceiver;
import com.amazon.alexaautoclientservice.receiver.PingReceiver;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AlexaAutoClientService extends Service implements AACSContext {
    private static final String TAG = AACSConstants.AACS + "-" + AlexaAutoClientService.class.getSimpleName();

    // AACS Version Info
    private static final double MIN_SUPPORTED_VERSION = 1.0;
    private static final double CURRENT_VERSION = 1.1;

    private static final int THREAD_JOIN_TIMEOUT_IN_MILLIS = 2000;
    private static final int FETCH_READ_BUFFER_CHUNK_SIZE = 4096;
    private static final int PUSH_WRITE_BUFFER_CHUNK_SIZE = 300;
    private static final int PUSH_WRITE_BUFFER_MIN_CHUNK_SIZE = 20;
    private static final String AACS_CHANNEL_ID = "com.amazon.alexaautoclientservice";
    private static final String AACS_CHANNEL_NAME = "AACS";
    private static final int AACS_SERVICE_STARTED_NOTIFICATION_ID = 1;

    private Context mContext = this;
    private AASBHandler mAASBHandler;
    private NetworkInfoProviderHandler mNetworkInfoProviderHandler;
    private Engine mEngine;
    private AACSStateMachine mStateMachine;
    private HashMap<String, String> mAACSContextMap;

    private HandlerThread mEngineHandlerThread;
    private Handler mEngineHandler;

    private AACSReceiver mAACSReceiver;
    private ContentProviderClient mAPCP;
    private ExecutorService mIPCStreamReadExecutor = Executors.newFixedThreadPool(2);
    private ExecutorService mIPCStreamWriteExecutor = Executors.newFixedThreadPool(2);
    private LVCInteractionProvider mLVCInteractionProvider;
    private LVCReceiver mLVCConfigReceiver;
    private PingReceiver mPingReceiver;
    private PropertyManagerHandler mPropertyManagerHandler;
    private MACCPlayer mMACCPlayer;
    private ConcurrentHashMap<String, ParcelFileDescriptor.AutoCloseOutputStream> mOutputStreamMap =
            new ConcurrentHashMap<>();
    private Long mOnCreateTimeMs;

    public AlexaAutoClientService() {
        mAASBHandler = null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mOnCreateTimeMs = System.currentTimeMillis();
        Log.i(TAG, "AlexaAutoClientService created");

        initPingReceiver();

        mStateMachine = new AACSStateMachine();
        mStateMachine.setState(State.STARTED);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand, " + intent);

        if (mStateMachine.getState() == State.STARTED && intent != null) {
            if (intent.hasExtra(AACSConstants.NEW_CONFIG)) {
                if (!intent.getBooleanExtra(AACSConstants.NEW_CONFIG, true) && FileUtil.isConfigurationSaved(this)) {
                    if (FileUtil.lvcEnabled()) {
                        mStateMachine.setState(State.WAIT_FOR_LVC_CONFIG);
                        Log.d(TAG,
                                "Setting AACSState to WAIT_FOR_LVC_CONFIG since the NEW_CONFIG parameter is false and LVC is enabled.");
                    } else {
                        mStateMachine.setState(State.CONFIGURED);
                        Log.d(TAG, "Setting AACSState to CONFIGURED since the NEW_CONFIG parameter is false.");
                    }
                } else {
                    Log.d(TAG, "Waiting for AACS Configuration message.");
                }
            }

            if (intent.getAction().equals(Intent.ACTION_SEND_MULTIPLE)) {
                if (intent.hasExtra(Intent.EXTRA_STREAM)) {
                    ArrayList<Uri> fileUris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
                    String module = "";
                    if (intent.hasExtra(AACSConstants.CONFIG_MODULE)) {
                        module = intent.getStringExtra(AACSConstants.CONFIG_MODULE);
                    } else {
                        Log.w(TAG,
                                String.format(
                                        "%s Intent is missing a configModule extra.", Intent.ACTION_SEND_MULTIPLE));
                    }
                    for (Uri fileUri : fileUris) {
                        if (fileUri != null) {
                            Log.d(TAG,
                                    String.format(
                                            "Acquired URI (%s) permission. Copying to AACS.", fileUri.toString()));
                            FileUtil.copyExternalFileToAACS(this, fileUri, module, fileUri.getLastPathSegment());
                        } else {
                            Log.w(TAG, String.format("%s Intent contains null URI.", Intent.ACTION_SEND_MULTIPLE));
                        }
                    }
                } else {
                    Log.w(TAG,
                            String.format("%s intent is missing %s extra containing URIs.", Intent.ACTION_SEND_MULTIPLE,
                                    Intent.EXTRA_STREAM));
                }
            }
        }

        if (intent != null && intent.hasExtra(AACSConstants.PAYLOAD)) {
            if (intent.getAction().equals(
                        IPCConstants.AASB_INTENT_PREFIX + Action.AudioOutput.GET_NUM_BYTES_BUFFERED)) {
                updateBytesBuffered(intent);
            } else {
                Log.i(TAG, "Received intent: " + intent);
                mAACSReceiver.receive(intent, null);
            }
        } else if (intent != null) {
            Log.w(TAG, "Missing Payload, noop...");
        }

        return Service.START_STICKY;
    }

    private void initPingReceiver() {
        mPingReceiver = new PingReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction("com.amazon.aacs.ping");
        filter.addCategory("com.amazon.aacs.pingtopic");
        mContext.registerReceiver(mPingReceiver, filter, "com.amazon.alexaautoclientservice.ping", null);
        Log.i(TAG, "Registered ping receiver");
    }
    private void initializeAASB() {
        Log.d(TAG, "initializeAASB");
        mAASBHandler = new AASBHandler(this);
    }

    private void initLVC() {
        Log.i(TAG, "Initializing LVCInteractionProvider and LVCConfigReceiver.");

        mLVCConfigReceiver = new LVCReceiver(mStateMachine, mAACSContextMap);
        IntentFilter filter = new IntentFilter(LVCServiceConstants.LVC_RECEIVER_INTENT);
        LocalBroadcastManager.getInstance(mContext).registerReceiver(mLVCConfigReceiver, filter);

        mLVCInteractionProvider = new LVCInteractionProvider(getApplicationContext());
        mLVCInteractionProvider.initialize();
    }

    private void initAACSReceivers() {
        Log.i(TAG, "Initialized AACSReceivers");
        ConfigMessageReceivedCallback configMessageReceivedCallback =
                new ConfigMessageReceivedCallback(this, mStateMachine);

        MessageReceivedCallback aasbMessageReceivedCallback = new MessageReceivedCallback() {
            @Override
            public void onMessageReceived(String s) {
                if (mAASBHandler != null) {
                    mAASBHandler.publish(s);
                }
            }
        };

        FetchStreamCallback aasbFetchCallback = new FetchStreamCallback() {
            @Override
            public void onStreamRequested(String streamId, ParcelFileDescriptor writePipe) {
                Log.i(TAG, "onStreamRequested called.  Getting AASBStream by streamId: " + streamId);
                ParcelFileDescriptor.AutoCloseOutputStream stream =
                        new ParcelFileDescriptor.AutoCloseOutputStream(writePipe);
                mOutputStreamMap.put(streamId, stream);

                AASBStream aasbStream = mAASBHandler.openStream(streamId, AASBStream.Mode.READ);
                Log.i(TAG, "onStreamRequested: opening stream.  Stream found =" + (aasbStream != null));
                if (aasbStream == null) {
                    Log.e(TAG, "onStreamRequested: aasbStream is null");
                    closePipe(writePipe);
                    return;
                }

                if (mAASBHandler == null) {
                    Log.e(TAG, "onStreamRequested: mAASBHandler is null");
                    closePipe(writePipe);
                    return;
                }

                if (mIPCStreamWriteExecutor.isShutdown()) {
                    Log.w(TAG, "mIPCStreamWriteExecutor is shut down");
                    closePipe(writePipe);
                    return;
                }

                mIPCStreamWriteExecutor.submit(() -> {
                    Log.i(TAG, "onStreamRequested handler's post Begin.  Getting AASBStream by streamId: " + streamId);
                    try (ParcelFileDescriptor.AutoCloseOutputStream autoCloseOutputStream = stream) {
                        byte[] buffer = new byte[FETCH_READ_BUFFER_CHUNK_SIZE];
                        int size;
                        while (mOutputStreamMap.containsKey(streamId) && !aasbStream.isClosed()) {
                            size = aasbStream.read(buffer);
                            if (size > 0)
                                autoCloseOutputStream.write(buffer, 0, size);
                        }
                        Log.i(TAG, "onStreamRequested: Finished copying from aasbStream");
                    } catch (IOException e) {
                        Log.e(TAG, "onStreamRequested: exception when writing to fetch stream.  e = " + e);
                    }

                    closePipe(writePipe);
                    Log.i(TAG, "onStreamRequested: Finished writing stream.");
                });
            }

            @Override
            public void onStreamFetchCancelled(String streamId) {
                Log.i(TAG, "onStreamFetchCancelled.");
                cancelStream(streamId);
            }
        };

        AACSReceiver.StreamPushedFromSenderCallback
                aasbStreamPushedFromSenderCallback = new AACSReceiver.StreamPushedFromSenderCallback() {
            @Override
            public void onStreamPushedFromSenderCallback(String streamId, ParcelFileDescriptor readPipe) {
                Log.i(TAG, "onStreamPushedFromSenderCallback called.  Getting AASBStream by streamId: " + streamId);

                ParcelFileDescriptor.AutoCloseInputStream stream =
                        new ParcelFileDescriptor.AutoCloseInputStream(readPipe);

                if (mIPCStreamReadExecutor.isShutdown()) {
                    Log.w(TAG, "mIPCStreamReadExecutor is shut down");
                    closePipe(readPipe);
                    return;
                }

                mIPCStreamReadExecutor.submit(() -> {
                    Log.i(TAG,
                            "onStreamPushedFromSenderCallback handler's post begin.  Getting AASBStream by streamId: "
                                    + streamId);
                    try (ParcelFileDescriptor.AutoCloseInputStream autoCloseInputStream = stream) {
                        if (mAASBHandler == null) {
                            Log.e(TAG, "onStreamPushedFromSenderCallback: mAASBHandler is null");
                            closePipe(readPipe);
                            return;
                        }

                        AASBStream aasbStream = mAASBHandler.openStream(streamId, AASBStream.Mode.WRITE);
                        Log.i(TAG,
                                "onStreamPushedFromSenderCallback: opening stream.  Stream found ="
                                        + (aasbStream != null));
                        if (aasbStream == null) {
                            Log.e(TAG, "onStreamPushedFromSenderCallback: aasbStream is null");
                            closePipe(readPipe);
                            return;
                        }

                        byte[] buffer = new byte[PUSH_WRITE_BUFFER_CHUNK_SIZE];
                        boolean isReading = true;

                        while (isReading) {
                            int bytesAvailable = Math.max(Math.min(autoCloseInputStream.available(), buffer.length),
                                    PUSH_WRITE_BUFFER_MIN_CHUNK_SIZE);
                            int bytesRead = autoCloseInputStream.read(buffer, 0, bytesAvailable);
                            if (bytesRead > 0) {
                                aasbStream.write(buffer, 0, bytesRead);
                            }
                            if (bytesRead < 0) {
                                isReading = false;
                            }
                        }
                    } catch (IOException e) {
                        Log.e(TAG,
                                "onStreamPushedFromSenderCallback: exception when writing to fetch stream.  e = " + e);
                    }

                    closePipe(readPipe);
                    Log.i(TAG, "onStreamPushedFromSenderCallback: Finished writing stream.");
                });
            }
        };

        AACSReceiver.Builder builder = new AACSReceiver.Builder();
        mAACSReceiver = builder.withAASBCallback(aasbMessageReceivedCallback)
                                .withConfigCallback(configMessageReceivedCallback)
                                .withFetchCallback(aasbFetchCallback)
                                .withPushCallback(aasbStreamPushedFromSenderCallback)
                                .build();
    }

    private List<AACSModuleFactoryInterface> getExtraModuleFactories() {
        List<AACSModuleFactoryInterface> extraModuleFactories = new ArrayList<>();
        try {
            String folderName = "alexaautoclientservice";
            String factoryKey = "factory";
            String category = "name";
            String[] fileList = getAssets().list(folderName);
            Log.i(TAG, "Begin loading extras");
            for (String f : fileList) {
                InputStream is = getAssets().open(folderName + "/" + f);
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                JSONObject obj = new JSONObject(json);
                if (obj != null) {
                    String factoryName = obj.getJSONObject(factoryKey).getString(category);
                    AACSModuleFactoryInterface instance =
                            (AACSModuleFactoryInterface) Class.forName(factoryName).newInstance();
                    extraModuleFactories.add(instance);
                    Log.i(TAG, "load extra module:" + factoryName);
                }
                is.close();
            }
        } catch (Exception e) {
            Log.e(TAG, String.format("Error while loading extras: %s", e.getMessage()));
        }
        return extraModuleFactories;
    }

    private void getExtrasConfiguration(AACSContext aacsContext, List<AACSModuleFactoryInterface> extraFactories,
            ArrayList<EngineConfiguration> configuration) {
        for (AACSModuleFactoryInterface moduleFactory : extraFactories) {
            List<EngineConfiguration> moduleConfigs = moduleFactory.getConfiguration(aacsContext);
            for (EngineConfiguration moduleConfig : moduleConfigs) {
                Log.d(TAG, moduleConfig.toString());
                configuration.add(moduleConfig);
            }
        }
    }

    private void loadExtrasPlatformInterfaces(List<AACSModuleFactoryInterface> extraFactories) {
        for (AACSModuleFactoryInterface moduleFactory : extraFactories) {
            List<PlatformInterface> platformInterfaces = moduleFactory.getModulePlatformInterfaces(this);
            for (PlatformInterface platformInterface : platformInterfaces) {
                if (!mEngine.registerPlatformInterface(platformInterface))
                    Log.e(TAG, "Could not register extra module interface");
            }
        }
    }

    private boolean startEngine() {
        List<AACSModuleFactoryInterface> extraFactories = getExtraModuleFactories();
        mEngine = Engine.create(this);

        // Generate extras config first, since this may require removing extras modules from OEM config.
        ArrayList<EngineConfiguration> extraConfiguration = new ArrayList<>();
        getExtrasConfiguration(this, extraFactories, extraConfiguration);

        ArrayList<EngineConfiguration> configuration = FileUtil.getEngineConfiguration(this);
        configuration.addAll(extraConfiguration);
        EngineConfiguration[] configurationArray = configuration.toArray(new EngineConfiguration[configuration.size()]);

        if (!mEngine.configure(configurationArray)) {
            Log.e(TAG, "Could not configure engine! Please check the provided configuration.");
            return false;
        }

        loadExtrasPlatformInterfaces(extraFactories);

        if (isDefaultImplementationEnabled(this, FileUtil.AACS_CONFIG_LOCATION_PROVIDER)) {
            mEngine.registerPlatformInterface(new LocationProviderHandler(this));
            Log.v(TAG, "registerPlatformInterface LocationProviderHandler");
        }

        if (isDefaultImplementationEnabled(this, FileUtil.AACS_CONFIG_NETWORK_INFO_PROVIDER)) {
            mEngine.registerPlatformInterface(mNetworkInfoProviderHandler = new NetworkInfoProviderHandler(this));
            Log.v(TAG, "registerPlatformInterface mNetworkInfoProviderHandler");
        }

        if (isDefaultImplementationEnabled(this, FileUtil.AACS_CONFIG_EXTERNAL_MEDIA_ADAPTER)) {
            mMACCPlayer = new MACCPlayer(this);
            mEngine.registerPlatformInterface(mMACCPlayer);
            mMACCPlayer.runDiscovery();
            Log.v(TAG, "registerPlatformInterface mMACCPlayer");
        }

        if (isDefaultImplementationEnabled(this, FileUtil.AACS_CONFIG_PROPERTY_MANAGER)) {
            mPropertyManagerHandler = new PropertyManagerHandler();
            mEngine.registerPlatformInterface(mPropertyManagerHandler);
            Log.v(TAG, "registerPlatformInterface mPropertyManagerHandler");
        }

        if (!mEngine.registerPlatformInterface(mAASBHandler))
            throw new RuntimeException("Could not register AASB handler");

        if (!mEngine.start())
            throw new RuntimeException("Could not start engine");

        // Initialize Property Content Provider only after engine starts and config setting to true
        if (isDefaultImplementationEnabled(this, FileUtil.AACS_CONFIG_PROPERTY_MANAGER)) {
            mAPCP = getContentResolver().acquireContentProviderClient(
                    Uri.parse("content://" + AACSConstants.AACS_PROPERTY_URI));
            AACSPropertyContentProvider apcp = (AACSPropertyContentProvider) mAPCP.getLocalContentProvider();
            apcp.setPropertyManagerHandler(mPropertyManagerHandler);
        }

        return true;
    }

    private void createNotificationAndStartForegroundService() {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            // Create notification channel. Required for Android O and above.
            Log.d(TAG, "Creating notification channel");
            NotificationManager notificationManager =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            NotificationChannel notificationChannel =
                    new NotificationChannel(AACS_CHANNEL_ID, AACS_CHANNEL_NAME, NotificationManager.IMPORTANCE_LOW);

            if (notificationManager != null) {
                notificationManager.createNotificationChannel(notificationChannel);
            }
        }

        // Create notification. Notification channel id is ignored in Android versions below O.
        Log.d(TAG, "Build foreground notification");
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, AACS_CHANNEL_ID)
                                                     .setContentTitle(this.getString(R.string.aacs_title))
                                                     .setContentText(this.getString(R.string.aacs_description))
                                                     .setSmallIcon(R.drawable.alexa_notification_icon)
                                                     .setStyle(new NotificationCompat.BigTextStyle());

        Notification notification = builder.build();

        long duration = System.currentTimeMillis() - mOnCreateTimeMs.longValue();
        startForeground(AACS_SERVICE_STARTED_NOTIFICATION_ID, notification);
        Log.d(TAG, "Starting service in the foreground " + String.valueOf(duration) + " ms after onCreate().");
    }

    private boolean isConfiguredVersionValid() {
        double configuredVersion = FileUtil.getVersionNumber(mContext);
        if (MIN_SUPPORTED_VERSION <= configuredVersion && configuredVersion <= CURRENT_VERSION) {
            return true;
        } else {
            Log.e(TAG,
                    "Invalid AACS version number. Current version=" + CURRENT_VERSION
                            + " Minimum version=" + MIN_SUPPORTED_VERSION);
            return false;
        }
    }

    private void cancelStream(String streamId) {
        ParcelFileDescriptor.AutoCloseOutputStream stream = mOutputStreamMap.remove(streamId);
        try {
            if (stream != null) {
                stream.flush();
                stream.close();
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage(), e);
        }
    }

    private void updateBytesBuffered(Intent intent) {
        try {
            Bundle bundle = intent.getExtras().getBundle("payload");
            String message = bundle.getString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE);
            JSONObject aasbMessage = new JSONObject(message);
            String payload = "";
            if (aasbMessage.has(AASBConstants.PAYLOAD) && !aasbMessage.isNull(AASBConstants.PAYLOAD)) {
                payload = aasbMessage.getJSONObject(AASBConstants.PAYLOAD).toString();
            }

            JSONObject payloadJson;
            payloadJson = new JSONObject(payload);
            String bytes = payloadJson.getString("bufferedBytes");
            mAASBHandler.updateBytesBuffered(bytes);

        } catch (Exception e) {
            Log.e(TAG, "Failed to parse payload");
        }
    }

    private void closePipe(ParcelFileDescriptor pipe) {
        try {
            if (pipe != null)
                pipe.close();
        } catch (Exception e) {
            Log.e(TAG, "Failed to close parcel file descriptor, error: " + e.toString());
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        mContext.unregisterReceiver(mPingReceiver);
        super.onDestroy();
        mStateMachine.setState(State.STOPPED);
    }

    //
    // AACS State Management
    //

    public enum State { STARTED, WAIT_FOR_LVC_CONFIG, CONFIGURED, STOPPED }

    private interface AACSState {
        void enter();

        State getState();
    }

    private class AACSStartedState implements AACSState {
        public AACSStartedState() {}

        @Override
        public void enter() {
            Log.d(TAG, "AACSStateMachine enter started");

            File fileDir = getFilesDir();
            File certsDir = new File(getCacheDir(), FileUtil.CERTS_DIR);
            File appDataDir = new File(fileDir, FileUtil.APPDATA_DIR);
            File externalFileDir = new File(fileDir, FileUtil.EXTERNAL_FILE_DIR);
            File modelsDir = new File(externalFileDir, FileUtil.MODEL_DIR);

            FileUtil.createDirectoriesAndCopyCertificates(
                    certsDir, appDataDir, externalFileDir, modelsDir, getAssets());

            mAACSContextMap = new HashMap<>();
            mAACSContextMap.put(AACSContext.APPDATA_DIR, appDataDir.getPath());
            mAACSContextMap.put(AACSContext.MODEL_DIR, modelsDir.getPath());
            mAACSContextMap.put(AACSContext.CERTS_DIR, certsDir.getPath());

            initAACSReceivers();

            mEngineHandlerThread = new HandlerThread("EngineHandlerThread");
            mEngineHandlerThread.start();
            mEngineHandler = new Handler(mEngineHandlerThread.getLooper());
        }

        @Override
        public State getState() {
            return State.STARTED;
        }
    }

    private class AACSWaitForLVCConfigState implements AACSState {
        public AACSWaitForLVCConfigState() {}

        @Override
        public void enter() {
            if (FileUtil.lvcEnabled())
                initLVC();
        }

        @Override
        public State getState() {
            return State.WAIT_FOR_LVC_CONFIG;
        }
    }

    private class AACSConfiguredState implements AACSState {
        public AACSConfiguredState() {}

        @Override
        public void enter() {
            Log.d(TAG, "AACSStateMachine enter configured");
            // Start engine in its own thread to prevent blocking the main thread of AACS
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    if (isConfiguredVersionValid()) {
                        Log.d(TAG, "Initializing AASB");
                        initializeAASB();
                        Log.d(TAG, "Starting engine");
                        boolean success = startEngine();
                        Log.d(TAG, "Start engine result: " + success);

                        if (success) {
                            // if service is not configured to be a persistent service, start it in the foreground.
                            if (!FileUtil.isPersistentSystemService(mContext)) {
                                Log.d(TAG, "Creating notification and starting foreground service");
                                createNotificationAndStartForegroundService();
                            } else {
                                Log.d(TAG, "Is a persistent service so not calling startForeground()");
                            }
                        }
                    }
                }
            };
            mEngineHandler.post(runnable);
        }

        @Override
        public State getState() {
            return State.CONFIGURED;
        }
    }

    private class AACSStoppedState implements AACSState {
        public AACSStoppedState() {}

        @Override
        public void enter() {
            if (mAACSReceiver != null) {
                mAACSReceiver.shutdown();
                mAACSReceiver = null;
            }

            if (mEngine != null) {
                mEngine.dispose();
            }

            if (mIPCStreamReadExecutor != null) {
                mIPCStreamReadExecutor.shutdown();
                mIPCStreamReadExecutor = null;
            }

            if (mIPCStreamWriteExecutor != null) {
                mIPCStreamWriteExecutor.shutdown();
                mIPCStreamWriteExecutor = null;
            }

            if (mEngineHandlerThread != null) {
                mEngineHandlerThread.quitSafely();
                try {
                    mEngineHandlerThread.join(THREAD_JOIN_TIMEOUT_IN_MILLIS);
                    mEngineHandlerThread = null;
                } catch (InterruptedException e) {
                    Log.e(TAG, "unable to finish EngineHandler thread: " + e);
                }
            }

            if (mNetworkInfoProviderHandler != null) {
                mNetworkInfoProviderHandler.unregister();
            }

            if (mMACCPlayer != null) {
                mMACCPlayer.cleanupMACCClient();
            }

            if (mAASBHandler != null) {
                mAASBHandler.cleanUp();
            }

            if (mAPCP != null) {
                mAPCP.close();
            }

            Log.i(TAG, "AlexaAutoClientService Destroyed");
            mAASBHandler = null;

            if (FileUtil.lvcEnabled()) {
                if (mLVCInteractionProvider != null)
                    mLVCInteractionProvider.uninitialize();

                if (mLVCConfigReceiver != null)
                    LocalBroadcastManager.getInstance(mContext).unregisterReceiver(mLVCConfigReceiver);
            }
        }

        @Override
        public State getState() {
            return State.STOPPED;
        }
    }

    public class AACSStateMachine {
        private AACSStartedState mStartedState;
        private AACSWaitForLVCConfigState mWaitForLVCConfigState;
        private AACSConfiguredState mConfiguredState;
        private AACSStoppedState mStoppedState;
        private AACSState mState;

        public AACSStateMachine() {
            mStartedState = new AACSStartedState();
            mWaitForLVCConfigState = new AACSWaitForLVCConfigState();
            mConfiguredState = new AACSConfiguredState();
            mStoppedState = new AACSStoppedState();
            mState = mStoppedState;
        }

        public void setState(State newState) {
            switch (newState) {
                case STARTED:
                    mState = mStartedState;
                    break;
                case WAIT_FOR_LVC_CONFIG:
                    mState = mWaitForLVCConfigState;
                    break;
                case CONFIGURED:
                    mState = mConfiguredState;
                    break;
                case STOPPED:
                    mState = mStoppedState;
                    break;
                default:
                    Log.e(TAG, "Attempting to set to AACS state that doesn't exist.");
                    return;
            }
            Log.i(TAG, String.format("AACSState updated to %s", mState.getState().toString()));
            mPingReceiver.setState(mState.getState().name());
            mState.enter();
        }

        public State getState() {
            return mState.getState();
        }
    }

    //
    // AACSContext (For Extras Configuration)
    //
    @Override
    public Context getContext() {
        return this;
    }

    @Override
    public String getData(String key) {
        return mAACSContextMap.get(key);
    }

    @Override
    public JSONObject getConfiguration(String configKey) {
        return FileUtil.removeExtrasModuleConfiguration(this, configKey);
    }

    @Override
    public boolean isPlatformInterfaceEnabled(String name) {
        return isDefaultImplementationEnabled(this, name);
    }

    @Override
    public void deregisterAASBPlatformInterface(String module, String name) {
        FileUtil.deregisterAASBPlatformInterface(this, module, name);
    }
}
