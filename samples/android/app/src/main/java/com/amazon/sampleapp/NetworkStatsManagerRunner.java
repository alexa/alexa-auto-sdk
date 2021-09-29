/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

import static android.app.AppOpsManager.MODE_ALLOWED;
import static android.app.AppOpsManager.OPSTR_GET_USAGE_STATS;

import android.Manifest;
import android.app.Activity;
import android.app.AppOpsManager;
import android.app.usage.NetworkStats;
import android.app.usage.NetworkStatsManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.RemoteException;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;

import com.amazon.sampleapp.impl.DeviceUsage.DeviceUsageHandler;

/**
 * The NetworkStatsManagerRunner is responsible to report network usage data of an application
 * to the DeviceUsageHandler using the Android NetworkStatsManager.
 * <p>
 * Note: NetworkStatsManager is only supported for Android (M) (API level 23 and higher)
 * </p>
 * Refer to https://developer.android.com/reference/android/app/usage/NetworkStatsManager for documentation
 * NetworkStatsManager has the following requirements:
 *
 * Declare the READ_PHONE_STATE permission in your manifest in order to be able to get the subscriberId and you
 * must grant this permission to your application.
 *
 */
public class NetworkStatsManagerRunner implements Runnable {
    private static final String sTag = "NetworkStatsManagerRunner";
    private NetworkStatsManager mNetworkStatsManager;
    private DeviceUsageHandler mDeviceUsageHandler;
    private Context mContext;
    int mAlexaApplicationPackageUid;

    // The starting time of the network consumption data capture
    private long mStartTime = 0L;
    // The ending time of the network consumption data capture
    private long mEndTime = 0L;

    /**
     * The subscriber id associated to the SIM on the car. The subscriber id is only required to query
     * the network consumption over the MOBILE interface. This value needs to be specified by OEMs if it
     * is available to them upfront. The sample implementation queries the subscriber id using the
     * Telephony manager. Android devices starting API level 10 (Q) cannot access subscriber id without
     * priviliged permissions. i.e android.permission.READ_PRIVILEGED_PHONE_STATE
     */
    String mSubscriberId = "";
    int networkType = 999;

    @RequiresApi(api = Build.VERSION_CODES.M)
    public NetworkStatsManagerRunner(
            Activity activity, DeviceUsageHandler deviceUsageHandler, Context context, int alexaApplicationPackageUid) {
        mContext = context;
        mDeviceUsageHandler = deviceUsageHandler;
        mNetworkStatsManager = (NetworkStatsManager) context.getSystemService(Context.NETWORK_STATS_SERVICE);
        mAlexaApplicationPackageUid = alexaApplicationPackageUid;

        /**
         * Check if the application has the READ_PHONE_STATE permission.
         */
        if (!checkForReadPhoneStatePermission()) {
            Log.e(sTag, "Read phone state permissions required to record mobile network consumption");
            Toast.makeText(context, "Read phone state permissions required", Toast.LENGTH_LONG).show();
        }
    }

    /**
     * Get the subscriber id of the SIM.
     * @param context
     * @return subscriber id
     */
    private String getSubscriberId(Context context) {
        if (ConnectivityManager.TYPE_MOBILE == networkType) {
            TelephonyManager manager = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
            if (!checkForReadPhoneStatePermission()) {
                Log.e(sTag, "Read phone state permissions required");
                return "";
            }
            return manager.getSubscriberId();
        }
        return "";
    }

    /**
     * Get the current network information
     * @return the id associated with the current active network connection
     */
    private int getCurrentNetworkInfo() {
        ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo();
        if (info == null || !info.isConnected())
            Log.e(sTag, "Application not connected to any network");
        else if (info.getType() == ConnectivityManager.TYPE_WIFI)
            networkType = ConnectivityManager.TYPE_WIFI;
        else if (info.getType() == ConnectivityManager.TYPE_MOBILE)
            networkType = ConnectivityManager.TYPE_MOBILE;

        return networkType;
    }

