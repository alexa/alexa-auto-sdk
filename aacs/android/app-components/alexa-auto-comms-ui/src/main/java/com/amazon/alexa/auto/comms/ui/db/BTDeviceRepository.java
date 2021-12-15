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
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (targetDevice != null) {
                                EventBus.getDefault().post(new BTDeviceDiscoveryMessage(targetDevice, true));
                            }
                        }
                    });
                } else {
                    Log.d(TAG, "There is no connected device.");
                    EventBus.getDefault().post(new BTDeviceDiscoveryMessage(null, false));
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

        public BTDeviceDiscoveryMessage(BTDevice device, boolean isFound) {
            this.device = device;
            this.isFound = isFound;
        }

        public BTDevice getBTDevice() {
            return this.device;
        }

        public Boolean isFound() {
            return this.isFound;
        }
    }
}
