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

import com.amazon.aacsconstants.Action;

/**
 * AASBHandler is an AACS message handler which implements {@link IAACSMessageHandler}. It is registered
 * with {@link com.amazon.aacstts.MessageHandler} and is used for handling messages with AASB topic.
 */
public class AASBHandler implements IAACSMessageHandler {
    private static final String TAG = AACS_TTS_LOG_PREFIX + AASBHandler.class.getSimpleName();

    @Override
    public void handleAACSMessage(String messageId, String topic, String action, String payload) {
        Log.d(TAG, "handleAACSMessage." + action + payload);

        if (action.equals(Action.AASB.STOP_SERVICE)) {
            handleAASBStopServiceAction(payload);
        }
    }

    /**
     * Handles AASB AlexaClient message with StopService action
     * and clears the capabilities cache.
     *
     * @param payload String payload of AASB StopService message
     */
    private void handleAASBStopServiceAction(String payload) {
        Log.d(TAG, "handleAACSStopServiceAction " + payload);
        TTSHandler.clearCapabilitiesCache();
    }

    @Override
    public void cleanUp() {}
}
