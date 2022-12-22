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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;

import java.util.Optional;

/**
 * Handles SMS related intents.
 */
public class AACSMessagingReceiver extends BroadcastReceiver {
    private static final String TAG = AACSMessagingReceiver.class.getCanonicalName();
    private static MessagingHandler mMessagingHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "Messaging intent received: " + intent.getAction());

        if (mMessagingHandler == null) {
            mMessagingHandler = MessagingHandler.getInstance(context);
        }

        // AASB directive intents
        Optional<AACSMessage> messageOptional = AACSMessageBuilder.parseEmbeddedIntent(intent);
        if (messageOptional.isPresent()) {
            AACSMessage message = messageOptional.get();
            if (Topic.MESSAGING.equals(message.topic)) {
                switch (message.action) {
                    case Action.Messaging.SEND_MESSAGE:
                        mMessagingHandler.sendMessage(message.payload);
                        break;
                    case Action.Messaging.UPDATE_MESSAGES_STATUS:
                        mMessagingHandler.updateMessagesStatus(message.payload);
                        break;
                    case Action.Messaging.UPLOAD_CONVERSATIONS:
                        mMessagingHandler.uploadConversationsReport(message.payload);
                        break;
                }
            }
        } else {
            Log.e(TAG, "AACS SMS Intent Message cannot be parsed: " + intent);
        }
    }
}