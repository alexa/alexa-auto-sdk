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

package com.amazon.aacstelephony;

import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.net.Uri;
import android.telephony.SmsManager;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;

import java.lang.reflect.Method;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * This class provides functionality to send text message through a connected
 * bluetooth device as well as querying unread messages.
 */
public class BluetoothMapClientManager {
    private static final String TAG = AACSConstants.AACS + "-" + BluetoothMapClientManager.class.getSimpleName();
    private static ExecutorService mExecutor;
    private Class BluetoothMapClient = null;
    private Method mSetMessageStatus = null;

    public BluetoothMapClientManager() {
        mExecutor = Executors.newSingleThreadExecutor();
        try {
            BluetoothMapClient = Class.forName("android.bluetooth.BluetoothMapClient");
            mSetMessageStatus = BluetoothMapClient.getDeclaredMethod(
                    "setMessageStatus", BluetoothDevice.class, String.class, int.class);
        } catch (ClassNotFoundException e) {
            Log.e(TAG, "SMS functionality is not available on this Android platform.");
        } catch (NoSuchMethodException e) {
            Log.w(TAG, "SMS setMessageStatus not supported on this Android platform.");
        }
    }

    public boolean isBluetoothMapClientSupported() {
        return BluetoothMapClient != null;
    }

    public String getBluetoothMapClientConnectionState(Context context) {
        BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = manager.getAdapter();
        boolean connected =
                adapter.getProfileConnectionState(MessagingConstants.MAP_CLIENT) == BluetoothAdapter.STATE_CONNECTED;
        return connected ? Constants.ConnectionState.CONNECTED : Constants.ConnectionState.DISCONNECTED;
    }

    /**
     * Sends text through connected Bluetooth device
     */
    public boolean sendMessage(
            Context context, String deviceAddress, Uri[] destAddr, String text, PendingIntent sentIntent) {
        Log.d(TAG, "sendMessage request received: " + deviceAddress);

        BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = manager.getAdapter();

        MapMessageSenderListener senderListener =
                new MapMessageSenderListener(context, deviceAddress, destAddr, text, sentIntent);

        return adapter.getProfileProxy(context, senderListener, MessagingConstants.MAP_CLIENT);
    }

    public boolean getUnreadMessages(Context context, String deviceAddress) {
        Log.d(TAG, "getUnreadMessages request received: " + deviceAddress);

        BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = manager.getAdapter();
        MapMessageReaderListener readerListener = new MapMessageReaderListener(context, deviceAddress);

        return adapter.getProfileProxy(context, readerListener, MessagingConstants.MAP_CLIENT);
    }

    public boolean setMessageStatus(Context context, String deviceAddress, String handle, int status) {
        BluetoothManager manager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter adapter = manager.getAdapter();
        MapMessageStatusListener statusListener = new MapMessageStatusListener(context, deviceAddress, handle, status);

        return adapter.getProfileProxy(context, statusListener, MessagingConstants.MAP_CLIENT);
    }

    //-------------------------------------------------------------------------
    // Private helper classes and methods
    //-------------------------------------------------------------------------

    /**
     * Base Map Service Listner to initiate messaging API calls.
     */
    private class MapServiceListener implements BluetoothProfile.ServiceListener {
        private static final long WAIT_TIME_MILLIS = 1000;
        private static final int CONNECTED_DEVICES_RETRIES = 4;
        protected BluetoothAdapter mAdapter;
        protected BluetoothDevice mDevice;
        protected BluetoothProfile mMapProfile;
        protected String mDeviceAddress;
        protected Context mContext;

        MapServiceListener(Context context, String deviceAddress) {
            super();
            mContext = context;
            mMapProfile = null;
            mAdapter = null;
            mDevice = null;
            mDeviceAddress = deviceAddress;
        }

        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            Log.d(TAG, "MapServiceListener connected");

            BluetoothManager manager = (BluetoothManager) mContext.getSystemService(Context.BLUETOOTH_SERVICE);
            mAdapter = manager.getAdapter();

            try {
                mMapProfile = (BluetoothProfile) BluetoothMapClient.cast(proxy);
            } catch (ClassCastException e) {
                Log.e(TAG, "Error initializing Bluetooth MAP Client proxy");
                return;
            }

            Log.d(TAG, "Searching for device address: " + mDeviceAddress);
            for (int i = 0; i < CONNECTED_DEVICES_RETRIES; i++) {
                List<BluetoothDevice> connectedDevices = proxy.getConnectedDevices();
                if (connectedDevices.isEmpty()) {
                    try {
                        Log.v(TAG, "Retrying for connected devices: " + mDeviceAddress);
                        Thread.sleep(WAIT_TIME_MILLIS);
                    } catch (InterruptedException e) {
                        Log.e(TAG, "Interrupted", e);
                    }
                    continue;
                }
                for (BluetoothDevice device : connectedDevices) {
                    Log.d(TAG, "checking device: " + device.getAddress());
                    if (mDeviceAddress.equalsIgnoreCase(device.getAddress())) {
                        Log.d(TAG, "device selected: " + device.getAddress());
                        mDevice = device;
                    }
                }
                break;
            }
        }

