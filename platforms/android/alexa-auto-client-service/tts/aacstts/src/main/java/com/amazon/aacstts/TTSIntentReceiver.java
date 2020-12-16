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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import com.amazon.aacsipc.IPCConstants;

/**
 * TTSIntentReceiver is a broadcastReceiver which receives all the intents broadcast from AACS
 * and uses {@link com.amazon.aacsipc.AACSReceiver} to receive the embedded or streamed message.
 */
public class TTSIntentReceiver extends BroadcastReceiver {
    private static final String TAG = AACS_TTS_LOG_PREFIX + TTSIntentReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, String.format("receive intent: %s", intent));
        Bundle bundle = intent.getExtras().getBundle(IPCConstants.AACS_IPC_MESSAGE_PAYLOAD);
        if (bundle == null) {
            Log.w(TAG,
                    String.format("onReceive: the bundle received is null. Doing nothing for intent with action %s",
                            intent.getAction()));
            return;
        }

        final String type = bundle.getString(IPCConstants.AACS_IPC_MESSAGE_TYPE);
        if (AmazonTextToSpeechService.mAACSReceiver == null) {
            Log.w(TAG,
                    String.format("onReceive: AACSReceiver is null. Doing nothing for intent with action %s",
                            intent.getAction()));
            return;
        }
        if (type.equals(IPCConstants.AacsIpcMessageType.EMBEDDED.getTypeAsString())) {
            AmazonTextToSpeechService.mAACSReceiver.receive(intent, null);
        } else {
            final PendingResult result = goAsync();
            AmazonTextToSpeechService.mAACSReceiver.receive(intent, result);
        }
    }
}
