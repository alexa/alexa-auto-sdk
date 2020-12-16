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

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexaautoclientservice.constants.LVCServiceConstants;
import com.amazon.alexaautoclientservice.util.FileUtil;
import com.amazon.alexalve.ILVCClient;
import com.amazon.alexalve.ILVCService;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.IOException;
import java.lang.ref.WeakReference;

public class LVCInteractionProvider {
    private static final String TAG = AACSConstants.AACS + "-"
            + "LVC_" + LVCInteractionProvider.class.getSimpleName();

    private WeakReference<Context> mContext;
    private ILVCService mLVCService;
    private volatile String mLVCConfig = "";

    public LVCInteractionProvider(@NonNull Context context) {
        mContext = new WeakReference<>(context);
    }

    public void initialize() {
        if (mLVCService != null) {
            Log.i(TAG, "onStartCommand received when LVC service is already connected");
            if (!mLVCConfig.isEmpty()) {
                sendAHEInitSuccess(mLVCConfig);
            } else {
                Log.e(TAG, "LVC service is already connected but config is not yet available");
            }
        } else {
            Log.i(TAG, "onStartCommand received, LVC service not connected; proceeding to initialize");
            initLVC();
        }
    }

    private void initLVC() {
        Log.d(TAG, "Attempting to initialize remote LVC service");

        Context context = mContext.get();

        Intent serviceIntent = new Intent();
        serviceIntent.setAction(LVCServiceConstants.LVC_SERVICE_ACTION);
        serviceIntent.setPackage(LVCServiceConstants.LVC_SERVICE_PACKAGE_NAME);
        PackageManager packageManager = context.getPackageManager();
        ResolveInfo resolveInfo = packageManager.resolveService(serviceIntent, 0);

        if (resolveInfo != null) {
            Log.i(TAG, "LVC service found, binding to service");
            context.bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        } else {
            Log.i(TAG, "LVC service not installed on the device. Stopping LVCInteractionService");
            sendAHEInitFailure(LVCServiceConstants.LVC_RECEIVER_FAILURE_REASON_LVC_NOT_INSTALLED);
            context.stopService(serviceIntent);
        }
    }

    private void sendAHEInitFailure(String reason) {
        Log.i(TAG, "sendAHEInitFailure: Broadcasting LVC Failure intent.");
        Context context = mContext.get();
        Intent intent = new Intent();
        intent.setAction(LVCServiceConstants.LVC_RECEIVER_INTENT);
        intent.putExtra(LVCServiceConstants.LVC_RECEIVER_FAILURE_REASON, reason);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    private void sendAHEInitSuccess(String result) {
        Log.i(TAG, "sendAHEInitSuccess: Broadcasting LVC Configuration intent.");
        Context context = mContext.get();
        Intent intent = new Intent();
        intent.setAction(LVCServiceConstants.LVC_RECEIVER_INTENT);
        intent.putExtra(LVCServiceConstants.LVC_RECEIVER_CONFIGURATION, result);
        LocalBroadcastManager.getInstance(context).sendBroadcast(intent);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.i(TAG, "onServiceConnected: Connected to LocalVoiceEngineService");

            mLVCService = ILVCService.Stub.asInterface(service);
            if (mLVCService == null) {
                return;
            }

            try {
                Log.i(TAG, "Registering as the Auto SDK client for LVC Service");
                mLVCService.registerClient(mLVCClient);

                Log.i(TAG, "Sending start signal to LVC service");
                mLVCService.start();
            } catch (RemoteException e) {
                Log.e(TAG, "Error calling remote process ", e);
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            Log.i(TAG, "onServiceDisconnected. Disconnected from LocalVoiceEngineService");
            mLVCService = null;
        }
    };

    public void uninitialize() {
        Context context = mContext.get();
        if (mLVCService != null) {
            try {
                Log.i(TAG, "Un-registering the Auto SDK client for LVC Service");
                mLVCService.unregisterClient(mLVCClient);
            } catch (RemoteException e) {
                Log.e(TAG, "Error calling remote process ", e);
            }

            Log.i(TAG, "Unbinding from LocalVoiceControlService");
            context.unbindService(mConnection);
        }
    }

    private ILVCClient mLVCClient = new ILVCClient.Stub() {
        @Override
        public String getConfiguration() {
            String configString = FileUtil.getLVCConfiguration(mContext.get());

            File fileDir = mContext.get().getFilesDir();
            JSONObject configToLvcApk = new JSONObject();
            try {
                if (configString != null && !configString.isEmpty()) {
                    configToLvcApk = new JSONObject(configString);
                }
                File lssSocket = new File(fileDir, "LSS.socket");
                JSONObject lssJson = new JSONObject();
                lssJson.put("UnixDomainSocketPath", lssSocket.getPath());
                configToLvcApk.put("LocalSkillService", lssJson);

                File externalFiles = new File(fileDir, "externalFiles");
                File lvcFiles = new File(externalFiles, "aacs.localVoiceControl");
                File ccAssetsFile = new File(lvcFiles, "CarControlAssets.json");
                if (ccAssetsFile.exists()) {
                    JSONObject ccJson = new JSONObject();
                    ccJson.put("CustomAssetsFilePath", ccAssetsFile.getPath());
                    configToLvcApk.put("CarControl", ccJson);
                }
                JSONObject localSearchJson = new JSONObject();
                localSearchJson.put("NavigationPOISocketDir", fileDir.getPath());
                localSearchJson.put("NavigationPOISocketName", "poi_navigation.socket");
                localSearchJson.put("POIEERSocketDir", fileDir.getPath());
                localSearchJson.put("POIEERSocketName", "poi_eer.socket");
                configToLvcApk.put("LocalSearch", localSearchJson);
            } catch (JSONException e) {
                Log.w(TAG, String.format("Error while constructing config for LVC APK. Error: %s", e.getMessage()));
            }

            Log.d(TAG, "Returning config to LVC service");
            return configToLvcApk.toString();
        }

        @Override
        public void configure(String configuration) {
            Log.i(TAG, String.format("Configuration received from LVC service."));
            mLVCConfig = configuration;
            sendAHEInitSuccess(configuration);
        }

        @Override
        public void onStart() {
            Log.i(TAG, "onStart from LVC service");
        }

        @Override
        public void onStop() {
            Log.i(TAG, "onStop from LVC service");
        }
    };
}
