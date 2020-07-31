/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Messaging;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.amazon.aace.messaging.Messaging;
import com.amazon.sampleapp.FileUtils;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * The handler provides a sample implementation of the Messaging platform
 * interface.
 */
public class MessagingHandler extends Messaging {
    private static String sTag = "Messaging";

    // External
    private final Activity mActivity;
    private final String mConversationsDataPath;
    private final LoggerHandler mLogger;

    // UI Components
    private TextView mConfigurePermissionsButton;
    private TextView mConversationsReportButton;
    private TextView mDisplayInfoButton;
    private TextView mSendMessageSucceededButton;
    private TextView mSendMessageFailedButton;
    private TextView mUpdateMessagesStatusSucceededButton;
    private TextView mUpdateMessagesStatusFailedButton;
    private LinearLayout mControlsLayout;

    // Used for response callbacks
    private String mToken;
    private String mConversationId;
    private JSONObject mStatusJson;
    private ArrayList<String> mReadMessages;
    private ArrayList<String> mSentMessages;

    // Messaging endpoint state
    private ConnectionState mConnectionState = ConnectionState.DISCONNECTED;
    private PermissionState mSendPermission = PermissionState.OFF;
    private PermissionState mReadPermission = PermissionState.OFF;

    // Respond to messages or status updates automatically
    private boolean mAutomaticReponses = true;

    // Configurations for messaging
    private boolean mConfigurationValues[] = new boolean[3];
    private String mConfigurationNames[] = new String[3];

    // Conversations report object read from file
    JSONArray mConversations;

    // Executor
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    /**
     * Constructor for messaging handler.
     * @param activity The Android activity instance.
     * @param logger The logger instance.
     * @param conversationsDataPath The path to the sample conversations report.
     */
    public MessagingHandler(Activity activity, LoggerHandler logger, String conversationsDataPath) {
        mActivity = activity;
        mConversationsDataPath = conversationsDataPath;
        mLogger = logger;
        mToken = "";
        mConversationId = "";
        mStatusJson = null;
        mReadMessages = new ArrayList<>();
        mSentMessages = new ArrayList<>();

        // Set the names of the permission strings to be displayed
        mConfigurationNames[0] = mActivity.getString(R.string.messaging_send_permission);
        mConfigurationNames[1] = mActivity.getString(R.string.messaging_read_permission);
        mConfigurationNames[2] = mActivity.getString(R.string.messaging_automatic_responses);

        // Set up GUI callbacks
        setupGUI();
        // Update GUI
        updateGUI();
    }

    @Override
    public void sendMessage(String token, String message, String recipients) {
        mExecutor.submit(() -> {
            try {
                JSONArray recipientsJSON = new JSONArray(recipients);
                StringBuffer outputBuffer = new StringBuffer();

                outputBuffer.append("Send message received:")
                        .append("\n")
                        .append("Token: ")
                        .append(token)
                        .append("\n")
                        .append("Text: ")
                        .append(message)
                        .append("\n")
                        .append("Recipients: ")
                        .append(recipientsJSON.toString(2));

                mLogger.postInfo(sTag, outputBuffer.toString());
                toastMessage(outputBuffer.toString());

                // Construct a string version of the send message for display purposes later
                String messageInfo = "Message: " + message.substring(0, message.indexOf("\n")) + " Recipients: ";
                for (int i = 0; i < recipientsJSON.length(); i++) {
                    messageInfo += recipientsJSON.getJSONObject(i).getString("address");
                    if (i < recipientsJSON.length() - 1) {
                        messageInfo += ", ";
                    }
                }

                // Save in list of sent messages
                mSentMessages.add(messageInfo);

                // Send response
                if (mAutomaticReponses) {
                    if (mSendPermission == PermissionState.ON) {
                        mLogger.postInfo(sTag, "sendMessageSucceeded:token:" + token);
                        sendMessageSucceeded(token);
                    } else {
                        mLogger.postInfo(sTag, "sendMessageFailed:token:" + token);
                        sendMessageFailed(token, ErrorCode.NO_PERMISSION, "Send permission is not enabled");
                    }
                    resetGlobals();
                } else {
                    mToken = token;
                }
            } catch (JSONException e) {
                mLogger.postError(sTag, e.getMessage());
            }
        });
    }

