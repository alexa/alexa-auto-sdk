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

package com.amazon.sampleapp.impl.DeviceUsage;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import androidx.annotation.RequiresApi;

import com.amazon.aace.core.PlatformInterface;
import com.amazon.aace.deviceUsage.DeviceUsage;
import com.amazon.sampleapp.NetworkStatsManagerRunner;
import com.amazon.sampleapp.connectivity.AlexaConnectivityHandler;
import com.amazon.sampleapp.core.EngineStatusListener;
import com.amazon.sampleapp.core.SampleAppContext;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class DeviceUsageHandler extends DeviceUsage implements EngineStatusListener {
    private static final String sTag = "DeviceUsageHandler";

    // The package name of the application using Alexa
    private final String packageName = "com.amazon.sampleapp";

    // The time interval in seconds before the application reports its usage to the Engine every time.
    private final int TIME_INTERVAL = 300;

    private final Activity mActivity;
    private final SampleAppContext mSampleAppContext;
    private final LoggerHandler mLogger;

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

    public DeviceUsageHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mSampleAppContext = (SampleAppContext) activity;
    }

    @RequiresApi(api = Build.VERSION_CODES.M)
    private void startNetworkStatsProvider() {
        Context context = mActivity.getApplicationContext();
        ApplicationInfo applicationInfo = null;
        PackageManager manager = context.getPackageManager();
        try {
            applicationInfo = manager.getApplicationInfo(packageName, 0);
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            return;
        }
        // Create the package object
        mAlexaApplicationPackage = new Package(packageName, applicationInfo.uid);
        mExecutor = Executors.newScheduledThreadPool(1);

        // NetworkStatsManager requires minimum API level as M
        mNetworkStatsManagerRunner =
                new NetworkStatsManagerRunner(mActivity, this, context, mAlexaApplicationPackage.packageUid);
        mExecutor.scheduleAtFixedRate(mNetworkStatsManagerRunner, 5, TIME_INTERVAL, TimeUnit.SECONDS);
    }

    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    public void onEngineStart() {
        // Start the network stats provider
        startNetworkStatsProvider();
    }

    @Override
    public void onEngineStop() {
        Log.i("onEngineStop", " shutdown");
        mExecutor.shutdown();
        mAlexaApplicationPackage.networkTypeConsumptionMap.clear();
    }

    public void reportUsage(NetworkType networkInterfaceType, long currentRxBytes, long currentTxBytes,
            long currentTotalBytes, long startTime, long endTime) {
        Consumption consumptionToReport =
                calculateNetworkConsumption(networkInterfaceType, currentRxBytes, currentTxBytes, currentTotalBytes);
        if (consumptionToReport != null && networkInterfaceType != null) {
            Log.i("ReportUsage", "Reporting data usage to engine");
            reportDataUsage(consumptionToReport, networkInterfaceType.name, startTime, endTime);
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
            Log.i(sTag, "Creating new consumption");
        } else {
            // The consumption is already reported earlier. Get the previous consumption, subtract the previous
            // consumption from the current consumption to get the network data consumed in the specified time interval
            // ( i.e TIME_INTERVAL)
            Consumption currentConsumption =
                    mAlexaApplicationPackage.networkTypeConsumptionMap.get(networkInterfaceType.name);
            consumptionDiff = new Consumption(Math.abs(currentConsumption.rxBytes - currentRxBytes),
                    Math.abs(currentConsumption.txBytes - currentTxBytes),
                    Math.abs(currentConsumption.totalBytes - currentTotalBytes));
            Log.i(sTag, "Updating current consumption");

            // Update the new current consumed bytes in the map
            mAlexaApplicationPackage.networkTypeConsumptionMap.put(
                    networkInterfaceType.name, new Consumption(currentRxBytes, currentTxBytes, currentTotalBytes));
        }
        return consumptionDiff;
    }

    private void reportDataUsage(
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

            // Platform implementations that have registered the AlexaConnectivity platform interface are required to
            // provide the type of data plan the device is currently on. The dataPlanType field should be populated with
            // the data plan type retrieved from the AlexaConnectivity platform implementation. The dataPlanType is an
            // optional field and should not be provided in the payload if AlexaConnectivity platform is not registered.
            // Refer to AlexaConnectivity documentation for more details on dataPlanType. Additionally, dataPlanType is
            // only reported if the managedProvider type is MANAGED
            AlexaConnectivityHandler alexaConnectivityHandler =
                    (AlexaConnectivityHandler) mSampleAppContext.getPlatformInterfaceHandler("AlexaConnectivity");
            if (alexaConnectivityHandler != null) {
                String connectivityState = alexaConnectivityHandler.getConnectivityState();
                if (!connectivityState.isEmpty()) {
                    JSONObject connectivityStateJSON = new JSONObject(connectivityState);
                    JSONObject managedProvider = connectivityStateJSON.getJSONObject("managedProvider");
                    String type = managedProvider.getString("type");
                    JSONObject dataPlan = connectivityStateJSON.getJSONObject("dataPlan");
                    String dataPlanType = dataPlan.getString("type");
                    if (type.equals("MANAGED") && !dataPlanType.isEmpty()) {
                        dataUsage.put("dataPlanType", dataPlanType);
                    }
                }
            }
            Log.i(sTag, "Device usage is " + dataUsage.toString());
            reportNetworkDataUsage(dataUsage.toString());
        } catch (JSONException e) {
            Log.e(sTag, "reportDataUsage", e);
        }
    }
}
