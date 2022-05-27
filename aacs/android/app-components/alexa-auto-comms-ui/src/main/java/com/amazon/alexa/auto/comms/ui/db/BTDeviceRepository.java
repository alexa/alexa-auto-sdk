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
package com.amazon.alexa.auto.comms.ui.db;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.lifecycle.LiveData;
import androidx.room.Room;

import com.amazon.alexa.auto.comms.ui.Constants;

import org.greenrobot.eventbus.EventBus;

import java.util.List;

public class BTDeviceRepository {
    private String DB_NAME = "db_bt_device";
    private String CONNECTED_DEVICE_DB_NAME = "db_connected_bt_device";
    private BTDeviceDatabase btDeviceDatabase;
    private ConnectedBTDeviceDatabase connectedBTDeviceDatabase;

    private Handler mHandler = new Handler(Looper.getMainLooper());

    private static BTDeviceRepository sInstance;
    private String TAG = BTDeviceRepository.class.getSimpleName();

    public static BTDeviceRepository getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new BTDeviceRepository(context);
        }
        return sInstance;
    }

    public BTDeviceRepository(Context context) {
        btDeviceDatabase = Room.databaseBuilder(context, BTDeviceDatabase.class, DB_NAME).build();
        connectedBTDeviceDatabase =
                Room.databaseBuilder(context, ConnectedBTDeviceDatabase.class, CONNECTED_DEVICE_DB_NAME).build();
    }

    public void insertEntry(String deviceAddress, String deviceName, String connectionState,
            String contactUploadPermission, boolean isPrimaryDevice) {
        BTDevice device = new BTDevice();
        device.setDeviceAddress(deviceAddress);
        device.setDeviceName(deviceName);
        device.setContactsUploadPermission(contactUploadPermission);
        insertEntry(device);
    }

    public LiveData<BTDevice> getBTDeviceByAddress(String deviceAddress) {
        return btDeviceDatabase.btDeviceDao().getBTDeviceByAddress(deviceAddress);
    }

    public List<BTDevice> getBTDevices() {
        return btDeviceDatabase.btDeviceDao().getDevicesSync();
    }

    public void shouldRequestContactsConsent(final BTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                BTDevice device = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(entry.getDeviceAddress());
                if (device != null) {
                    Log.i(TAG, "Device is paired, checking contacts consent permission");
                    String permission = device.getContactsUploadPermission();
                    Boolean firstPair = device.getFirstPair();
                    if (permission.equals(Constants.CONTACTS_PERMISSION_NO) && firstPair == true) {
                        Log.d(TAG, "Contacts consent not granted, popping up contacts consent screen");
                        mHandler.post(
                                () -> EventBus.getDefault().post(new BTDeviceDiscoveryMessage(device, true, true)));
                        device.setFirstPair(false);
                        btDeviceDatabase.btDeviceDao().updateBTDevice(device);
                    }
                } else {
                    Log.w(TAG,
                            String.format("BTDevice with address=%s does not exist in the database",
                                    entry.getDeviceAddress()));
                }
                return null;
            }
        }.execute();
    }

    public void insertEntry(final BTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                if (btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(entry.getDeviceAddress()) != null) {
                    Log.i(TAG, "bt device entry already exists");
                    BTDevice device = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(entry.getDeviceAddress());
                } else {
                    Log.i(TAG, "Inserting bt device entry: " + entry.getDeviceAddress());
                    btDeviceDatabase.btDeviceDao().insertBTDevice(entry);
                }
                return null;
            }
        }.execute();
    }

    public void deleteEntry(final BTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                btDeviceDatabase.btDeviceDao().deleteBTDevice(entry);
                return null;
            }
        }.execute();
    }

    public void updateEntry(final BTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                btDeviceDatabase.btDeviceDao().updateBTDevice(entry);
                return null;
            }
        }.execute();
    }

    public void updateContactsPermission(final String deviceAddress, final String permission) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                BTDevice device = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(deviceAddress);
                device.setContactsUploadPermission(permission);
                btDeviceDatabase.btDeviceDao().updateBTDevice(device);
                return null;
            }
        }.execute();
    }

    public void updateFirstPair(final String deviceAddress, final Boolean firstPair) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                BTDevice device = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(deviceAddress);
                if (device != null) {
                    device.setFirstPair(firstPair);
                    btDeviceDatabase.btDeviceDao().updateBTDevice(device);
                }
                return null;
            }
        }.execute();
    }

    public void findPrimaryBTDeviceEntry() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                List<ConnectedBTDevice> connectedDeviceList =
                        connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDevicesSync();
                if (connectedDeviceList.size() > 0) {
                    // Getting last connected device.
                    BTDevice targetDevice = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(
                            connectedDeviceList.get(connectedDeviceList.size() - 1).getDeviceAddress());
                    mHandler.post(() -> {
                        if (targetDevice != null) {
                            EventBus.getDefault().post(new BTDeviceDiscoveryMessage(targetDevice, true, false));
                        }
                    });
                } else {
                    Log.d(TAG, "There is no connected device.");
                    mHandler.post(() -> EventBus.getDefault().post(new BTDeviceDiscoveryMessage(null, false, false)));
                }
                return null;
            }
        }.execute();
    }

    /**
     * Bluetooth Device Discovery Message
     */
    public static class BTDeviceDiscoveryMessage {
        private BTDevice device;
        private boolean isFound;
        private boolean firstPair;

        public BTDeviceDiscoveryMessage(BTDevice device, boolean isFound, boolean firstPair) {
            this.device = device;
            this.isFound = isFound;
            this.firstPair = firstPair;
        }

        public Boolean getFirstPair() {
            return this.firstPair;
        }

        public BTDevice getBTDevice() {
            return this.device;
        }

        public Boolean isFound() {
            return this.isFound;
        }
    }
}
