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

package com.amazon.aacstts.handler;

import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;

import android.util.Log;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Optional;

/**
 * AlexaClientHandler is an AACS message handler which implements {@link IAACSMessageHandler}. It is registered
 * with {@link com.amazon.aacstts.MessageHandler} and is used for handling messages with AlexaClient topic.
 */
public class AlexaClientHandler implements IAACSMessageHandler {
    private static final String TAG = AACS_TTS_LOG_PREFIX + AlexaClientHandler.class.getSimpleName();
    private Optional<Boolean> mIsAlexaClientConnected = Optional.empty();

    public Optional<Boolean> isAlexaClientConnected() {
        return mIsAlexaClientConnected;
    }

    @Override
    public void handleAACSMessage(String messageId, String topic, String action, String payload) {
        Log.d(TAG, "handleAlexaClient." + action + payload);

        if (action.equals(Action.AlexaClient.CONNECTION_STATUS_CHANGED)) {
            handleConnectionStatusChanged(payload);
        }
    }

    /**
     * Handles AASB AlexaClient message with ConnectionStatusChanged action.
     * Updates the UI to reflect the changed connection status.
     *
     * @param payload String payload of AASB ConnectionStatusChanged message
     */
    private void handleConnectionStatusChanged(String payload) {
        Log.d(TAG, "handleConnectionStatusChanged " + payload);

        String status = null;
        try {
            JSONObject obj = new JSONObject(payload);
            if (obj.has("status")) {
                status = obj.getString("status");
            }
        } catch (JSONException e) {
            Log.e(TAG, "ConnectionStatusChanged JSON cannot be parsed.");
        }

        switch (status) {
            case AASBConstants.AlexaClient.ALEXA_CLIENT_STATUS_CONNECTED:
                mIsAlexaClientConnected = Optional.of(true);
                break;
            default:
                mIsAlexaClientConnected = Optional.of(false);
        }
    }

    @Override
    public void cleanUp() {}
}