        @Override
        public void onServiceDisconnected(int profile) {
            Log.i(TAG, "MapServiceListener disconnected");
        }
    }

    private class MapMessageSenderListener extends MapServiceListener {
        Uri[] mContacts;
        String mMessage;
        PendingIntent mSentIntent;

        MapMessageSenderListener(final Context context, final String deviceAddress, final Uri[] contacts,
                final String message, final PendingIntent sentIntent) {
            super(context, deviceAddress);
            mContacts = contacts;
            mMessage = message;
            mSentIntent = sentIntent;
        }

        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            mExecutor.submit(() -> {
                super.onServiceConnected(profile, proxy);

                Log.d(TAG, "MapMessageSender connected");

                if (mMapProfile == null || mDevice == null) {
                    Log.e(TAG,
                            String.format("BluetoothMapClient: is null: profile: %b device: %b", mMapProfile == null,
                                    mDevice == null));
                    sendErrorInPendingIntent(mSentIntent, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                    return;
                }

                if (mMessage != null) {
                    Log.d(TAG, "Sending message through bluetooth map client");
                    try {
                        Log.d(TAG, "Check device " + BluetoothAdapter.checkBluetoothAddress(mDevice.getAddress()));
                        Method sendMessage = BluetoothMapClient.getDeclaredMethod("sendMessage", BluetoothDevice.class,
                                Uri[].class, String.class, PendingIntent.class, PendingIntent.class);
                        boolean invoked = (boolean) sendMessage.invoke(
                                mMapProfile, mDevice, mContacts, mMessage, mSentIntent, null);
                        Log.i(TAG, "sendMessage invoked: " + invoked);
                        if (!invoked) {
                            sendErrorInPendingIntent(mSentIntent, SmsManager.RESULT_ERROR_NO_SERVICE);
                        }
                        mMessage = null;
                    } catch (NoSuchMethodException e) {
                        Log.d(TAG, "Send message is not supported by BluetoothMapClient");
                        sendErrorInPendingIntent(mSentIntent, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                    } catch (Exception e) {
                        Log.d(TAG, "Error invoking sendMessage method: " + e);
                        sendErrorInPendingIntent(mSentIntent, SmsManager.RESULT_ERROR_NO_SERVICE);
                    }
                } else {
                    Log.e(TAG, "Message is null");
                    sendErrorInPendingIntent(mSentIntent, SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                }

                mAdapter.closeProfileProxy(MessagingConstants.MAP_CLIENT, mMapProfile);
            });
        }

        @Override
        public void onServiceDisconnected(int profile) {
            super.onServiceDisconnected(profile);
            if (mMessage != null) {
                Log.d(TAG, "Bluetooth disconnected before sending the message");
                sendErrorInPendingIntent(mSentIntent, SmsManager.RESULT_ERROR_NO_SERVICE);
                mMessage = null;
            }
        }
    }

    private class MapMessageReaderListener extends MapServiceListener {
        MapMessageReaderListener(final Context context, final String deviceAddress) {
            super(context, deviceAddress);
        }

        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            mExecutor.submit(() -> {
                super.onServiceConnected(profile, proxy);
                Log.d(TAG, "MapMessageReader connected");

                if (mMapProfile == null || mDevice == null) {
                    Log.e(TAG,
                            String.format("BluetoothMapClient: is null: profile: %b device: %b", mMapProfile == null,
                                    mDevice == null));
                    return;
                }

                try {
                    Method getUnreadMessages =
                            BluetoothMapClient.getDeclaredMethod("getUnreadMessages", BluetoothDevice.class);

                    boolean invoked = (boolean) getUnreadMessages.invoke(mMapProfile, mDevice);
                    Log.i(TAG, "getUnreadMessages invoked: " + invoked);
                } catch (NoSuchMethodException e) {
                    Log.i(TAG, "Get unread messages is not supported by BluetoothMapClient");
                } catch (Exception e) {
                    Log.e(TAG, "onServiceConnected failed", e);
                }

                mAdapter.closeProfileProxy(MessagingConstants.MAP_CLIENT, mMapProfile);
            });
        }
    }

    private class MapMessageStatusListener extends MapServiceListener {
        String mHandle;
        int mStatus;

        MapMessageStatusListener(final Context context, String deviceAddress, String handle, int status) {
            super(context, deviceAddress);
            this.mHandle = handle;
            this.mStatus = status;
        }

        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            mExecutor.submit(() -> {
                super.onServiceConnected(profile, proxy);
                Log.d(TAG, "MapMessageStatus connected");

                if (mMapProfile == null || mDevice == null) {
                    Log.e(TAG,
                            String.format("BluetoothMapClient: is null: profile: %b device: %b", mMapProfile == null,
                                    mDevice == null));
                    return;
                }

                try {
                    Method setMessageStatus = BluetoothMapClient.getDeclaredMethod(
                            "setMessageStatus", BluetoothDevice.class, String.class, int.class);
                    boolean invoked = (boolean) setMessageStatus.invoke(mMapProfile, mDevice, mHandle, mStatus);
                    Log.i(TAG, "setMessageStatus invoked: " + invoked);
                } catch (NoSuchMethodException e) {
                    Log.i(TAG, "Setting message status is not supported by BluetoothMapClient");
                } catch (Exception e) {
                    Log.e(TAG, "onServiceConnected failed", e);
                }

                mAdapter.closeProfileProxy(MessagingConstants.MAP_CLIENT, mMapProfile);
            });
        }
    }

    public boolean supportsSetMessageStatus() {
        return mSetMessageStatus != null;
    }

    private void sendErrorInPendingIntent(PendingIntent intent, int errorCode) {
        if (intent == null) {
            return;
        }
        try {
            intent.send(errorCode);
        } catch (PendingIntent.CanceledException e) {
            Log.e(TAG, "PendingIntent.CanceledException: ", e);
        }
    }
}
