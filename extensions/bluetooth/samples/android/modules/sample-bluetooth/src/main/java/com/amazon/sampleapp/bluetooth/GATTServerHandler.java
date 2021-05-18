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

package com.amazon.sampleapp.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.os.ParcelUuid;
import android.util.Log;

import com.amazon.aace.bluetooth.GATTServer;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class GATTServerHandler extends GATTServer {
    private static final String TAG = GATTServerHandler.class.getSimpleName();

    private final Context mContext;

    private BluetoothManager mBluetoothManager;
    private BluetoothGattServer mGattServer;
    private List<BluetoothGattService> mGattServices = new ArrayList<>();
    private BluetoothLeAdvertiser mAdvertiser;
    private BluetoothDevice mBluetoothDevice;

    public GATTServerHandler(Context context) {
        mContext = context;
        mBluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
    }

    @Override
    public boolean start(String configuration) {
        Log.v(TAG, "configuration=" + configuration);

        try {
            // create the gatt server
            mGattServer = mBluetoothManager.openGattServer(mContext, mGattServerCallback);

            AdvertiseSettings.Builder advertiseSettings =
                    new AdvertiseSettings.Builder()
                            .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY)
                            .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH)
                            .setConnectable(true);

            AdvertiseData.Builder advertiseData = new AdvertiseData.Builder().setIncludeDeviceName(false);

            // parse the json services
            JSONObject configurationRoot = new JSONObject(configuration);
            JSONArray servicesList = configurationRoot.getJSONArray("services");

            for (int j = 0; j < servicesList.length(); j++) {
                JSONObject nextServiceRoot = servicesList.getJSONObject(j);
                String serviceId = nextServiceRoot.getString("id");
                JSONObject serviceConfig = nextServiceRoot.getJSONObject("configuration");
                JSONArray charcteristicsList = serviceConfig.getJSONArray("characteristics");

                BluetoothGattService gattService =
                        new BluetoothGattService(UUID.fromString(serviceId), BluetoothGattService.SERVICE_TYPE_PRIMARY);

                for (int i = 0; i < charcteristicsList.length(); i++) {
                    JSONObject nextCharacteristicRoot = charcteristicsList.getJSONObject(i);
                    String characteristicId = nextCharacteristicRoot.getString("id");
                    JSONArray propertyList = nextCharacteristicRoot.getJSONArray("properties");
                    JSONArray permissionList = nextCharacteristicRoot.getJSONArray("permissions");

                    BluetoothGattCharacteristic gattCharacteristic =
                            new BluetoothGattCharacteristic(UUID.fromString(characteristicId),
                                    getPropertiesFromJson(propertyList), getPermissionsFromJson(permissionList));

                    // add descriptors
                    if (nextCharacteristicRoot.has("descriptors")) {
                        JSONArray descriptorsList = nextCharacteristicRoot.getJSONArray("descriptors");

                        for (int x = 0; x < descriptorsList.length(); x++) {
                            JSONObject nextDescriptorRoot = descriptorsList.getJSONObject(x);
                            String descriptorId = nextDescriptorRoot.getString("id");
                            JSONArray descriptorPermissionList = nextDescriptorRoot.getJSONArray("permissions");

                            BluetoothGattDescriptor gattDescriptor = new BluetoothGattDescriptor(
                                    UUID.fromString(descriptorId), getPermissionsFromJson(descriptorPermissionList));

                            gattCharacteristic.addDescriptor(gattDescriptor);
                        }
                    }

                    gattService.addCharacteristic(gattCharacteristic);
                }

                Log.i(TAG, "add GATT service " + gattService);
                mGattServer.addService(gattService);
                mGattServices.add(gattService);

                advertiseData.addServiceUuid(ParcelUuid.fromString(serviceId));
            }

            mAdvertiser = BluetoothAdapter.getDefaultAdapter().getBluetoothLeAdvertiser();
            mAdvertiser.startAdvertising(advertiseSettings.build(), advertiseData.build(), mAdvertisingCallback);

            return true;
        } catch (Throwable ex) {
            stop();
            return false;
        }
    }

    private static final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();

    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    @Override
    public boolean setCharacteristicValue(String serviceId, String characteristicId, byte[] data) {
        Log.i(TAG,
                String.format("setCharacteristicValue [serviceId=%s,characteristicId=%s,size=%d,value=%s]", serviceId,
                        characteristicId, data.length, bytesToHex(data)));

        if (mBluetoothDevice == null) {
            Log.e(TAG, "invalidBluetoothDevice");
            return false;
        }

        BluetoothGattCharacteristic characteristic =
                mGattServer.getService(UUID.fromString(serviceId)).getCharacteristic(UUID.fromString(characteristicId));

        if (characteristic != null) {
            characteristic.setValue(data);

            if ((characteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) != 0) {
                if (!mGattServer.notifyCharacteristicChanged(mBluetoothDevice, characteristic, false)) {
                    Log.e(TAG, "notifyCharacteristicChanged failed!");
                    return false;
                }
            }
        } else {
            Log.e(TAG, "invalidCharacteristic=" + characteristicId);
            return false;
        }

        return true;
    }

    private int getPropertiesFromJson(JSONArray propertyList) throws JSONException {
        int properties = 0;

        for (int j = 0; j < propertyList.length(); j++) {
            String prop = propertyList.getString(j).toLowerCase();

            switch (prop) {
                case "read":
                    properties |= BluetoothGattCharacteristic.PROPERTY_READ;
                    break;
                case "write":
                    properties |= BluetoothGattCharacteristic.PROPERTY_WRITE;
                    break;
                case "notify":
                    properties |= BluetoothGattCharacteristic.PROPERTY_NOTIFY;
                    break;
                default:
                    Log.e(TAG, "Unhandled property type: " + prop);
            }
        }

        return properties;
    }

    private int getPermissionsFromJson(JSONArray permissionList) throws JSONException {
        int permissions = 0;

        for (int j = 0; j < permissionList.length(); j++) {
            String perm = permissionList.getString(j).toLowerCase();

            switch (perm) {
                case "read":
                    permissions |= BluetoothGattCharacteristic.PERMISSION_READ;
                    break;
                case "write":
                    permissions |= BluetoothGattCharacteristic.PERMISSION_WRITE;
                    break;
                default:
                    Log.e(TAG, "Unhandled permission type: " + perm);
            }
        }

        return permissions;
    }

    @Override
    public boolean stop() {
        if (mGattServer != null) {
            for (BluetoothGattService s : mGattServices) {
                mGattServer.removeService(s);
                Log.i(TAG, "remove GATT service " + s);
            }
            mGattServices.clear();
            mGattServer.close();
            mGattServer = null;
        }
        return true;
    }

    //
    // BluetoothGattServerCallback
    //
    private final BluetoothGattServerCallback mGattServerCallback = new BluetoothGattServerCallback() {
        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            Log.i(TAG,
                    String.format(
                            "onConnectionStateChange [device=%s,status=%d,newState=%d]", device, status, newState));

            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    mBluetoothDevice = device;
                    connectionStateChanged(device.getAddress(), ConnectionState.CONNECTED);
                    break;
                case BluetoothProfile.STATE_DISCONNECTED:
                    mBluetoothDevice = null;
                    connectionStateChanged(device.getAddress(), ConnectionState.DISCONNECTED);
                    break;
            }
        }

        @Override
        public void onCharacteristicReadRequest(
                BluetoothDevice device, int requestId, int offset, BluetoothGattCharacteristic characteristic) {
            Log.i(TAG,
                    String.format("onCharacteristicReadRequest [device=%s,requestId=%d,offset=%d,characteristic=%s]",
                            device, requestId, offset, characteristic));
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, offset, characteristic.getValue());
        }

        @Override
        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId,
                BluetoothGattCharacteristic characteristic, boolean preparedWrite, boolean responseNeeded, int offset,
                byte[] value) {
            BluetoothGattService service = characteristic.getService();

            Log.i(TAG,
                    String.format(
                            "onCharacteristicWriteRequest [device=%s,requestId=%d,service=%s,characteristic=%s,preparedWrite=%b,responseNeeded=%b,offset=%d,value=%s]",
                            device, requestId, service.getUuid().toString(), characteristic.getUuid().toString(),
                            preparedWrite, responseNeeded, offset, bytesToHex(value)));

            // let the engine impl handle the request
            requestCharacteristic(device.getAddress(), requestId, service.getUuid().toString(),
                    characteristic.getUuid().toString(), value);

            // send the gatt response if requested
            if (responseNeeded) {
                mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, null);
            }
        }

        @Override
        public void onDescriptorReadRequest(
                BluetoothDevice device, int requestId, int offset, BluetoothGattDescriptor descriptor) {
            Log.i(TAG,
                    String.format("onDescriptorReadRequest [device=%s,requestId=%d,offset=%d,descriptor=%s]", device,
                            requestId, offset, descriptor));
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, offset, descriptor.getValue());
        }

        @Override
        public void onDescriptorWriteRequest(BluetoothDevice device, int requestId, BluetoothGattDescriptor descriptor,
                boolean preparedWrite, boolean responseNeeded, int offset, byte[] value) {
            BluetoothGattCharacteristic characteristic = descriptor.getCharacteristic();
            BluetoothGattService service = characteristic.getService();

            Log.i(TAG,
                    String.format(
                            "onDescriptorWriteRequest [device=%s,requestId=%d,service=%s,characteristic=%s,descriptor=%s,preparedWrite=%b,responseNeeded=%b,offset=%d,value=%s]",
                            device.getAddress(), requestId, service.getUuid().toString(),
                            characteristic.getUuid().toString(), descriptor.getUuid().toString(), preparedWrite,
                            responseNeeded, offset, bytesToHex(value)));

            // let the engine impl handle the request
            requestDescriptor(device.getAddress(), requestId, service.getUuid().toString(),
                    characteristic.getUuid().toString(), descriptor.getUuid().toString(), value);

            // send the gatt response if requested
            if (responseNeeded) {
                mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, 0, null);
            }
        }
    };

    //
    // AdvertiseCallback
    //
    private final AdvertiseCallback mAdvertisingCallback = new AdvertiseCallback() {
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
            super.onStartSuccess(settingsInEffect);
            Log.i(TAG, "BLE Advertising Started.");
        }

        @Override
        public void onStartFailure(int errorCode) {
            Log.e(TAG, "BLE Advertising onStartFailure: " + errorCode);
            super.onStartFailure(errorCode);
        }
    };
}
