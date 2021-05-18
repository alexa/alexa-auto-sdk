package com.amazon.alexa.auto.comms.ui.db;

import android.content.Context;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.lifecycle.LiveData;
import androidx.room.Room;

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

    public void insertEntry(final BTDevice device) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                if (connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(
                            device.getDeviceAddress())
                        != null) {
                    ConnectedBTDevice previousRecord =
                            connectedBTDeviceDatabase.connectedBTDeviceDao().getConnectedDeviceByAddress(
                                    device.getDeviceAddress());
                    connectedBTDeviceDatabase.connectedBTDeviceDao().deleteConnectedBTDevice(previousRecord);
                }

                BTDevice btDevice = btDeviceDatabase.btDeviceDao().getBTDeviceByAddressSync(device.getDeviceAddress());

                ConnectedBTDevice connectedBTDevice = new ConnectedBTDevice();
                connectedBTDevice.setDeviceAddress(btDevice.getDeviceAddress());
                connectedBTDevice.setDeviceName(btDevice.getDeviceName());
                connectedBTDevice.setContactsUploadPermission(btDevice.getContactsUploadPermission());
                Log.i(TAG, "Inserting bt connected device entry: " + connectedBTDevice.getDeviceAddress());
                connectedBTDeviceDatabase.connectedBTDeviceDao().insertConnectedBTDevice(connectedBTDevice);

                return null;
            }
        }.execute();
    }

    public void deleteEntry(final ConnectedBTDevice entry) {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                connectedBTDeviceDatabase.connectedBTDeviceDao().deleteConnectedBTDevice(entry);
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
                device.setContactsUploadPermission(permission);
                connectedBTDeviceDatabase.connectedBTDeviceDao().updateConnectedBTDevice(device);
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
}