    /**
     * Check if READ_PHONE_STATE permission is granted
     * @return true if granted, else false
     */
    private boolean checkForReadPhoneStatePermission() {
        if (ActivityCompat.checkSelfPermission(mContext, Manifest.permission.READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED) {
            return false;
        }
        return true;
    }

    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    public void run() {
        calculateNetworkStatsUsage();
    }

    @RequiresApi(api = Build.VERSION_CODES.M)
    private void calculateNetworkStatsUsage() {
        Log.i(sTag, " calculateNetworkStatsUsage");
        long currentRxBytes = 0L;
        long currentTxBytes = 0L;
        long currentTotalBytes = 0L;
        try {
            NetworkStats networkStatsByApp = null;
            mEndTime = System.currentTimeMillis();
            Log.i(sTag, "End time is " + String.valueOf(mEndTime));
            networkType = getCurrentNetworkInfo();
            if (networkType == ConnectivityManager.TYPE_WIFI) {
                networkStatsByApp = mNetworkStatsManager.querySummary(networkType, "", 0, mEndTime);
            } else if (networkType == ConnectivityManager.TYPE_MOBILE) {
                // Subscriber id is required to query NetworkStatsManager for network consumption data over mobile
                // interface. OEMs can either hardcode the subscriber id or can use the following mechanism to access
                // the subscriber id at runtime. The sample implementation queries the subscriber id using the
                // Telephony manager. Android devices starting API level 10 (Q) cannot access subscriber id without
                // priviliged permissions. i.e android.permission.READ_PRIVILEGED_PHONE_STATE.
                // Note : Third-party apps installed from the Google Play Store cannot declare privileged permissions.
                // Refer to https://developer.android.com/about/versions/10/privacy/changes#non-resettable-device-ids
                // for more details.
                if (mSubscriberId.isEmpty()) {
                    try {
                        mSubscriberId = getSubscriberId(mContext);
                    } catch (SecurityException e) {
                        Log.e(sTag,
                                "Cannot access subscriber id because READ_PHONE_STATE permission is not granted or Android API level is 29 or higher");
                        Log.e(sTag, "calculateNetworkStatsUsage", e);
                        return;
                    }
                    if (mSubscriberId == null) {
                        Log.e(sTag, "Cannot access subscriber id because READ_PHONE_STATE permission is not granted");
                        return;
                    }
                    if (mSubscriberId.isEmpty()) {
                        Log.e(sTag, "Subscriber id is empty");
                        return;
                    }
                }

                // Query the NetworkStatsManager for the network usage consumption
                networkStatsByApp = mNetworkStatsManager.querySummary(networkType, mSubscriberId, 0, mEndTime);
            } else {
                Log.e(sTag, " networkType not supported");
                return;
            }

            if (networkStatsByApp == null) {
                Log.e(sTag, "Null networkStatsByApp");
                return;
            }
            do {
                NetworkStats.Bucket bucket = new NetworkStats.Bucket();
                networkStatsByApp.getNextBucket(bucket);
                if (mAlexaApplicationPackageUid == bucket.getUid()) {
                    currentRxBytes += bucket.getRxBytes();
                    currentTxBytes += bucket.getTxBytes();
                }
            } while (networkStatsByApp.hasNextBucket());
            currentTotalBytes = currentRxBytes + currentTxBytes;
            Log.i("Current Consumption is ",
                    "Current Rx Bytes " + String.valueOf(currentRxBytes) + " Current Tx Bytes "
                            + String.valueOf(currentTxBytes) + " Current Total Bytes "
                            + String.valueOf(currentTotalBytes));

            DeviceUsageHandler.NetworkType networkInterfaceType = null;
            if (networkType == ConnectivityManager.TYPE_MOBILE) {
                networkInterfaceType = DeviceUsageHandler.NetworkType.MOBILE;
            } else if (networkType == ConnectivityManager.TYPE_WIFI) {
                networkInterfaceType = DeviceUsageHandler.NetworkType.WIFI;
            }
            mDeviceUsageHandler.reportUsage(
                    networkInterfaceType, currentRxBytes, currentTxBytes, currentTotalBytes, mStartTime, mEndTime);
            mStartTime = mEndTime;
        } catch (RemoteException | SecurityException e) {
            Log.e(sTag, "calculateNetworkStatsUsage", e);
        }
    }
}
