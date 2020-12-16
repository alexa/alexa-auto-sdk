/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Environment;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;

import com.amazon.alexalve.ILVCClient;
import com.amazon.alexalve.ILVCService;
import com.amazon.sampleapp.FileUtils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.util.Iterator;

/**
 * LVCInteractionService is responsible for establishing a connection with the LVC service and
 * provides handlers for bi-directional communication with LVC
 */
public class LVCInteractionService extends Service {
    private static final String TAG = "LVC_" + LVCInteractionService.class.getSimpleName();

    private static final String LVC_SERVICE_ACTION = "com.amazon.alexalve.LocalVoiceControlService";
    private static final String LVC_SERVICE_PACKAGE_NAME = "com.amazon.alexalve";

    public static final String LVC_RECEIVER_INTENT = "com.amazon.sampleapp.lvcconfigreceiver";
    public static final String LVC_RECEIVER_CONFIGURATION = "configuration";
    public static final String LVC_RECEIVER_FAILURE_REASON = "failure_reason";
    public static final String LVC_RECEIVER_FAILURE_REASON_LVC_NOT_INSTALLED = "lvc_not_installed";

    private ILVCService mLVCService;
    private volatile String mLVCConfig = "";

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand; startId: " + startId);
        if (mLVCService != null) {
            // Connection to LVC service is already established
            Log.i(TAG, "onStartCommand received when LVC service is already connected");
            if (!mLVCConfig.isEmpty()) {
                // We have config from when LVC started previously
                sendAHEInitSuccess(mLVCConfig);
            } else {
                Log.i(TAG, "LVC service is already connected but config is not yet available");
            }
        } else {
            // LVC is not connected. Try to initialize LVC
            Log.i(TAG, "onStartCommand received, LVC service not connected; proceeding to initialize");
            initLVC();
        }

