package com.amazon.sampleapp;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import com.amazon.alexalve.ILVECallback;
import com.amazon.alexalve.ILVEService;

import org.json.JSONObject;

import java.io.File;
import java.util.List;


/**
 * LVEInteractionService that is responsible for establishing a connection with AHE and also
 * provides handlers for bi directional communication with AHE
 */
public class LVEInteractionService extends Service {

    private static final String TAG = "LVE_" + LVEInteractionService.class.getSimpleName();

    private static final String LVE_SERVICE_ACTION = "com.amazon.alexalve.LocalVoiceEngineService";
    private static final String LVE_SERVICE_PACKAGE_NAME = "com.amazon.alexalve";
    public static final String LVE_RECEIVER_INTENT = "com.amazon.sampleapp.lveconfigreceiver";
    public static final String LVE_RECEIVER_CONFIGURATION = "configuration";
    public static final String LVE_RECEIVER_FAILURE_REASON = "failure_reason";
    public static final String LVE_RECEIVER_FAILURE_REASON_LVE_NOT_INSTALLED = "lve_not_installed";

    private ILVEService mLocalVoiceEngineService;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        try {
            if (mLocalVoiceEngineService != null && mLocalVoiceEngineService.isRunning()) {
                // LVE is already running. Send success broadcast
                String configurationFromLVE = getConfigurationFromLVE();
                sendAHEInitSuccess(configurationFromLVE);
                return super.onStartCommand(intent, flags, startId);
            } else {
                // LVE is not running. Try to initialize LVE
                initLVE();
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return super.onStartCommand(intent, flags, startId);
    }

    private void initLVE() {
        // Check if LVE service is installed on this device. LVE service is part of LVE apk
        Intent serviceIntent = new Intent();
        serviceIntent.setAction(LVE_SERVICE_ACTION);
        serviceIntent.setPackage(LVE_SERVICE_PACKAGE_NAME);
        PackageManager packageManager = getPackageManager();
        List<ResolveInfo> resolveInfos = packageManager.queryIntentServices(serviceIntent, 0);
        if (resolveInfos != null && resolveInfos.size() > 0) {
            // LVE service available. Try to bind to the service. This will initialize LVE
            bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        } else {
            //LVE service not installed. Send failure broadcast
            Log.i(TAG, "LVE apk not installed on the device stopping LVEInteractionService");
            sendAHEInitFailure(LVE_RECEIVER_FAILURE_REASON_LVE_NOT_INSTALLED);
            stopSelf();
        }
    }

    private void sendAHEInitFailure(String reason) {
        Intent intent = new Intent();
        intent.setAction(LVE_RECEIVER_INTENT);
        intent.putExtra(LVE_RECEIVER_FAILURE_REASON, reason);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private void sendAHEInitSuccess(String result) {
        Intent intent = new Intent();
        intent.setAction(LVE_RECEIVER_INTENT);
        intent.putExtra(LVE_RECEIVER_CONFIGURATION, result);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.i(TAG, "Connection to LVE app is Successful ");

            mLocalVoiceEngineService = ILVEService.Stub.asInterface(service);
            try {
                if (mLocalVoiceEngineService == null) {
                    return;
                }
                updateConfigAndStartLVE();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            mLocalVoiceEngineService = null;
            Log.i(TAG, "Lost Connection to LVE app ");
        }
    };

    private void updateConfigAndStartLVE() throws RemoteException {
        mLocalVoiceEngineService.registerCallback(mCallback);

        // Get the Config from LVE and use it to start AAC Engine
        String configurationFromLVE = getConfigurationFromLVE();
        mLocalVoiceEngineService.configure(configurationFromLVE);
        sendAHEInitSuccess(configurationFromLVE);
        mLocalVoiceEngineService.start();
    }

    private String getConfigurationFromLVE() throws RemoteException {
        String json = mLocalVoiceEngineService.getConfiguration();
        try {
            JSONObject config = new JSONObject(json);
            File cacheDir = getCacheDir();
            File appDataDir = new File( cacheDir, "appdata" );

            config.put("LocalSkillService.Server.Endpoint", appDataDir.getPath() + "/LSS.socket");
            json = config.toString();
        }
        catch( Throwable ex ) {
            ex.printStackTrace();
        }
        return json;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mLocalVoiceEngineService != null) {
            try {
                mLocalVoiceEngineService.unregisterCallback(mCallback);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            unbindService(mConnection);
        }
    }

    /**
     * This implementation is used to receive callbacks from the remote LVE service.
     */
    private ILVECallback mCallback = new ILVECallback.Stub() {

        /**
         * This is called by the remote AHE service.  Note that IPC calls are dispatched through a
         * thread pool running in each process, so the code executing here will
         * NOT be running in our main thread like most other things -- so,
         * to update the UI, we need to use a Handler to hop over there.
         */
        @Override
        public void sendMessage(String path, String payload) throws RemoteException {
            Log.i(TAG, "sendMessage received in Sample App. Path: " + path + " payload: "+ payload);
        }
    };
}
