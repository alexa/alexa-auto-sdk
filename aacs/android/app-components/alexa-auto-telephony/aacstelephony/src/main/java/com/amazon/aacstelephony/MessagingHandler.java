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

import static com.amazon.aacstelephony.MessagingConstants.SEND_MESSAGE_FAIL_ERROR_GENERIC;
import static com.amazon.aacstelephony.MessagingConstants.SEND_MESSAGE_FAIL_ERROR_PERMISSION;

import android.Manifest;
import android.app.PendingIntent;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.telecom.PhoneAccount;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

public class MessagingHandler implements ConversationsReport.ConversationsReportUploadListener {
    private static final String TAG = AACSConstants.AACS + "-" + MessagingHandler.class.getSimpleName();
    private static final int READ = 1;

    private Context mContext;
    private AACSMessageSender mAACSMessageSender;
    private PendingIntent mSentIntent;
    private Intent mSendIntent;
    private String mDeviceAddress;
    private String mDeviceName;
    private boolean mConsent;
    private boolean hasConsentBeenGrantedPreviously;
    private ConversationsReport mConversationsReport;

    private BluetoothMapClientManager mBluetoothMapClientManager;
    private static MessagingHandler mInstance;

    private final SharedPreferences.OnSharedPreferenceChangeListener
            mMessagingPreferencesChangeListener = new SharedPreferences.OnSharedPreferenceChangeListener() {
        @Override
        public void onSharedPreferenceChanged(SharedPreferences messagingPreferences, String key) {
            Log.v(TAG, "Messaging SharedPreferences changed for key: " + key);
            if (mBluetoothMapClientManager.isBluetoothMapClientSupported()) {
                if (MessagingConstants.CONSENT_PROPERTY.equals(key)) {
                    String state = messagingPreferences.getString(MessagingConstants.STATE_PROPERTY, "");
                    if (state.isEmpty()) {
                        state = mBluetoothMapClientManager.getBluetoothMapClientConnectionState(mContext);
                        Log.v(TAG, "state: " + state);
                    }

                    mConsent = messagingPreferences.getBoolean(MessagingConstants.CONSENT_PROPERTY, false);
                    mDeviceAddress = messagingPreferences.getString(MessagingConstants.DEVICE_ADDRESS_PROPERTY, "");
                    mDeviceName = Util.getBluetoothDeviceName(mContext, mDeviceAddress);

                    Util.publishMessagingEndpointStateToAACS(mContext, mAACSMessageSender, state, mConsent);

                    Log.v(TAG,
                            String.format("key: %s value: %s deviceAddress: %s deviceName: %s", key, mConsent,
                                    mDeviceAddress, mDeviceName));
                    if (mConsent) {
                        uploadConversationsReport();
                        getUnreadMessages();
                        hasConsentBeenGrantedPreviously = true;
                    } else {
                        clearConversationsReport();
                    }
                } else if (MessagingConstants.DEVICE_ADDRESS_PROPERTY.equals(key)) {
                    String newDeviceAddress =
                            messagingPreferences.getString(MessagingConstants.DEVICE_ADDRESS_PROPERTY, "");
                    Log.v(TAG, String.format("key: %s value: %s old: %s", key, newDeviceAddress, mDeviceAddress));
                    // Primary bluetooth device changed
                    if (!newDeviceAddress.equals(mDeviceAddress)) {
                        clearConversationsReport();
                    }
                    getUnreadMessages();
                } else if (MessagingConstants.STATE_PROPERTY.equals(key)) {
                    String state = messagingPreferences.getString(
                            MessagingConstants.STATE_PROPERTY, Constants.ConnectionState.DISCONNECTED);
                    Log.v(TAG, String.format("key: %s value: %s deviceAddress: %s", key, state, mDeviceAddress));
                    if (Constants.ConnectionState.CONNECTED.equals(state)) {
                        getUnreadMessages();
                    } else {
                        clearConversationsReport();
                    }
                }
            } else {
                // In the event that the BluetoothMapClient API is not available on the
                // platform, then we disable the Messaging context to avoid unnecessary
                // Alexa conversations that will end up failing.
                Log.w(TAG, "Messaging SharedPreferences change ignored since SMS is not supported on this platform");
                Util.publishMessagingEndpointStateToAACS(
                        mContext, mAACSMessageSender, Constants.ConnectionState.DISCONNECTED, false);
            }
        }
    };

    /**
     * Clear the conversations report and publish to cloud. This should happen
     * when SMS consent is lost or the mobile device is disconnected.
     */
    private void clearConversationsReport() {
        mConversationsReport.clear();
        // Clear conversation report from cloud only consent was granted as least once
        if (hasConsentBeenGrantedPreviously) {
            // Clear conversation report when consent is lost
            Util.toast(mContext, mContext.getString(R.string.aacs_telephony_toast_sms_removing));
            sendConversationsReport("");
            Util.toast(mContext, mContext.getString(R.string.aacs_telephony_toast_sms_removed));
        }
    }

