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

package com.amazon.aacstts;

import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacstts.handler.IAACSMessageHandler;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

/**
 * MessageHandler receives all the AASB messages sent from AACS
 * and identifies the right {@link IAACSMessageHandler} to be used for
 * handling the AASB message based on the topic and the action specified in the message.
 */
public class MessageHandler extends Handler {
    private static final String TAG = AACS_TTS_LOG_PREFIX + MessageHandler.class.getSimpleName();

    public static final String MESSAGE_KEY = "message";

    private Map<String, IAACSMessageHandler> mHandlers;

    public MessageHandler() {
        mHandlers = new HashMap<>();
    }

    @Override
    public void handleMessage(Message msg) {
        final String message = msg.getData().getString(MESSAGE_KEY);

        try {
            final JSONObject aasbMessage = new JSONObject(message);

            final String topic = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                         .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                         .getString(AASBConstants.TOPIC);
            final String action = aasbMessage.getJSONObject(AASBConstants.HEADER)
                                          .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                          .getString(AASBConstants.ACTION);
            final String messageId = aasbMessage.getJSONObject(AASBConstants.HEADER).getString(AASBConstants.ID);

            String payload = "";
            if (aasbMessage.has(AASBConstants.PAYLOAD)) {
                payload = aasbMessage.getJSONObject(AASBConstants.PAYLOAD).toString();
            }

            if (mHandlers.containsKey(topic)) {
                IAACSMessageHandler aacsMessageHandler = mHandlers.get(topic);

                if (aacsMessageHandler == null) {
                    Log.e(TAG,
                            String.format(
                                    "Error encountered when handling message because AACSMessageHandler for topic:[%s] was null",
                                    topic));
                    return;
                }
                aacsMessageHandler.handleAACSMessage(messageId, topic, action, payload);
            } else {
                Log.w(TAG, String.format("Topic : [%s] was not recognized. so failed handling the message.", topic));
            }

        } catch (final Exception e) {
            Log.d(TAG, String.format("Failed to handle %s message. %s ", message, e.toString()));
        }
    }

    public MessageHandler register(String category, IAACSMessageHandler handler) {
        mHandlers.put(category, handler);
        return this;
    }

    public void cleanUp() {
        for (IAACSMessageHandler aacsMessageHandler : mHandlers.values()) {
            aacsMessageHandler.cleanUp();
        }
    }
}
