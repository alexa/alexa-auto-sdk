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

import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class ConversationsReport {
    private static final String TAG = AACSConstants.AACS + "-" + ConversationsReport.class.getSimpleName();
    private static final long NOTIFICATION_WAIT_TIME_MILLIS = 200;
    private static final int MAX_MESSAGES = 10;
    private long mLastNotification;

    private final Object mNotificationLock = new Object();
    private boolean mNotificationActive;
    private int mNotificationRequestCount;

    private ExecutorService mExecutor;

    // Listener for when conversation report can be uploaded
    private ConversationsReportUploadListener mListener;

    // List of all current conversations
    Hashtable<String, Conversation> mConversations;

    // Only Android 12+ supports marking messages as "READ" on the connected device. So we have
    // to keep track of messages that have already been read out by Alexa since they will be
    // returned the next time messages are pulled from the connected device.
    final static HashSet<String> mKnownMessageIds = new HashSet<>();

    public ConversationsReport() {
        mConversations = new Hashtable<>();
        mExecutor = Executors.newSingleThreadExecutor();
        mNotificationActive = false;
        mNotificationRequestCount = 0;
    }

    public void addListener(ConversationsReportUploadListener listener) {
        mListener = listener;
    }

    public synchronized void clear() {
        mConversations.clear();
    }

    public synchronized void updateMessage(String id, String text, String time, String phone, String[] recipients) {
        if (!hasAlreadyBeenRead(id)) {
            if (phone == null || phone.isEmpty()) {
                Log.i(TAG, String.format("Ignoring non SMS message id %s phone %s", id));
                return;
            }

            List<String> participants = new ArrayList<>();
            participants.add(phone);
            if (recipients != null) {
                participants.addAll(Arrays.asList(recipients));
            }

            Conversation conversation = getConversationByParticipants(participants);

            if (conversation == null) {
                conversation = new Conversation();
            }

            Log.d(TAG, String.format("Adding id %s phone %s", id, phone));
            conversation.addMessage(id, text, time, phone, participants);
            addConversation(conversation);
            notifyUpload();
        } else {
            Log.v(TAG, "Ignoring duplicate SMS id: " + id);
        }
    }

    public boolean hasAlreadyBeenRead(String id) {
        return mKnownMessageIds.contains(id);
    }

    public synchronized void updateConversation(String conversationId, List<String> readMessageIds) {
        if (mConversations.containsKey(conversationId)) {
            Conversation conversation = mConversations.get(conversationId);

            for (String messageId : readMessageIds) {
                conversation.removeMessage(messageId);
                // Save message id for message that Alexa has read out
                mKnownMessageIds.add(messageId);
                Log.v(TAG, String.format("Removing message id %s from conversation %s", messageId, conversationId));
            }
            if (conversation.size() == 0) {
                Log.v(TAG, "Removing empty conversation: " + conversationId);
                mConversations.remove(conversationId);
            }
        }
    }

    public synchronized Conversation getConversationByParticipants(List<String> participants) {
        for (Conversation conversation : mConversations.values()) {
            if (conversation.containsParticipants(participants)) {
                return conversation;
            }
        }

        return null;
    }

    public synchronized JSONArray toJson() throws JSONException {
        JSONArray output = new JSONArray();
        for (Conversation conversation : mConversations.values()) {
            output.put(conversation.toJson());
        }
        return output;
    }

    /**
     * Notifies that a conversation report can be uploaded to the cloud.
     */
    public interface ConversationsReportUploadListener { public void readyForUpload(); }

    private synchronized void addConversation(Conversation conversation) {
        mConversations.put(conversation.getId(), conversation);
    }

    /**
     * When a message is updated, a request is made to upload conversations report.
     * To avoid uploading a conversations report per message update, we will wait
     * for a short time to batch more updates into a single conversations report.
     */
    private void notifyUpload() {
        synchronized (mNotificationLock) {
            mLastNotification = System.currentTimeMillis();
            if (!mNotificationActive) {
                mNotificationActive = true;
                mNotificationRequestCount = 1;
                executeNotificationWaitLoop();
            } else {
                Log.v(TAG, "Update notification requested count: " + mNotificationRequestCount);
                mNotificationRequestCount++;
                mNotificationLock.notifyAll();
            }
        }
    }

    /**
     * This loop will wait for small amount of time to allow another update to come in.
     * Typically when framework is publishing unread messages, they will be sent every
     * 50-150 milliseconds. So if we see continue seeing updates come in, then we will wait
     * until a maximum of MAX_MESSAGES updates are seen before sending report. Otherwise,
     * if no updates were seen, then we will upload the report.
     */
    private void executeNotificationWaitLoop() {
        // clang-format off
        mExecutor.submit(() -> {
            synchronized (mNotificationLock) {
                try {
                    Log.v(TAG, "Notifying started");
                    for (; ; ) {
                        mNotificationLock.wait(NOTIFICATION_WAIT_TIME_MILLIS);
                        boolean noUpdatesWhileWaiting =
                                (System.currentTimeMillis() - mLastNotification) >= NOTIFICATION_WAIT_TIME_MILLIS;

                        if (mNotificationRequestCount >= MAX_MESSAGES || noUpdatesWhileWaiting) {
                            if (mListener != null) {
                                Log.v(TAG,
                                        "Notifying ready for upload. Update count: " + mNotificationRequestCount);
                                mListener.readyForUpload();
                            }
                            mNotificationActive = false;
                            mNotificationRequestCount = 0;
                            break;
                        }

                        Log.v(TAG, "Wait for more updates");
                    }
                    Log.v(TAG, "Notification loop ending");
                } catch (InterruptedException e) {
                    Log.e(TAG, "Notify interrupted", e);
                }
            }
        });
    }
// clang-format on
}