        return super.onStartCommand(intent, flags, startId);
    }

    private void initLVC() {
        Log.d(TAG, "Attempting to initialize remote LVC service");

        // Check if LVC service (from LVC APK) is installed on this device
        Intent serviceIntent = new Intent();
        serviceIntent.setAction(LVC_SERVICE_ACTION);
        serviceIntent.setPackage(LVC_SERVICE_PACKAGE_NAME);
        PackageManager packageManager = getPackageManager();
        ResolveInfo resolveInfo = packageManager.resolveService(serviceIntent, 0);

        if (resolveInfo != null) {
            Log.i(TAG, "LVC service found, binding to service");
            bindService(serviceIntent, mConnection, Context.BIND_AUTO_CREATE);
        } else {
            // LVC service not installed. Send failure broadcast and stop this service
            Log.i(TAG, "LVC service not installed on the device. Stopping LVCInteractionService");
            sendAHEInitFailure(LVC_RECEIVER_FAILURE_REASON_LVC_NOT_INSTALLED);
            stopSelf();
        }
    }

    private void sendAHEInitFailure(String reason) {
        Intent intent = new Intent();
        intent.setAction(LVC_RECEIVER_INTENT);
        intent.putExtra(LVC_RECEIVER_FAILURE_REASON, reason);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private void sendAHEInitSuccess(String result) {
        Intent intent = new Intent();
        intent.setAction(LVC_RECEIVER_INTENT);
        intent.putExtra(LVC_RECEIVER_CONFIGURATION, result);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.i(TAG, "onServiceConnected: Connected to LocalVoiceEngineService");

            mLVCService = ILVCService.Stub.asInterface(service);
            if (mLVCService == null) {
                return;
            }

            try {
                // Register this as the Auto SDK client (config provider and observer) for LVC service
                Log.i(TAG, "Registering as the Auto SDK client for LVC Service");
                mLVCService.registerClient(mLVCClient);

                // Send signal to LVC service to start when ready
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

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");

        if (mLVCService != null) {
            try {
                Log.i(TAG, "Un-registering the Auto SDK client for LVC Service");
                mLVCService.unregisterClient(mLVCClient);
            } catch (RemoteException e) {
                Log.e(TAG, "Error calling remote process ", e);
            }

            Log.i(TAG, "Unbinding from LocalVoiceControlService");
            unbindService(mConnection);
        }

        super.onDestroy();
    }

    /**
     * Provides Auto SDK configuration component and receives observer callbacks from remote
     * LVC service
     */
    private ILVCClient mLVCClient = new ILVCClient.Stub() {
        @Override
        public String getConfiguration() {
            Log.i(TAG, "Configuration requested from LVC Service");

            String configString = "";
            try {
                // Construct config expected by LVC
                JSONObject config = new JSONObject();

                JSONObject localSkillServiceNode = new JSONObject();
                JSONObject carControlNode = new JSONObject();
                config.put("LocalSkillService", localSkillServiceNode);

                File cacheDir = getCacheDir();
                File appDataDir = new File(cacheDir, "appdata");
                String appDataDirPath = appDataDir.getAbsolutePath();
                localSkillServiceNode.put("UnixDomainSocketPath", appDataDirPath + "/LSS.socket");

                // To use custom car control assets defined in "CarControlAssets.json", add the
                // "CustomAssetsFilePath". This tells the car control component in the LVC APK about
                // any assets we have defined in our car control configuration that are additions
                // to the default it already uses.
                // If your car control config used by your own application doesn't use additional
                // assets, skip adding the "CarControl" node to this configuration entirely.
                config.put("CarControl", carControlNode);
                String carControlAssetsPath = getCarControlAssetsPath(appDataDirPath);
                carControlNode.put("CustomAssetsFilePath", carControlAssetsPath);

                // To configure LVC to use the custom volume range, add the "CustomVolume" node inside
                // the "lvcConfig" node. This tells the LVC service in the LVC APK to use a volume range
                // different than the default. If your application does not use a custom volume range for online
                // utterances, skip supplying a custom volume configuration to LVC.
                //
                // To facilitate the testing, we check if a custom volume in configuration file
                // "autoSDKSampleAppExtraConfig.json" is present on root of the SD card. If present, it should
                // define the custom volume config in the JSON format as shown below and should match
                // the custom configuration you use for the cloud.
                //
                // {
                //     "lvcConfig": {
                //         "CustomVolume": {
                //             "minVolumeValue": "<Min VUI Volume Value>",
                //             "maxVolumeValue": "<Max VUI Volume Value>",
                //             "volumeAdjustmentStepValue": "<Device volume adjustment step value>"
                //         }
                //     }
                // }

                JSONObject extraConfig =
                        FileUtils.getOptionalConfigFromSDCard("autoSDKSampleAppExtraConfig.json", "lvcConfig");
                if (extraConfig != null) {
                    Log.i(TAG, "Reading extra config directly from file.");
                    Iterator<String> keys = extraConfig.keys();
                    while (keys.hasNext()) {
                        String key = keys.next();
                        Log.i(TAG, "Config key = " + key);
                        Object val = extraConfig.get(key);
                        if (val instanceof JSONObject) {
                            config.put(key, extraConfig.getJSONObject(key));
                        } else if (val instanceof JSONArray) {
                            config.put(key, extraConfig.getJSONArray(key));
                        }
                    }
                }

                // To use offline local POI search and navigation features, provide the "LocalSearch"
                // node. You must also register implementations of platform interfaces Navigation
                // and LocalSearchProvider.
                JSONObject localSearchConfig = new JSONObject();
                localSearchConfig.put("NavigationPOISocketDir", appDataDirPath);
                localSearchConfig.put("NavigationPOISocketName", "poi_navigation.socket");
                localSearchConfig.put("POIEERSocketDir", appDataDirPath);
                localSearchConfig.put("POIEERSocketName", "poi_eer.socket");
                config.put("LocalSearch", localSearchConfig);

                configString = config.toString();
            } catch (JSONException e) {
                Log.e(TAG, e.getMessage());
            }
            Log.d(TAG, "Returning config to LVC service: " + configString);
            return configString;
        }

        @Override
        public void configure(String configuration) {
            Log.i(TAG, "Configuration received from LVC service");
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

    /**
     * Utility function to get the path to the car control custom assets
     *
     * Note: Normally such a function would not be needed, as the custom assets and their location
     * would be known ahead of time. However, to facilitate testing with this application we allow
     * using a custom car control configuration that overrides the default generated by this
     * application. It is possible that this custom configuration includes custom assets different
     * from the default custom assets that match the default config. If this is the case,
     * the user should also have a "CarControlAssets.json" on the SD card that complements their
     * overridden config "CarControlConfig.json" also on the SD card. We check for the custom
     * assets file on the SD card here and provide its path if it's present. Else we use the
     * default custom assets this application already provides.
     *
     * Note: If the custom assets file is present on the SD card, this function assumes it works
     * with whatever car control configuration you are providing (whether it be the default
     * generated or your own custom file). Do not include the assets file on the SD card if it
     * won't match the car control configuration you provided to the Auto SDK Engine, and do not
     * include custom assets in your car control config that are different from the default custom
     * assets without including their corresponding definitions in the SD card file.
     *
     * @param appDataDirPath The path to an "appdata" subdirectory of the cache directory
     * @return The path to car control custom assets
     */
    private String getCarControlAssetsPath(String appDataDirPath) {
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String externalAssetsPath = sdCardPath + "/CarControlAssets.json";
        File externalAssetsFile = new File(externalAssetsPath);

        if (externalAssetsFile.exists()) {
            Log.i(TAG, "Using car control custom assets from file on SD card");
            return externalAssetsFile.getAbsolutePath();
        } else {
            // The default custom assets live in the assets directory of this application's source
            // code. We copy that file to the "appdata" subdirectory of the cache directory
            // and provide that path
            File cacheDirFile = new File(appDataDirPath, "CarControlAssets.json");
            FileUtils.copyAsset(getAssets(), "CarControlAssets.json", cacheDirFile, false);
            return cacheDirFile.getAbsolutePath();
        }
    }
}
