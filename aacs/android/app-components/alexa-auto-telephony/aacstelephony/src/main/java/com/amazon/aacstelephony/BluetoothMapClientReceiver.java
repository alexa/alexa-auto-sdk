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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.telephony.SmsManager;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import com.amazon.aacsconstants.AACSConstants;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * The Android bluetooth MAP profile publishes intents when the
 * BluetoothMapClient sendMessage and getUnreadMessages APIs are invoked.
 * This receiver will handle those framework intents. It also handles connection state
 * updates for the MAP profile.
 */
public class BluetoothMapClientReceiver extends BroadcastReceiver {
    private static final String TAG = AACSConstants.AACS + "-" + BluetoothMapClientReceiver.class.getSimpleName();

    private static final String EXTRA_MESSAGE_HANDLE = "android.bluetooth.mapmce.profile.extra.MESSAGE_HANDLE";
    private static final String EXTRA_MESSAGE_TIMESTAMP = "android.bluetooth.mapmce.profile.extra.MESSAGE_TIMESTAMP";
    private static final String EXTRA_MESSAGE_READ_STATUS =
            "android.bluetooth.mapmce.profile.extra.MESSAGE_READ_STATUS";
    private static final String EXTRA_SENDER_CONTACT_URI = "android.bluetooth.mapmce.profile.extra.SENDER_CONTACT_URI";
    private static final String EXTRA_SENDER_CONTACT_NAME =
            "android.bluetooth.mapmce.profile.extra.SENDER_CONTACT_NAME";

    private static ExecutorService mExecutor;
    private MessagingHandler mMessagingHandler;

    public BluetoothMapClientReceiver(@NonNull Context context) {
        mExecutor = Executors.newSingleThreadExecutor();
        mMessagingHandler = MessagingHandler.getInstance(context);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(TAG, "action: " + action + " intent: " + intent + " result code: " + getResultCode());

        mExecutor.submit(() -> {
            if (MessagingConstants.ACTION_MESSAGE_RECEIVED.equals(action)) {
                String handle = intent.getStringExtra(EXTRA_MESSAGE_HANDLE);
                String senderName = intent.getStringExtra(EXTRA_SENDER_CONTACT_NAME);
                String senderUri = intent.getStringExtra(EXTRA_SENDER_CONTACT_URI);

                Log.v(TAG, "senderUri: " + senderUri);
                if (senderUri != null && senderUri.toLowerCase().startsWith("tel:")) {
                    senderUri = senderUri.replaceAll("[^0-9\\+]", "");
                }

                String text = intent.getStringExtra(android.content.Intent.EXTRA_TEXT);

                TimeZone tz = TimeZone.getTimeZone("UTC");
                SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'", Locale.getDefault());
                sdf.setTimeZone(tz);
                String receivedTime =
                        sdf.format(new Date(intent.getLongExtra(EXTRA_MESSAGE_TIMESTAMP, System.currentTimeMillis())));

                String[] recipients = intent.getStringArrayExtra(Intent.EXTRA_CC);
                StringBuffer outputBuffer = new StringBuffer();
                outputBuffer.append("Message received:")
                        .append("\n")
                        .append("handle: ")
                        .append(handle)
                        .append("\n")
                        .append("receivedTime: ")
                        .append(receivedTime)
                        .append("\n")
                        .append("Text: ")
                        .append(text)
                        .append("\n")
                        .append("senderName: ")
                        .append(senderName)
                        .append("\n")
                        .append("senderUri: ")
                        .append(senderUri)
                        .append("\n")
                        .append("recipients: ")
                        .append(recipients);

                Log.v(TAG, outputBuffer.toString());

                mMessagingHandler.updateMessageInConversationsReport(handle, text, receivedTime, senderUri, recipients);
            } else if (MessagingConstants.ACTION_MESSAGE_SENT.equals(action)) {
                Log.d(TAG, "Handling com.amazon.aacstelephony.MESSAGE_SENT");

                String token = intent.getStringExtra("token");
                token = token == null ? "" : token;

                int resultCode = getResultCode();
                if (resultCode <= 0) {
                    mMessagingHandler.sendMessageSucceeded(token);
                } else {
                    String messagingErrorCode = getMessagingErrorCode(resultCode);
                    mMessagingHandler.sendMessageFailed(token, messagingErrorCode, "BluetoothMapClient error");
                }
            } else if (MessagingConstants.CONNECTION_STATE_CHANGED.equals(action)) {
                boolean isConnected = BluetoothAdapter.STATE_CONNECTED
                        == intent.getIntExtra(BluetoothProfile.EXTRA_STATE, BluetoothAdapter.STATE_DISCONNECTED);

                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

                Log.i(TAG,
                        String.format("Bluetooth MAP connection changed deviceAddress: %s connected: %b",
                                device.getAddress(), isConnected));
                mMessagingHandler.connectionStateChanged(device, isConnected);
            }
        });
    }

    private String getMessagingErrorCode(int code) {
        switch (code) {
            case SmsManager.RESULT_ERROR_NO_SERVICE:
                return MessagingConstants.SEND_MESSAGE_FAIL_ERROR_CONNECTION;
            default:
                return MessagingConstants.SEND_MESSAGE_FAIL_ERROR_GENERIC;
        }
    }
}
