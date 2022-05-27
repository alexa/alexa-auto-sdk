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

package com.amazon.aacstelephony;

import static com.amazon.aacstelephony.Constants.HEADSET_CLIENT_PROFILE_ID;
import static com.amazon.aacstelephony.Constants.PBAP_CLIENT_PROFILE_ID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import java.util.Set;

public class BluetoothStateListener extends BroadcastReceiver {
    private static final String TAG = AACSConstants.AACS + "-" + BluetoothStateListener.class.getSimpleName();
    private AACSMessageSender mAACSMessageSender;

    public BluetoothStateListener(@NonNull AACSMessageSender aacsMessageSender) {
        mAACSMessageSender = aacsMessageSender;
    }

    public void initialConnectionCheck(@NonNull Context context) {
        Log.d(TAG, "initialConnectionCheck");
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Log.e(TAG,
                    "Could not get the default Bluetooth adapter. Please check if Bluetooth is supported on your device.");
            return;
        }
        Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
        if (pairedDevices.size() > 0) {
            for (BluetoothDevice device : pairedDevices) {
                Log.i(TAG, "Adding devices paired before initial check to database");
                Util.broadcastPairedDevices(context, device.getName(), device.getAddress());
            }
        }

        if (bluetoothAdapter.getProfileConnectionState(HEADSET_CLIENT_PROFILE_ID) == BluetoothAdapter.STATE_CONNECTED) {
            Log.i(TAG, "Bluetooth connected on AACS boot up, sending ConnectionStateChanged message to AACS");
            Util.publishConnectionStateToAACS(Constants.ConnectionState.CONNECTED, mAACSMessageSender);
        }

        if (bluetoothAdapter.getProfileConnectionState(PBAP_CLIENT_PROFILE_ID) == BluetoothAdapter.STATE_CONNECTED) {
            if (pairedDevices.size() == 0)
                Log.i(TAG, "No devices connected/paired");

            for (BluetoothDevice device : pairedDevices) {
                Util.broadcastBluetoothState(context, true, device.getName(), device.getAddress());
            }
            Util.broadcastConnectionCheckCompleted(context);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "action: " + action);
        if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            Log.d(TAG, "bonded state: " + device.getBondState());
            Util.broadcastBondState(context, device.getName(), device.getAddress(), device.getBondState());
        }

        if (Constants.ACTION_BLUETOOTH_PBAP_CLIENT_STATE_CHANGED.equals(action)) {
            // report PBAP connection state changes to listeners
            boolean isConnected = BluetoothAdapter.STATE_CONNECTED
                    == intent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.STATE_DISCONNECTED);
            BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            Util.broadcastBluetoothState(context, isConnected, device.getName(), device.getAddress());

        } else if (Constants.ACTION_BLUETOOTH_HFP_CLIENT_STATE_CHANGED.equals(action)) {
            boolean isConnected = BluetoothAdapter.STATE_CONNECTED
                    == intent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.STATE_DISCONNECTED);

            if (isConnected) {
                Log.v(TAG, "Bluetooth HFP connected, sending ConnectionStateChanged message to AACS");
                Util.publishConnectionStateToAACS(Constants.ConnectionState.CONNECTED, mAACSMessageSender);
            } else {
                BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
                if (bluetoothAdapter == null) {
                    Log.e(TAG,
                            "Could not get the default Bluetooth adapter. Please check if Bluetooth is supported on your device.");
                    return;
                }
                // Only report disconnection when no any devices connected through HFP
                if (bluetoothAdapter.getProfileConnectionState(HEADSET_CLIENT_PROFILE_ID)
                        == BluetoothAdapter.STATE_DISCONNECTED) {
                    Log.i(TAG, "Bluetooth HFP disconnected, sending ConnectionStateChanged message to AACS");
                    Util.publishConnectionStateToAACS(Constants.ConnectionState.DISCONNECTED, mAACSMessageSender);
                }
            }
        }
    }
}
