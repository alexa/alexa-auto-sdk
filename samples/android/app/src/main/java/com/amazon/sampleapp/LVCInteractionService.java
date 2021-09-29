/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.amazon.alexalve.ILVCClient;
import com.amazon.alexalve.ILVCService;

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
                config.put("LocalSkillService", localSkillServiceNode);

                File cacheDir = getCacheDir();
                File appDataDir = new File(cacheDir, "appdata");
                String appDataDirPath = appDataDir.getAbsolutePath();
                localSkillServiceNode.put("UnixDomainSocketPath", appDataDirPath + "/LSS.socket");

                // The "CarControl" node of the config JSON is required only when custom assets are
                // used in the "aace.carControl" Engine configuration. The value we put in
                // "CarControl.CustomAssetsFilePath" tells the car control component in the LVC APK
                // where to find the definitions of any assets we have used in our car control
                // Engine configuration that are additions to the default assets it already knows
                // about.
                String customAssetsPath = getCarControlAssetsPath();
                if (!customAssetsPath.isEmpty()) {
                    Log.i(TAG, "Using car control custom assets from path " + customAssetsPath);
                    JSONObject carControlNode = new JSONObject();
                    carControlNode.put("CustomAssetsFilePath", customAssetsPath);
                    config.put("CarControl", carControlNode);
                } else {
                    Log.i(TAG, "No car control custom assets are used");
                }

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

                // To use offline local search and navigation features, provide the "LocalSearch"
                // node. You must also register implementations of platform interfaces Navigation
                // and LocalSearchProvider.
                JSONObject localSearchConfig = new JSONObject();
                localSearchConfig.put("NavigationLocalSearchSocketDir", appDataDirPath);
                localSearchConfig.put("NavigationLocalSearchSocketName", "local_search_navigation.socket");
                localSearchConfig.put("LocalSearchEERSocketDir", appDataDirPath);
                localSearchConfig.put("LocalSearchEERSocketName", "local_search_eer.socket");
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
     * Utility function to get the path to the car control custom assets definition file.
     *
     * Note: Normally such a function would not be needed, as the custom assets and their location
     * would be known ahead of time. However, to facilitate testing with this application we allow
     * using a custom car control configuration that overrides the default sample generated by this
     * application. It is possible that this custom configuration includes custom assets, so this
     * function gets the right path to the custom assets file if it exists.
     *
     * @return The path to car control custom assets file or empty string if no custom assets file
     * exists
     */
    private String getCarControlAssetsPath() {
        // If there is a "CarControlConfig.json" on the SD card that overrides the default
        // "aace.carControl" configuration generated by CarControlDataProvider, it may or may
        // not use any custom assets. If it does, the sample app expects the custom assets to
        // be defined in a file called "CarControlAssets.json". If CarControlConfig.json exists
        // but CarControlAssets.json does not, we assume the CarControlConfig.json does not use any
        // custom assets
        String sdCardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String externalConfigPath = sdCardPath + "/CarControlConfig.json";
        File externalConfigFile = new File(externalConfigPath);
        if (externalConfigFile.exists()) {
            String externalCustomAssetsPath = sdCardPath + "/CarControlAssets.json";
            File externalCustomAssetsFile = new File(externalCustomAssetsPath);
            if (externalCustomAssetsFile.exists()) {
                Log.i(TAG, "Using car control custom assets from file on SD card");
                return externalCustomAssetsFile.getAbsolutePath();
            } else {
                Log.i(TAG, "Custom car control config file on SD card does not use custom assets");
                return "";
            }
        }
        // If there is not a "CarControlConfig.json" on the SD card, the sample app is using a
        // default sample "aace.carControl" config generated by CarControlDataProvider. This
        // default sample uses sample custom assets for demo purposes. The sample custom assets
        // are defined in "CarControlAssets.json" in the sample app's assets directory.
        // We copy that file to the app's cache directory so we can retrieve its file path
        else {
            File cacheDir = getCacheDir();
            File appDataDir = new File(cacheDir, "appdata");
            String appDataDirPath = appDataDir.getAbsolutePath();
            File sampleCustomAssetsFile = new File(appDataDir, "CarControlAssets.json");
            FileUtils.copyAsset(getAssets(), "CarControlAssets.json", sampleCustomAssetsFile, false);
            return sampleCustomAssetsFile.getAbsolutePath();
        }
    }
}