    /**
     * Singleton instance.
     * @param context Application context
     * @return MessagingHandler singleton instance.
     */
    public synchronized static MessagingHandler getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new MessagingHandler(context);
        }

        return mInstance;
    }

    /**
     * Private construction for the MessagingHandler.
     * @param context Application context.
     */
    private MessagingHandler(Context context) {
        mContext = context;
        mConversationsReport = new ConversationsReport();
        mConversationsReport.addListener(this);
        mAACSMessageSender = new AACSMessageSender(new WeakReference<>(context), new AACSSender());
        mBluetoothMapClientManager = new BluetoothMapClientManager();
        mDeviceAddress = "";
        mDeviceName = "";
        SharedPreferences messagingPreferences =
                context.getSharedPreferences(MessagingConstants.PREFERENCES_FILE_NAME, context.MODE_PRIVATE);
        messagingPreferences.registerOnSharedPreferenceChangeListener(mMessagingPreferencesChangeListener);
        mConsent = messagingPreferences.getBoolean("consent", false);
        hasConsentBeenGrantedPreviously = mConsent;
        Log.v(TAG, "MessagingHandler created. Current consent: " + mConsent);
    }

    /**
     * This method parses the Messaging.SendMessage payload, extracts token, message,
     * and recipients. It calls sendMessage API in BluetoothMapClient to send SMS message
     * through connected phone through Bluetooth.
     *
     * @param payload JSON payload containing SMS information.
     */

    public void sendMessage(String payload) {
        Log.v(TAG, "Sending message: " + payload);
        String token = "";

        try {
            JSONObject payloadJSON = new JSONObject(payload);
            String message = payloadJSON.getString("message");
            JSONArray recipients = new JSONArray(payloadJSON.getString("recipients"));
            token = payloadJSON.getString("token");

            mSendIntent = new Intent(MessagingConstants.ACTION_MESSAGE_SENT);
            mSendIntent.putExtra("token", token);

            if (mContext.checkSelfPermission(Manifest.permission.SEND_SMS) == PackageManager.PERMISSION_GRANTED) {
                mSentIntent = PendingIntent.getBroadcast(mContext, 0, mSendIntent, PendingIntent.FLAG_ONE_SHOT);
                Uri[] recipientsPhoneNumber = new Uri[recipients.length()];
                for (int i = 0; i < recipients.length(); i++) {
                    recipientsPhoneNumber[i] = new Uri.Builder()
                                                       .appendPath(recipients.getJSONObject(i).getString("address"))
                                                       .scheme(PhoneAccount.SCHEME_TEL)
                                                       .build();
                    Log.d(TAG, "Recipient phone number: " + recipientsPhoneNumber[i]);
                }

                boolean result = mBluetoothMapClientManager.sendMessage(
                        mContext, mDeviceAddress, recipientsPhoneNumber, message, mSentIntent);
                if (!result) {
                    sendMessageFailed(token, MessagingConstants.SEND_MESSAGE_FAIL_ERROR_GENERIC,
                            "Unable to invoke sendMessage API");
                }
            } else {
                Log.e(TAG, "SEND_SMS permission is not granted. Unable to send message.");
                sendMessageFailed(token, SEND_MESSAGE_FAIL_ERROR_PERMISSION, "SEND_SMS permission is not granted");
            }
        } catch (JSONException e) {
            String errorMessage = "Error parsing payload to sendMessage: " + e;
            sendMessageFailed(token, SEND_MESSAGE_FAIL_ERROR_GENERIC, errorMessage);
            Log.e(TAG, errorMessage);
        }
    }

    /**
     * This method parses Messaging.UpdateMessagesStatus payload, extracts token,
     * conversationId, and status. It updates message information in the locally
     * managed conversation report. If device supports it, it calls setMessageStatus
     * API in BluetoothMapClient to update message status on the connected phone.
     *
     * @param messageStatusPayload JSON payload containing SMS messages to be updated.
     */
    public void updateMessagesStatus(String messageStatusPayload) {
        try {
            JSONObject messageStatus = new JSONObject(messageStatusPayload);
            String conversationId = messageStatus.getString("conversationId");
            String token = messageStatus.getString("token");
            JSONObject status = new JSONObject(messageStatus.getString("status"));
            JSONArray readMessageIds = status.getJSONArray("read");
            List<String> readMessageIdsList = new ArrayList<>();

            for (int i = 0; i < readMessageIds.length(); i++) {
                String currId = readMessageIds.getString(i);
                readMessageIdsList.add(currId);
                if (mBluetoothMapClientManager.supportsSetMessageStatus()) {
                    boolean result =
                            mBluetoothMapClientManager.setMessageStatus(mContext, mDeviceAddress, currId, READ);
                    if (!result) {
                        Log.e(TAG, "Failed to request setMessageStatus for: " + currId);
                    }
                }
            }

            mConversationsReport.updateConversation(conversationId, readMessageIdsList);
            sendUpdateMessagesSucceeded(token);

        } catch (JSONException e) {
            Log.e(TAG, "Error parsing payload: ", e);
        }
    }

    /**
     * Upload conversation report driven by device, which means no token is present.
     */
    public void uploadConversationsReport() {
        uploadConversationsReport("");
    }

    /**
     * This method parses Messaging.UploadConversation payload, extracts token
     * and sends the latest conversation report to the cloud.
     *
     * @param payload The JSON payload containing token or empty.
     */
    public void uploadConversationsReport(String payload) {
        Log.d(TAG, "Upload conversations report: " + payload);
        try {
            String token = "";
            if (payload != null && !payload.isEmpty()) {
                JSONObject tokenPayload = new JSONObject(payload);
                token = tokenPayload.getString("token");
            }

            // Upload current conversations report
            sendConversationsReport(token);
        } catch (JSONException e) {
            Log.e(TAG, "Error retrieving token for upload conversation", e);
        }
    }

    /**
     * Call platform API to discover unread messages from the connected device.
     */
    private void getUnreadMessages() {
        Log.v(TAG,
                String.format("Start getUnreadMessages: deviceAddress: %s name: %s consent: %s", mDeviceAddress,
                        mDeviceName, mConsent));
        if (mContext.checkSelfPermission(Manifest.permission.READ_SMS) == PackageManager.PERMISSION_GRANTED) {
            if (mDeviceAddress != null && !mDeviceAddress.isEmpty() && mConsent) {
                boolean result = mBluetoothMapClientManager.getUnreadMessages(mContext, mDeviceAddress);
                if (!result) {
                    Log.e(TAG, "Failed to request unread messages");
                } else {
                    Util.toast(mContext,
                            String.format(mContext.getString(R.string.aacs_telephony_toast_sms_syncing), mDeviceName));
                }
            }
        } else {
            Log.e(TAG, "READ_SMS permission is not granted. Unable to request unread messages.");
        }
    }

    /**
     * This method sends AACS message containing message token and conversation
     * content to Engine to
     * notify Engine to upload conversation report to cloud.
     *
     * @param token Token for conversation report or empty
     */
    public void sendConversationsReport(String token) {
        try {
            String conversationReportJson = mConversationsReport.toJson().toString();
            JSONObject payload = new JSONObject();
            payload.put("token", token);
            payload.put("conversations", conversationReportJson);
            mAACSMessageSender.sendMessage(Topic.MESSAGING, Action.Messaging.CONVERSATIONS_REPORT, payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, "failed to send conversation report" + e);
        }
    }

    /**
     * This method sends AACS message containing message token to Engine to notify
     * Engine that message
     * sending was successful.
     *
     * @param token
     */
    public void sendMessageSucceeded(String token) {
        try {
            JSONObject payload = new JSONObject();
            payload.put("token", token);
            mAACSMessageSender.sendMessage(
                    Topic.MESSAGING, Action.Messaging.SEND_MESSAGE_SUCCEEDED, payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Failed to construct sendMessageSucceeded message to AACS: " + e);
        }
    }

    /**
     * This method sends AACS message containing message token, error code and error
     * message to Engine
     * to notify Engine that message sending was failed.
     *
     * @param token
     * @param code
     * @param message
     */
    public void sendMessageFailed(String token, String code, String message) {
        try {
            JSONObject payload = new JSONObject();
            payload.put("token", token);
            payload.put("code", code);
            payload.put("message", message);
            mAACSMessageSender.sendMessage(Topic.MESSAGING, Action.Messaging.SEND_MESSAGE_FAILED, payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Failed to construct sendMessageFailed message to AACS: " + e);
        }
    }

    /**
     * Notify that messages were updated successfully locally.
     *
     * @param token
     */
    public void sendUpdateMessagesSucceeded(String token) {
        try {
            JSONObject payload = new JSONObject();
            payload.put("token", token);
            mAACSMessageSender.sendMessage(
                    Topic.MESSAGING, Action.Messaging.UPDATE_MESSAGES_STATUS_SUCCEEDED, payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Failed to construct UpdateMessagesStatusSucceeded message to AACS: " + e);
        }
    }

    public void updateMessageInConversationsReport(
            String id, String text, String time, String phone, String[] recipients) {
        mConversationsReport.updateMessage(id, text, time, phone, recipients);
    }

    public void connectionStateChanged(BluetoothDevice device, boolean connected) {
        Log.v(TAG,
                String.format("connectionStateChanged deviceAddress: %s primaryAddress: %s name: %s isConnected: %b",
                        device.getAddress(), mDeviceAddress, mDeviceName, connected));
        if (device.getAddress().equals(mDeviceAddress)) {
            mDeviceName = device.getName();
            if (connected) {
                Util.publishMessagingEndpointStateToAACS(
                        Constants.ConnectionState.CONNECTED, mContext, mAACSMessageSender);
            } else {
                Util.publishMessagingEndpointStateToAACS(
                        Constants.ConnectionState.DISCONNECTED, mContext, mAACSMessageSender);
            }
        }
    }

    @Override
    public void readyForUpload() {
        Log.v(TAG, "readyForUpload: consent: " + mConsent);
        if (mConsent) {
            sendConversationsReport("");
            Util.toast(
                    mContext, String.format(mContext.getString(R.string.aacs_telephony_toast_sms_synced), mDeviceName));
        }
    }
}