/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.comms.ui.receiver;

import static com.amazon.alexa.auto.comms.ui.Constants.AACS_BT_CONNECTION_CHECK_COMPLETED;
import static com.amazon.alexa.auto.comms.ui.Constants.ALEXA_AUTO_COMMS_PRIMARY_PHONE_CHANGED;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.util.Log;

import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.dependencies.AndroidModule;
import com.amazon.alexa.auto.comms.ui.dependencies.DaggerCommunicationComponent;
import com.amazon.alexa.auto.comms.ui.handler.BluetoothDirectiveHandler;

import javax.inject.Inject;

/**
 * Receiver that gets Android telephony bluetooth directives.
 */
public class BluetoothReceiver extends BroadcastReceiver {
    private static final String TAG = BluetoothReceiver.class.getSimpleName();

    @VisibleForTesting
    BTDevice mBTDevice = new BTDevice();

    @Inject
    BluetoothDirectiveHandler mBluetoothDirectiveHandler;

    @Inject
    TelecomManager mTelecomManager;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (mBluetoothDirectiveHandler == null) {
            Log.i(TAG, this + " | first onReceive so doing injection");
            DaggerCommunicationComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectBluetoothReceiver(this);
        }

        if (intent.getAction() != null && Constants.ACTION_PAIRED_DEVICE.equals(intent.getAction())) {
            Log.d(TAG, "receiving paired device on initial connection check");
            mBTDevice.setDeviceName(intent.getExtras().getString(Constants.AACS_BT_DEVICE_NAME, ""));
            mBTDevice.setDeviceAddress(intent.getExtras().getString(Constants.AACS_BT_DEVICE_ADDRESS, ""));
            mBluetoothDirectiveHandler.insertPairedDevice(mBTDevice);
            return;
        }

        if (intent.getAction() != null && Constants.AACS_BT_BOND_STATE_CHANGED.equals(intent.getAction())) {
            Log.d(TAG, "intent bond state: " + intent.getExtras().getInt("bondState", -1));
            Integer bondState = intent.getExtras().getInt("bondState", -1);
            mBTDevice.setDeviceAddress(intent.getExtras().getString(Constants.AACS_BT_DEVICE_ADDRESS, ""));
            mBTDevice.setDeviceName(intent.getExtras().getString(Constants.AACS_BT_DEVICE_NAME, ""));
            mBluetoothDirectiveHandler.handleBondStateChange(mBTDevice, bondState);
            return;
        }

        if (intent.getAction() != null
                && (intent.getAction().equals(AACS_BT_CONNECTION_CHECK_COMPLETED)
                        || intent.getAction().equals(ALEXA_AUTO_COMMS_PRIMARY_PHONE_CHANGED))) {
            mBluetoothDirectiveHandler.handlePrimaryPhoneChangedCommand(getPrimaryDevice());
            return;
        }

        if (intent.getAction() != null && intent.getExtras() != null) {
            mBTDevice.setDeviceAddress(intent.getExtras().getString(Constants.AACS_BT_DEVICE_ADDRESS, ""));
            mBTDevice.setDeviceName(intent.getExtras().getString(Constants.AACS_BT_DEVICE_NAME, ""));

            String connectionState;
            if (intent.getAction().equals(Constants.AACS_BT_CONNECTED)) {
                connectionState = Constants.BT_CONNECTED;
            } else {
                connectionState = Constants.BT_DISCONNECTED;
            }
            mBluetoothDirectiveHandler.handleBTConnectionCommand(mBTDevice, connectionState);
        }
    }

    public String getPrimaryDevice() {
        String deviceAddress = "";
        if (mTelecomManager != null) {
            PhoneAccountHandle handle = mTelecomManager.getDefaultOutgoingPhoneAccount(PhoneAccount.SCHEME_TEL);
            if (handle != null) {
                deviceAddress = handle.getId();
            }
        }
        if (deviceAddress.isEmpty() || !BluetoothAdapter.checkBluetoothAddress(deviceAddress)) {
            Log.e(TAG, "cannot find any valid primary device.");
            return null;
        }
        return deviceAddress;
    }
}
