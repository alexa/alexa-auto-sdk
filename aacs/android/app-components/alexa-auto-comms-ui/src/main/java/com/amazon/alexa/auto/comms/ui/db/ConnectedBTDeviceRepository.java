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

public class ConnectedBTDeviceRepository {
    private String CONNECTED_DB_NAME = "db_connected_bt_device";
    private String DB_NAME = "db_bt_device";
    private ConnectedBTDeviceDatabase connectedBTDeviceDatabase;
    private BTDeviceDatabase btDeviceDatabase;
    private Handler mHandler;

    private static ConnectedBTDeviceRepository sInstance;
    private String TAG = ConnectedBTDeviceRepository.class.getSimpleName();

    public static ConnectedBTDeviceRepository getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new ConnectedBTDeviceRepository(context);
        }
        return sInstance;
    }

    private ConnectedBTDeviceRepository(Context context) {
        connectedBTDeviceDatabase =
                Room.databaseBuilder(context, ConnectedBTDeviceDatabase.class, CONNECTED_DB_NAME).build();
        btDeviceDatabase = Room.databaseBuilder(context, BTDeviceDatabase.class, DB_NAME).build();

        mHandler = new Handler(Looper.getMainLooper());
    }

    public LiveData<List<ConnectedBTDevice>> getDescConnectedDevices() {
        return connectedBTDeviceDatabase.connectedBTDeviceDao().getDescConnectedDevices();
    }

    public LiveData<List<ConnectedBTDevice>> getConnectedDevices() {
        return connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDevices();
    }

    public List<ConnectedBTDevice> getConnectedDevicesSync() {
        return connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDevicesSync();
    }

    public ConnectedBTDevice getConnectedDeviceByAddressSync(String deviceAddress) {
        return connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(deviceAddress);
    }

    public void setConnectedDeviceToPrimary(String deviceAddress) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                Log.i(TAG, "setConnectedDeviceToPrimary: " + deviceAddress);
                ConnectedBTDevice previousRecord =
                        connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(deviceAddress);
                if (previousRecord != null) {
                    connectedBTDeviceDatabase.connectedBTDeviceDao().deleteConnectedBTDevice(previousRecord);
                    ConnectedBTDevice connectedBTDevice = new ConnectedBTDevice();
                    connectedBTDevice.setDeviceAddress(previousRecord.getDeviceAddress());
                    connectedBTDevice.setDeviceName(previousRecord.getDeviceName());
                    connectedBTDevice.setContactsUploadPermission(previousRecord.getContactsUploadPermission());
                    String permission = btDeviceDatabase.btDeviceDao()
                                                .getBTDeviceByAddressSync(deviceAddress)
                                                .getContactsUploadPermission();
                    Log.i(TAG, "Inserting bt connected device entry: " + connectedBTDevice.getDeviceAddress());
                    connectedBTDeviceDatabase.connectedBTDeviceDao().insertConnectedBTDevice(connectedBTDevice);
                    EventBus.getDefault().post(new PrimaryPhoneChangeMessage(connectedBTDevice, false, permission));
                }
                return null;
            }
        }.execute();
    }

    public void insertEntry(final BTDevice device) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                boolean isNewDevice = true;
                if (connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(
                            device.getDeviceAddress())
                        != null) {
                    ConnectedBTDevice previousRecord =
                            connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(
                                    device.getDeviceAddress());
                    connectedBTDeviceDatabase.connectedBTDeviceDao().deleteConnectedBTDevice(previousRecord);
                    Log.i(TAG, "Deleting previous bt record: " + device.getDeviceAddress());
                    isNewDevice = false;
                }

                BTDevice btDevice = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(device.getDeviceAddress());
                String permission = btDevice.getContactsUploadPermission();
                ConnectedBTDevice connectedBTDevice = new ConnectedBTDevice();
                connectedBTDevice.setDeviceAddress(btDevice.getDeviceAddress());
                connectedBTDevice.setDeviceName(btDevice.getDeviceName());
                connectedBTDevice.setContactsUploadPermission(btDevice.getContactsUploadPermission());
                Log.i(TAG, "Inserting bt connected device entry: " + connectedBTDevice.getDeviceAddress());
                connectedBTDeviceDatabase.connectedBTDeviceDao().insertConnectedBTDevice(connectedBTDevice);
                EventBus.getDefault().post(new PrimaryPhoneChangeMessage(connectedBTDevice, isNewDevice, permission));
                return null;
            }
        }.execute();
    }

    public void deleteEntry(final ConnectedBTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                connectedBTDeviceDatabase.connectedBTDeviceDao().deleteConnectedBTDevice(entry);
                List<ConnectedBTDevice> listData = getConnectedDevicesSync();
                if (listData != null && listData.size() > 0) {
                    int index = listData.size() - 1;
                    BTDevice btDevice = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(
                            listData.get(index).getDeviceAddress());
                    String permission = btDevice.getContactsUploadPermission();
                    EventBus.getDefault().post(new PrimaryPhoneChangeMessage(listData.get(index), false, permission));
                } else {
                    EventBus.getDefault().post(
                            new PrimaryPhoneChangeMessage(null, false, Constants.CONTACTS_PERMISSION_NO));
                }
                return null;
            }
        }.execute();
    }

    public void updateContactsPermission(final String deviceAddress, final String permission) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                ConnectedBTDevice device =
                        connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(deviceAddress);
                if (device != null) {
                    device.setContactsUploadPermission(permission);
                    connectedBTDeviceDatabase.connectedBTDeviceDao().updateConnectedBTDevice(device);
                } else {
                    Log.e(TAG, "Device not found in ConnectedBTDeviceRepository");
                }
                return null;
            }
        }.execute();
    }

    public void findConnectedBTDeviceEntry(BTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                ConnectedBTDevice targetDevice =
                        connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(
                                entry.getDeviceAddress());
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (targetDevice != null) {
                            EventBus.getDefault().post(new ConnectedBTDeviceDiscoveryMessage(targetDevice, true));
                        } else {
                            EventBus.getDefault().post(new ConnectedBTDeviceDiscoveryMessage(null, false));
                        }
                    }
                });
                return null;
            }
        }.execute();
    }

    /**
     * Connected Bluetooth Device Discovery Message
     */
    public static class ConnectedBTDeviceDiscoveryMessage {
        private ConnectedBTDevice device;
        private boolean isFound;

        public ConnectedBTDeviceDiscoveryMessage(ConnectedBTDevice device, boolean isFound) {
            this.device = device;
            this.isFound = isFound;
        }

        public ConnectedBTDevice getBConnectedBTDevice() {
            return this.device;
        }

        public Boolean isFound() {
            return this.isFound;
        }
    }

    /**
     * Primary Phone Change Discovery Message
     */
    public static class PrimaryPhoneChangeMessage {
        private ConnectedBTDevice device;
        private boolean isNewDevice;
        private String contactsPermission;
        public PrimaryPhoneChangeMessage(ConnectedBTDevice device, boolean newDevice, String contactsPermission) {
            this.isNewDevice = newDevice;
            this.device = device;
            this.contactsPermission = contactsPermission;
        }
        public ConnectedBTDevice getConnectedBTDevice() {
            return this.device;
        }
        public boolean getIsNewDevice() {
            return this.isNewDevice;
        }
        public String getContactsPermission() {
            return this.contactsPermission;
        }
    }
}