    @Override
    public void uploadConversations(String token) {
        mToken = token;
        mExecutor.submit(() -> {
            String message = "Upload conversations request received";
            mLogger.postInfo(sTag, message);
            toastMessage(message);
            if (mConversations != null) {
                // Send conversation report
                if (mAutomaticReponses) {
                    conversationsReport(mToken, mConversations.toString());
                    mLogger.postInfo(sTag, "Conversation report sent for token: " + token);
                    resetGlobals();
                } else {
                    mToken = token;
                }
            } else {
                mLogger.postError(sTag, "Conversations report is not found");
            }
        });
    }

    @Override
    public void updateMessagesStatus(String token, String conversationId, String status) {
        mExecutor.submit(() -> {
            try {
                StringBuffer buffer = new StringBuffer();
                JSONObject statusJson = new JSONObject(status);
                buffer.append("Update messages status received\n")
                        .append("Token: ")
                        .append(token)
                        .append("\n")
                        .append("Conversation Id: ")
                        .append(conversationId)
                        .append("\n")
                        .append("Status: ")
                        .append(statusJson.toString(2))
                        .append("\n");
                mLogger.postInfo(sTag, buffer.toString());
                toastMessage(buffer.toString());

                // Update messages automatically
                if (mAutomaticReponses) {
                    // Update message status
                    updateMessages(conversationId, statusJson);
                    // Update engine
                    updateMessagesStatusSucceeded(token);
                    resetGlobals();
                } else {
                    // Save for manual update response
                    mToken = token;
                    mStatusJson = statusJson;
                    mConversationId = conversationId;
                }
            } catch (JSONException e) {
                mLogger.postError(sTag, e);
            }
        });
    }

    /**
     * Update the message status locally.
     * @param conversationId Conversation id to update.
     * @param statusJson New status for messages to be udpated.
     */
    private void updateMessages(String conversationId, JSONObject statusJson) {
        // Build map of read messages to delete
        try {
            HashSet<String> readSet = new HashSet<>();
            JSONArray readMessages = statusJson.getJSONArray("read");
            for (int i = 0; i < readMessages.length(); i++) {
                readSet.add(readMessages.getString(i));
            }

            // Conversations array
            for (int i = 0; i < mConversations.length(); i++) {
                JSONObject conversation = mConversations.getJSONObject(i);
                if (conversationId.equals(conversation.getString("id"))) {
                    JSONArray messages = conversation.getJSONArray("messages");
                    if (messages.length() > 0) {
                        for (int m = 0; m < messages.length(); m++) {
                            JSONObject message = messages.getJSONObject(m);
                            if (readSet.contains(message.getString("id"))) {
                                messages.remove(m);
                                mReadMessages.add("ID: " + message.getString("id")
                                        + " Text: " + message.getJSONObject("messagePayload").getString("text"));
                                int unreadMessageCount = conversation.getInt("unreadMessageCount") - 1;
                                conversation.put("unreadMessageCount", unreadMessageCount);
                            }
                        }
                    }
                    break;
                }
            }
        } catch (JSONException e) {
            mLogger.postError(sTag, e);
        }
    }

