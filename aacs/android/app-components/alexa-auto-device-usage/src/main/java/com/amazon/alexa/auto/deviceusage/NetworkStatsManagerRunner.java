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

package com.amazon.alexa.auto.deviceusage;

import android.Manifest;
import android.app.usage.NetworkStats;
import android.app.usage.NetworkStatsManager;
import android.content.Context;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.RemoteException;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.core.app.ActivityCompat;

import com.amazon.aacsconstants.AACSConstants;

/**
 * The NetworkStatsManagerRunner is responsible to report network usage data of an application
 * to the DeviceUsageHandler using the Android NetworkStatsManager.
 * Refer to https://developer.android.com/reference/android/app/usage/NetworkStatsManager for documentation
 *
 * On Android 10 or higher versions, the Alexa application must have the READ_PRIVILEGED_PHONE_STATE permission
 * in order to be able to get the subscriberId. On Android 9 or lower versions, the Alexa application requires
 * the READ_PHONE_STATE permission instead for getting the subscriberId.
 */
public class NetworkStatsManagerRunner implements Runnable {
    public static final String TAG = NetworkStatsManagerRunner.class.getSimpleName();
    private NetworkStatsManager mNetworkStatsManager;
    private DeviceUsageHandler mDeviceUsageHandler;
    private Context mContext;
    private int mAlexaApplicationPackageUid;

    // The starting time of the network consumption data capture
    private long mStartTime = 0L;
    // The ending time of the network consumption data capture
    private long mEndTime = 0L;

    /**
     * The subscriber id associated to the eSIM of a user. The subscriber id is only required to query
     * the network consumption over the MOBILE interface.
     * The default implementation queries the subscriber id using the Telephony manager.
     * Android devices starting API level 10 (Q) cannot access subscriber id without
     * android.permission.READ_PRIVILEGED_PHONE_STATE.
     * This is a privileged permission that can only be granted to apps preloaded on the device.
     * the application needs to be a privileged system app in order to obtain this permission.
     */
    private String mSubscriberId = "";
    private int mNetworkType = 999;

    public NetworkStatsManagerRunner(
            DeviceUsageHandler deviceUsageHandler, Context context, int alexaApplicationPackageUid) {
        this.mContext = context;
        this.mDeviceUsageHandler = deviceUsageHandler;
        mNetworkStatsManager = (NetworkStatsManager) context.getSystemService(Context.NETWORK_STATS_SERVICE);
        this.mAlexaApplicationPackageUid = alexaApplicationPackageUid;
    }

    /**
     * Get the subscriber id of the SIM.
     * @return subscriber id
     */
    private String getSubscriberId() throws SecurityException {
        if (ConnectivityManager.TYPE_MOBILE == mNetworkType) {
            TelephonyManager manager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P && !checkForReadPhoneStatePermission()) {
                Log.e(TAG,
                        "READ_PHONE_STATE permission is required for device on API 28 and lower. "
                                + "Device on API 29 and higher requires READ_PRIVILEGED_PHONE_STATE");
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
        int networkType = 999;
        if (info == null || !info.isConnected())
            Log.e(TAG, "Application not connected to any network");
        else if (info.getType() == ConnectivityManager.TYPE_WIFI)
            networkType = ConnectivityManager.TYPE_WIFI;
        else if (info.getType() == ConnectivityManager.TYPE_MOBILE)
            networkType = ConnectivityManager.TYPE_MOBILE;

        return networkType;
    }

    /**
     * Check if READ_PHONE_STATE permission is granted to get the subscriber Id
     *
     * Android devices starting API level 10 (Q) cannot access subscriber id without
     * android.permission.READ_PRIVILEGED_PHONE_STATE.
     * This is a privileged permission that can only be granted to apps preloaded on the device.
     * Your Alexa app needs to be a privileged system app in order to obtain this permission.
     *
     * @return true if granted, else false
     */
    private boolean checkForReadPhoneStatePermission() {
        if (ActivityCompat.checkSelfPermission(mContext, Manifest.permission.READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED) {
            return false;
        }
        return true;
    }

    @Override
    public void run() {
        calculateNetworkStatsUsage();
    }

    private void calculateNetworkStatsUsage() {
        long currentRxBytes = 0L;
        long currentTxBytes = 0L;
        long currentTotalBytes = 0L;
        try {
            NetworkStats networkStatsByApp = null;
            mEndTime = System.currentTimeMillis();
            Log.v(TAG, String.format("calculateNetworkStatsUsage: End time is %d", mEndTime));
            mNetworkType = getCurrentNetworkInfo();
            if (mNetworkType == ConnectivityManager.TYPE_WIFI) {
                networkStatsByApp = mNetworkStatsManager.querySummary(mNetworkType, "", 0, mEndTime);
            } else if (mNetworkType == ConnectivityManager.TYPE_MOBILE) {
                // Subscriber id is required to query NetworkStatsManager for network consumption data over mobile
                // interface. The default implementation queries the subscriber id using the Telephony manager.
                // Android devices starting API level 10 (Q) cannot access subscriber id without
                // priviliged permissions. i.e android.permission.READ_PRIVILEGED_PHONE_STATE.
                if (mSubscriberId.isEmpty()) {
                    try {
                        mSubscriberId = getSubscriberId();
                    } catch (SecurityException e) {
                        Log.e(TAG,
                                String.format(
                                        "calculateNetworkStatsUsage: Device on API level 29 or higher requires privileged permission to access subscriber Id. Exception encountered: %s",
                                        e.getMessage()));
                        return;
                    }
                    if (mSubscriberId == null) {
                        Log.e(TAG, "calculateNetworkStatsUsage: subscriber id is null.");
                        return;
                    }
                    if (mSubscriberId.isEmpty()) {
                        Log.e(TAG, "calculateNetworkStatsUsage: Subscriber id is empty.");
                        return;
                    }
                }

                // Query the NetworkStatsManager for the network usage consumption
                networkStatsByApp = mNetworkStatsManager.querySummary(mNetworkType, mSubscriberId, 0, mEndTime);
            } else {
                Log.e(TAG, "networkType not supported");
                return;
            }

            if (networkStatsByApp == null) {
                Log.e(TAG, "Null networkStatsByApp");
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

            DeviceUsageHandler.NetworkType networkInterfaceType = null;
            if (mNetworkType == ConnectivityManager.TYPE_MOBILE) {
                networkInterfaceType = DeviceUsageHandler.NetworkType.MOBILE;
            } else if (mNetworkType == ConnectivityManager.TYPE_WIFI) {
                networkInterfaceType = DeviceUsageHandler.NetworkType.WIFI;
            }
            mDeviceUsageHandler.reportUsage(
                    networkInterfaceType, currentRxBytes, currentTxBytes, currentTotalBytes, mStartTime, mEndTime);
            mStartTime = mEndTime;
        } catch (RemoteException | SecurityException e) {
            Log.e(TAG, String.format("Failed to report usage because %s", e.getMessage()));
        }
    }
}