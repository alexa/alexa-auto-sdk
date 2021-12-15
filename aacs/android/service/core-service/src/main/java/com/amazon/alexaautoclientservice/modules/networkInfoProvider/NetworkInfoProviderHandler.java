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

package com.amazon.alexaautoclientservice.modules.networkInfoProvider;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.network.NetworkInfoProvider;
import com.amazon.aacsconstants.AACSConstants;

public class NetworkInfoProviderHandler extends NetworkInfoProvider {
    private static final String TAG = AACSConstants.AACS + "-" + NetworkInfoProvider.class.getSimpleName();

    private final Context mContext;
    private final WifiManager mWifiManager;
    private final ConnectivityManager mConnectivityManager;
    private final NetworkChangeReceiver mReceiver;
    private NetworkStatus mStatus;

    public NetworkInfoProviderHandler(@NonNull Context context) {
        mStatus = NetworkStatus.UNKNOWN;
        mContext = context;
        mReceiver = new NetworkChangeReceiver();

        mContext.registerReceiver(mReceiver, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));

        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        mConnectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);

        updateNetworkStatus();
    }

    @Override
    public NetworkStatus getNetworkStatus() {
        return mStatus;
    }

    @Override
    public int getWifiSignalStrength() {
        return mWifiManager.getConnectionInfo().getRssi();
    }

    public class NetworkChangeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(final Context context, final Intent intent) {
            if (mConnectivityManager != null) {
                updateNetworkStatus();
                int rssi = mWifiManager.getConnectionInfo().getRssi();

                Log.i(TAG, String.format("Network status changed. STATUS: %s, RSSI: %s", mStatus, rssi));
                networkStatusChanged(mStatus, rssi);
            }
        }
    }

    public void unregister() {
        mContext.unregisterReceiver(mReceiver);
    }

    private void updateNetworkStatus() {
        NetworkInfo activeNetwork = mConnectivityManager.getActiveNetworkInfo();
        if (activeNetwork != null) {
            NetworkInfo.State state = activeNetwork.getState();
            switch (state) {
                case CONNECTED:
                    mStatus = NetworkStatus.CONNECTED;
                    break;
                case CONNECTING:
                    mStatus = NetworkStatus.CONNECTING;
                    break;
                case DISCONNECTING:
                    mStatus = NetworkStatus.DISCONNECTING;
                    break;
                case DISCONNECTED:
                case SUSPENDED:
                    mStatus = NetworkStatus.DISCONNECTED;
                    break;
                case UNKNOWN:
                    mStatus = NetworkStatus.UNKNOWN;
                    break;
            }
        } else {
            mStatus = NetworkStatus.UNKNOWN;
        }
    }
}