    /**
     *  Set GUI elements and callbacks.
     */
    private void setupGUI() {
        // Switch to toggle messaging endpoint connection
        View switchItem = mActivity.findViewById(R.id.toggleMessagingConnection);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.messaging_connection_switch);
        // Initialize the messaging drawer switch
        SwitchCompat drawerSwitch = switchItem.findViewById(R.id.drawerSwitch);
        drawerSwitch.setChecked(mConnectionState == ConnectionState.CONNECTED ? true : false);
        drawerSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                toggleMessagingConnectionState(isChecked);
            }
        });

        mControlsLayout = mActivity.findViewById(R.id.messagingConfig);
        mControlsLayout.setVisibility(View.GONE);

        mConfigurePermissionsButton = mActivity.findViewById(R.id.messagingConfiguration);
        mConversationsReportButton = mActivity.findViewById(R.id.conversationsReport);
        mDisplayInfoButton = mActivity.findViewById(R.id.displayInfo);
        mSendMessageSucceededButton = mActivity.findViewById(R.id.sendMessageSucceeded);
        mSendMessageFailedButton = mActivity.findViewById(R.id.sendMessageFailed);
        mUpdateMessagesStatusSucceededButton = mActivity.findViewById(R.id.updateMessagesStatusSucceeded);
        mUpdateMessagesStatusFailedButton = mActivity.findViewById(R.id.updateMessagesStatusFailed);

        mConfigurePermissionsButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onMessagingConfigurationUpdated();
            }
        });

        mConversationsReportButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mExecutor.submit(() -> { onConversationsReport(); });
            }
        });

        mDisplayInfoButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mExecutor.submit(() -> { onDisplayInfo(); });
            }
        });

        mSendMessageSucceededButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mExecutor.submit(() -> {
                    if (!mToken.isEmpty()) {
                        mLogger.postInfo(sTag, "Sent message succeeded response for token: " + mToken);
                        sendMessageSucceeded(mToken);
                        resetGlobals();
                    } else {
                        mLogger.postError(sTag, "Cannot send message succeeded response due to empty token");
                    }
                });
            }
        });

        mSendMessageFailedButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mExecutor.submit(() -> {
                    if (!mToken.isEmpty()) {
                        mLogger.postInfo(sTag, "Sent message failed response for token: " + mToken);
                        if (mSendPermission == PermissionState.OFF) {
                            sendMessageFailed(mToken, ErrorCode.NO_PERMISSION, "Send permission is not enabled");
                        } else {
                            sendMessageFailed(mToken, ErrorCode.GENERIC_FAILURE, "Unable to send message");
                        }
                        resetGlobals();
                    } else {
                        mLogger.postError(sTag, "Cannot send message failed response due to empty token");
                    }
                });
            }
        });

        mUpdateMessagesStatusSucceededButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mExecutor.submit(() -> {
                    if (!mToken.isEmpty()) {
                        if (!mConversationId.isEmpty() && mStatusJson != null) {
                            // Update messages
                            updateMessages(mConversationId, mStatusJson);
                            // Notify engine
                            updateMessagesStatusSucceeded(mToken);
                            resetGlobals();
                            mLogger.postInfo(
                                    sTag, "Sent update messages status succeeded response for token: " + mToken);
                        } else {
                            mLogger.postError(sTag,
                                    "Cannot send message failed response due to empty conversation id and status");
                        }
                    } else {
                        mLogger.postError(
                                sTag, "Cannot send update message status succeeded response due to empty token");
                    }
                });
            }
        });

        mUpdateMessagesStatusFailedButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mExecutor.submit(() -> {
                    if (!mToken.isEmpty()) {
                        mLogger.postInfo(sTag, "Sent update messages status failed response for token: " + mToken);
                        updateMessagesStatusFailed(
                                mToken, ErrorCode.GENERIC_FAILURE, "Unable to update messages status");
                        resetGlobals();
                    } else {
                        mLogger.postError(sTag, "Cannot send message failed due to empty token");
                    }
                });
            }
        });
    }

    /**
     * Reset global variables used in manual responses.
     */
    private void resetGlobals() {
        mToken = "";
        mConversationId = "";
        mStatusJson = null;
    }
    /**
     * Update GUI elements.
     */
    private void updateGUI() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                // Hide or show buttons depending on setting
                if (mAutomaticReponses) {
                    mSendMessageSucceededButton.setVisibility(View.GONE);
                    mSendMessageFailedButton.setVisibility(View.GONE);
                    mUpdateMessagesStatusSucceededButton.setVisibility(View.GONE);
                    mUpdateMessagesStatusFailedButton.setVisibility(View.GONE);
                } else {
                    mSendMessageSucceededButton.setVisibility(View.VISIBLE);
                    mSendMessageFailedButton.setVisibility(View.VISIBLE);
                    mUpdateMessagesStatusSucceededButton.setVisibility(View.VISIBLE);
                    mUpdateMessagesStatusFailedButton.setVisibility(View.VISIBLE);
                }
            }
        });
    }

    /**
     * Toggles the messaging connection state.
     *
     * @param enable The toggle state.
     */
    private void toggleMessagingConnectionState(boolean enable) {
        mConnectionState = enable ? ConnectionState.CONNECTED : ConnectionState.DISCONNECTED;
        if (enable) {
            mControlsLayout.setVisibility(View.VISIBLE);
            mLogger.postInfo(sTag, "ConnectionState: CONNECTED");
            updateMessagingEndpointState(mConnectionState, mSendPermission, mReadPermission);
        } else {
            mControlsLayout.setVisibility(View.GONE);
            mLogger.postInfo(sTag, "ConnectionState: DISCONNECTED");
            updateMessagingEndpointState(mConnectionState, mSendPermission, mReadPermission);
        }
    }

    /**
     * Upload all text messages from a sample file. The file must be
     * a JSON array with the specified format from the platform interface.
     */
    private void onConversationsReport() {
        try {
            // Token may be empty or contain the value from an uploadConversations request
            mConversations = FileUtils.parseFileAsJSONArray(mConversationsDataPath);
            StringBuffer buffer = new StringBuffer();
            boolean valid = validateConversationsReport(buffer);
            if (valid) {
                toastMessage("Conversations report uploaded");
                conversationsReport(mToken, mConversations.toString());
                resetGlobals();
                mReadMessages.clear();
            } else {
                toastMessage("Conversations report was not uploaded due to missing required fields");
                mLogger.postError(sTag, buffer.toString());
            }
        } catch (Exception e) {
            String errorMessage = "Could not load conversations report. Error: " + e.getMessage();
            toastMessage(errorMessage);
            mLogger.postError(sTag, errorMessage);
        }
    }

    /**
     * Display the current conversation report in the log.
     */
    private void onDisplayInfo() {
        StringBuffer buffer = new StringBuffer();

        buffer.append("Messaging information\n\n")
                .append("Connection state: ")
                .append(mConnectionState.toString())
                .append("\n")
                .append("Send permission: ")
                .append(mSendPermission.toString())
                .append("\n")
                .append("Read permission: ")
                .append(mReadPermission.toString())
                .append("\n")
                .append("Automatic responses: ")
                .append(mAutomaticReponses)
                .append("\n");

        if (!mAutomaticReponses) {
            buffer.append("Token: ").append(mToken).append("\n");
        }

        boolean valid = validateConversationsReport(buffer);

        if (!mReadMessages.isEmpty()) {
            buffer.append("\nRead messages\n\n");
            for (Iterator it = mReadMessages.iterator(); it.hasNext();) {
                buffer.append("  " + it.next() + "\n");
            }
        }

        if (!mSentMessages.isEmpty()) {
            buffer.append("\nSent messages\n\n");
            for (Iterator it = mSentMessages.iterator(); it.hasNext();) {
                buffer.append("  " + it.next() + "\n");
            }
        }

        // Log display information
        if (valid) {
            mLogger.postInfo(sTag, buffer.toString());
        } else {
            buffer.append("\nERROR: Conversations report is invalid due to missing required fields");
            mLogger.postError(sTag, buffer.toString());
        }
    }

    private boolean validateConversationsReport(StringBuffer buffer) {
        boolean isValid = true;
        if (mConversations != null) {
            buffer.append("\nConversations Report\n");
            for (int i = 0; i < mConversations.length(); i++) {
                try {
                    // Get conversation
                    JSONObject conversation = mConversations.getJSONObject(i);
                    if (conversation.has("id")) {
                        buffer.append("\n  Conversation id: ").append(conversation.getString("id")).append("\n\n");
                    } else {
                        buffer.append("\n  Conversation id: Not found\n");
                        isValid = false;
                    }

                    // Print other participants
                    if (conversation.has("otherParticipants")) {
                        JSONArray otherParticipants = conversation.getJSONArray("otherParticipants");
                        if (otherParticipants.length() > 0) {
                            buffer.append("    Participants\n\n");
                            // Print participants
                            for (int p = 0; p < otherParticipants.length(); p++) {
                                JSONObject participant = otherParticipants.getJSONObject(p);
                                if (participant.has("address")) {
                                    buffer.append("      ").append(participant.getString("address")).append("\n");
                                } else {
                                    buffer.append("      Phone number is missing").append("\n");
                                    isValid = false;
                                }

                                if (!participant.has("addressType")) {
                                    buffer.append("      addressType: Not found").append("\n");
                                    isValid = false;
                                } else {
                                    if (participant.getString("addressType").compareTo("PhoneNumberAddress") != 0) {
                                        buffer.append("      addressType: Expected 'PhoneNumberAddress' Found '")
                                                .append(participant.getString("addressType"))
                                                .append("'")
                                                .append("\n");
                                        isValid = false;
                                    }
                                }
                            }
                            buffer.append("\n");
                        }
                    } else {
                        buffer.append("    Participants: Not found\n\n");
                        isValid = false;
                    }

                    // Print messages
                    if (conversation.has("messages")) {
                        JSONArray messages = conversation.getJSONArray("messages");
                        if (messages.length() > 0) {
                            buffer.append("    Messages\n");
                            // Print messages
                            for (int m = 0; m < messages.length(); m++) {
                                JSONObject message = messages.getJSONObject(m);

                                buffer.append("\n      Message id: ");
                                if (message.has("id")) {
                                    buffer.append(message.getString("id"));
                                } else {
                                    buffer.append("Not found");
                                    isValid = false;
                                }

                                buffer.append("\n      Text: ");
                                if (message.has("messagePayload")) {
                                    JSONObject payload = message.getJSONObject("messagePayload");
                                    if (payload.has("text")) {
                                        buffer.append(payload.getString("text"));
                                    } else {
                                        buffer.append("Not found");
                                        isValid = false;
                                    }

                                    if (!payload.has("@type")) {
                                        buffer.append("\n      @type: Not found");
                                        isValid = false;
                                    } else {
                                        if (payload.getString("@type").compareTo("text") != 0) {
                                            buffer.append("\n      @type: Expected 'text' Found '")
                                                    .append(payload.getString("@type"))
                                                    .append("'");
                                            isValid = false;
                                        }
                                    }
                                } else {
                                    buffer.append("Not found");
                                    isValid = false;
                                }

                                buffer.append("\n      Status: ");
                                if (message.has("status")) {
                                    buffer.append(message.getString("status"));
                                } else {
                                    buffer.append("Not found");
                                    isValid = false;
                                }

                                buffer.append("\n      Time: ");
                                if (message.has("createdTime")) {
                                    buffer.append(message.getString("createdTime"));
                                } else {
                                    buffer.append("Not found");
                                    isValid = false;
                                }

                                buffer.append("\n      Sender: ");
                                if (message.has("sender")) {
                                    JSONObject sender = message.getJSONObject("sender");
                                    if (sender.has("address")) {
                                        buffer.append(sender.getString("address"));
                                    } else {
                                        buffer.append("Not found");
                                        isValid = false;
                                    }
                                    if (!sender.has("addressType")) {
                                        buffer.append("\n      addressType: Not found");
                                        isValid = false;
                                    } else {
                                        if (sender.getString("addressType").compareTo("PhoneNumberAddress") != 0) {
                                            buffer.append("\n      addressType: Expected 'PhoneNumberAddress' found '")
                                                    .append(sender.getString("addressType"))
                                                    .append("''");
                                            isValid = false;
                                        }
                                    }
                                } else {
                                    buffer.append("Not found");
                                    isValid = false;
                                }

                                buffer.append("\n");
                                // Space out messages
                                if (m < messages.length() - 1) {
                                    buffer.append("\n");
                                }
                            }
                        }
                    } else {
                        buffer.append("    Messages: Not found\n\n");
                        isValid = false;
                    }
                } catch (Exception e) {
                    mLogger.postError(sTag, e);
                }
            }
        }

        return isValid;
    }

    /**
     * Get send and read permissions from the user.
     */
    private void onMessagingConfigurationUpdated() {
        AlertDialog.Builder builder = new AlertDialog.Builder(mActivity);
        builder.setTitle(mActivity.getString(R.string.messaging_configure_title));

        // Use current committed values
        mConfigurationValues[0] = mSendPermission == PermissionState.ON ? true : false;
        mConfigurationValues[1] = mReadPermission == PermissionState.ON ? true : false;
        mConfigurationValues[2] = mAutomaticReponses;

        builder.setMultiChoiceItems(
                mConfigurationNames, mConfigurationValues, new DialogInterface.OnMultiChoiceClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int index, boolean value) {
                        mConfigurationValues[index] = value;
                    }
                });

        builder.setPositiveButton("Update", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                // Commit selected values
                mSendPermission = mConfigurationValues[0] ? PermissionState.ON : PermissionState.OFF;
                mReadPermission = mConfigurationValues[1] ? PermissionState.ON : PermissionState.OFF;
                mAutomaticReponses = mConfigurationValues[2];
                // Call platform interface to update permission and connection values
                updateMessagingEndpointState(mConnectionState, mSendPermission, mReadPermission);
                // Update GUI
                updateGUI();
                mLogger.postInfo(sTag, "Messaging configuration updated");
            }
        });

        builder.setNegativeButton("Cancel", null);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private void toastMessage(String message) {
        mActivity.runOnUiThread(() -> { Toast.makeText(mActivity, message, Toast.LENGTH_LONG).show(); });
    }
}
