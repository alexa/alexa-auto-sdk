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
package com.amazon.alexa.auto.comms.ui.handler;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.comms.ui.Constants;
import com.amazon.alexa.auto.comms.ui.ContactsControllerImpl;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;

import java.lang.ref.WeakReference;

/**
 * Handler for bluetooth directives coming from AACS.
 */
public class BluetoothDirectiveHandler {
    private final static String TAG = BluetoothDirectiveHandler.class.getSimpleName();

    Context mContext;
    BTDeviceRepository mBTDeviceRepository;
    ConnectedBTDeviceRepository mConnectedBTDeviceRepository;

    /**
     * Constructs the bluetooth commands handler.
     */
    public BluetoothDirectiveHandler(WeakReference<Context> context) {
        mContext = context.get();

        AlexaApp mApp = AlexaApp.from(mContext);
        if (!mApp.getRootComponent().getComponent(ContactsController.class).isPresent()) {
            mApp.getRootComponent().activateScope(new ContactsControllerImpl(context));
        }

        mBTDeviceRepository = BTDeviceRepository.getInstance(mContext);
        mConnectedBTDeviceRepository = ConnectedBTDeviceRepository.getInstance(mContext);
    }

    /**
     * Constructs the bluetooth commands handler.
     * This is created for unit testing.
     */
    @VisibleForTesting
    BluetoothDirectiveHandler(
            BTDeviceRepository btDeviceRepository, ConnectedBTDeviceRepository connectedBTDeviceRepository) {
        mBTDeviceRepository = btDeviceRepository;
        mConnectedBTDeviceRepository = connectedBTDeviceRepository;
    }

    /**
     * Handle bond state for BOND_BONDED (paired) to update the BTDevice. The 'firstPair' flag in
     * the BTDevice is used to distinguish between when a device is being paired versus connected.
     * The 'firstPair' flag in the BTDevice is set to false when the BOND_BONDED intent is received
     * for the device. Once it is set to false, it prevents the contact consent popup from showing
     * up in subsequent disconnect/reconnects.
     */
    public void handleBondStateChange(@NonNull BTDevice device, Integer bondState) {
        String deviceAddress = device.getDeviceAddress();
        Log.d(TAG, "BTHandler bond state: " + bondState);
        if (bondState == BluetoothDevice.BOND_BONDED) {
            Log.d(TAG, "Device bonded, update device on first pair");
            mBTDeviceRepository.insertEntry(device);
            mBTDeviceRepository.updateFirstPair(deviceAddress, true);
        } else if (bondState == BluetoothDevice.BOND_NONE ) {
            mBTDeviceRepository.updateContactsPermission(deviceAddress, Constants.CONTACTS_PERMISSION_NO);
        }
    }
    /**
     * Handle bluetooth connection directive coming from AACS.
     *
     * @param device BT device.
     */

    public void handleBTConnectionCommand(@NonNull BTDevice device, String connectedState) {
        if (connectedState.equals(Constants.BT_CONNECTED)) {
            mBTDeviceRepository.insertEntry(device);
            mConnectedBTDeviceRepository.insertEntry(device);
            mBTDeviceRepository.shouldRequestContactsConsent(device);
        } else {
            // Device is disconnected, it needs to be removed from connected device database, when found, the remove
            // action will be triggered.
            mConnectedBTDeviceRepository.findConnectedBTDeviceEntry(device);
        }
    }

    public void handlePrimaryPhoneChangedCommand(String deviceAddress) {
        if (deviceAddress != null && !deviceAddress.isEmpty()) {
            mConnectedBTDeviceRepository.setConnectedDeviceToPrimary(deviceAddress);
        }
    }

    public void insertPairedDevice(@NonNull BTDevice device) {
        mBTDeviceRepository.insertEntry(device);
    }
}
