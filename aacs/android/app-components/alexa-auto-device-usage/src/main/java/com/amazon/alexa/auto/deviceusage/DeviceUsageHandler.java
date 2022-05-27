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

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

/**
 * The DeviceUsageHandler starts/stops the NetworkStatsManagerRunner based on the AACS running status,
 * and reports the AACS network usage data to the engine via the AASB DeviceUsage messages.
 *
 * If your Alexa application runs in a different process from AACS, your Alexa app needs to hold the
 * {@link android.Manifest.permission#PACKAGE_USAGE_STATS} privileged permission to collect the network
 * usage data from AACS.
 *
 */
public class DeviceUsageHandler {
    public static final String TAG = DeviceUsageHandler.class.getSimpleName();

    // The time interval in minutes before the application reports its usage to the Engine.
    private final int TIME_INTERVAL = 5;

    private final Context mContext;

    // The package name of the application using Alexa
    private final String mPackageName;

    private NetworkStatsManagerRunner mNetworkStatsManagerRunner;

    private Package mAlexaApplicationPackage;
    private ScheduledExecutorService mExecutor;

    // Class to denote a package
    public class Package {
        public String packageName;
        public int packageUid;
        Map<String, Consumption> networkTypeConsumptionMap;

        public Package(String name, int uid) {
            this.packageName = name;
            this.packageUid = uid;
            this.networkTypeConsumptionMap = new HashMap<>();
        }
    }

    // Class to denote the type of network. Currently supported types are WIFI and MOBILE
    public enum NetworkType {
        WIFI("WIFI"),
        MOBILE("MOBILE");

        private String name;

        NetworkType(String name) {
            this.name = name;
        }
    }

    // Class to denote network consumption of a package
    public class Consumption {
        private long rxBytes;
        private long txBytes;
        private long totalBytes;

        public Consumption(long rxBytes, long txBytes, long totalBytes) {
            this.rxBytes = rxBytes;
            this.txBytes = txBytes;
            this.totalBytes = totalBytes;
        }

        public void setRxBytes(long rxBytes) {
            this.rxBytes = rxBytes;
        }

        public void setTxBytes(long txBytes) {
            this.txBytes = txBytes;
        }

        public void setTotalBytes(long totalBytes) {
            this.totalBytes = totalBytes;
        }
    }

    public DeviceUsageHandler(Context context) {
        mContext = context;
        mPackageName = AACSConstants.getAACSPackageName(new WeakReference<>(context));
    }

    public void startNetworkStatsProvider() {
        ApplicationInfo applicationInfo = null;
        PackageManager manager = mContext.getPackageManager();
        try {
            applicationInfo = manager.getApplicationInfo(mPackageName, 0);
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, String.format("startNetworkStatsProvider failed because %s", e.getMessage()));
            return;
        }
        // Create the package object
        mAlexaApplicationPackage = new Package(mPackageName, applicationInfo.uid);
        mExecutor = Executors.newScheduledThreadPool(1);

        mNetworkStatsManagerRunner = new NetworkStatsManagerRunner(this, mContext, mAlexaApplicationPackage.packageUid);
        mExecutor.scheduleAtFixedRate(mNetworkStatsManagerRunner, 0, TIME_INTERVAL, TimeUnit.MINUTES);
    }

    public void stopNetworkStatsProvider() {
        mExecutor.shutdown();
        mAlexaApplicationPackage.networkTypeConsumptionMap.clear();
    }

    public void reportUsage(NetworkType networkInterfaceType, long currentRxBytes, long currentTxBytes,
            long currentTotalBytes, long startTime, long endTime) {
        Consumption consumptionToReport =
                calculateNetworkConsumption(networkInterfaceType, currentRxBytes, currentTxBytes, currentTotalBytes);
        if (consumptionToReport != null && networkInterfaceType != null) {
            Log.v(TAG, "Reporting data usage to engine");
            reportNetworkDataUsage(consumptionToReport, networkInterfaceType.name, startTime, endTime);
        }
    }

    private Consumption calculateNetworkConsumption(
            NetworkType networkInterfaceType, long currentRxBytes, long currentTxBytes, long currentTotalBytes) {
        Consumption consumptionDiff = null;

        /**
         * Check if the network type consumption is already calculated for the specified package object
         * If not, create the entry for the network consumption of the specified type.
         * NOTE : Do not report this to engine since the first network consumption of the NetworkStatsManager
         * contains historical consumption even before device boot up.
         */
        if (!mAlexaApplicationPackage.networkTypeConsumptionMap.containsKey(networkInterfaceType.name)) {
            mAlexaApplicationPackage.networkTypeConsumptionMap.put(
                    networkInterfaceType.name, new Consumption(currentRxBytes, currentTxBytes, currentTotalBytes));
            Log.v(TAG, "calculateNetworkConsumption: Creating new consumption");
        } else {
            // The consumption is already reported earlier. Get the previous consumption, subtract the previous
            // consumption from the current consumption to get the network data consumed in the specified time interval
            // ( i.e TIME_INTERVAL)
            Consumption currentConsumption =
                    mAlexaApplicationPackage.networkTypeConsumptionMap.get(networkInterfaceType.name);
            consumptionDiff = new Consumption(Math.abs(currentConsumption.rxBytes - currentRxBytes),
                    Math.abs(currentConsumption.txBytes - currentTxBytes),
                    Math.abs(currentConsumption.totalBytes - currentTotalBytes));
            Log.i(TAG, "calculateNetworkConsumption: Updating current consumption");

            // Update the new current consumed bytes in the map
            mAlexaApplicationPackage.networkTypeConsumptionMap.put(
                    networkInterfaceType.name, new Consumption(currentRxBytes, currentTxBytes, currentTotalBytes));
        }
        return consumptionDiff;
    }

    private void reportNetworkDataUsage(
            Consumption consumptionToReport, String networkInterfaceType, long startTime, long endTime) {
        try {
            JSONObject dataUsage = new JSONObject();
            JSONObject byteUsage = new JSONObject();

            // Create the JSON payload
            dataUsage.put("startTimeStamp", startTime);
            dataUsage.put("endTimeStamp", endTime);
            dataUsage.put("networkInterfaceType", networkInterfaceType);
            byteUsage.put("rxBytes", consumptionToReport.rxBytes);
            byteUsage.put("txBytes", consumptionToReport.txBytes);
            dataUsage.put("bytesUsage", byteUsage);

            JSONObject payload = new JSONObject();
            payload.put("usage", dataUsage.toString());
            Log.v(TAG, "reportNetworkDataUsage:" + payload.toString());
            new AACSMessageSender(new WeakReference<>(mContext), new AACSSender())
                    .sendMessage(Topic.DEVICE_USAGE, Action.DeviceUsage.REPORT_NETWORK_DATA_USAGE, payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, String.format("reportNetworkDataUsage: failed to create usage json because %s", e.getMessage()));
        }
    }
}
