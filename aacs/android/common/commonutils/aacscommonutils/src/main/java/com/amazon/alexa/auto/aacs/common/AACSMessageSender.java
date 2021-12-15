/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexa.auto.aacs.common;

import android.content.ComponentName;
import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;

/**
 * Class for sending messages to AACS service.
 */
public class AACSMessageSender {
    private static final String TAG = AACSMessageSender.class.getSimpleName();

    @NonNull
    private WeakReference<Context> mContextWk;
    @NonNull
    private AACSSender mAACSSender;
    @NonNull
    private TargetComponent mAACSTarget;

    /**
     * Constructs an instance of @c AACSMessageSender.
     *
     * @param contextWk Android Context which will be used for sending intents
     *                  to AACS.
     * @param sender AACS Sender.
     */
    public AACSMessageSender(@NonNull WeakReference<Context> contextWk, @NonNull AACSSender sender) {
        this.mContextWk = contextWk;
        this.mAACSSender = sender;
        this.mAACSTarget = TargetComponent.withComponent(
                new ComponentName(AACSConstants.getAACSPackageName(contextWk), AACSConstants.AACS_CLASS_NAME),
                TargetComponent.Type.SERVICE);
    }

    /**
     * Send message to AACS.
     *
     * @param topic Topic of the message (typically capability name).
     * @param action Action of the message.
     * @param payload Optional payload of the message.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendMessage(@NonNull String topic, @NonNull String action, @Nullable String payload) {
        Context context = mContextWk.get();
        if (context == null) {
            Log.w(TAG, "Invalid context found while sending message. Topic: " + topic + " Action: " + action);
            return CompletableFuture.completedFuture(false);
        }

        Optional<String> message = AACSMessageBuilder.buildMessage(topic, action, payload);
        if (!message.isPresent()) {
            Log.w(TAG,
                    "Failed to build AACS payload for message. Topic: " + topic + " Action: " + action
                            + " Message payload: " + payload);
            return CompletableFuture.completedFuture(false);
        }

        return this.mAACSSender.sendAASBMessageAnySize(message.get(), action, topic, this.mAACSTarget, context);
    }

    /**
     * Send message to AACS.
     *
     * @param topic Topic of the message (typically capability name).
     * @param action Action of the message.
     * @param payload Optional payload of the message.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public String sendMessageReturnID(@NonNull String topic, @NonNull String action, @Nullable String payload) {
        Context context = mContextWk.get();
        if (context == null) {
            Log.w(TAG, "Invalid context found while sending message. Topic: " + topic + " Action: " + action);
            return "";
        }

        // Create String array to store message [0] and unique messageId [1]
        Optional<String[]> message = AACSMessageBuilder.buildMessageReturnID(topic, action, payload);
        if (!message.isPresent()) {
            Log.w(TAG,
                    "Failed to build AACS payload for message. Topic: " + topic + " Action: " + action
                            + " Message payload: " + payload);
            return "";
        }

        // Send message stored in [0] position
        this.mAACSSender.sendAASBMessageAnySize(message.get()[0], action, topic, this.mAACSTarget, context);
        // return messageId at [1] position
        return message.get()[1];
    }

    /**
     * Send reply message to AACS.
     *
     * @param replyToId Message Id that is sent with AACS Request.
     * @param topic Topic of the message (typically capability name).
     * @param action Action of the message.
     * @param payload Optional payload of the message.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendReplyMessage(
            @NonNull String replyToId, @NonNull String topic, @NonNull String action, @Nullable String payload) {
        Context context = mContextWk.get();
        if (context == null) {
            Log.w(TAG,
                    "Invalid context found while sending reply. ReplyId: " + replyToId + " Topic: " + topic
                            + " Action: " + action);
            return CompletableFuture.completedFuture(false);
        }

        Optional<String> message = AACSMessageBuilder.buildReplyMessage(replyToId, topic, action, payload);
        if (!message.isPresent()) {
            Log.w(TAG,
                    "Failed to build AACS payload for reply. ReplyId: " + replyToId + " Topic: " + topic
                            + " Action: " + action + " Message payload: " + payload);
            return CompletableFuture.completedFuture(false);
        }

        return this.mAACSSender.sendAASBMessageAnySize(message.get(), action, topic, this.mAACSTarget, context);
    }
}
