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
package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.aacsconstants.Topic;

import org.json.JSONException;
import org.json.JSONStringer;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;

/**
 * Parse Speech recognizer domain messages and send Speech recognizer messages
 * to AACS with this helper.
 */
public class SpeechRecognizerMessages {
    private static final String TAG = SpeechRecognizerMessages.class.getSimpleName();

    @NonNull
    private AACSMessageSender mAACSSender;

    public SpeechRecognizerMessages(@NonNull AACSMessageSender aacsSender) {
        mAACSSender = aacsSender;
    }

    /**
     * Send start audio capture message to AACS.
     *
     * @param initiatorType Type of initiator such as tap to talk, push to
     *                      talk etc.
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendStartCapture(@NonNull String initiatorType) {
        return sendStartCapture(initiatorType, "", -1, -1);
    }

    /**
     * Send start audio capture message to AACS.
     *
     * @param initiatorType Type of initiator such as tap to talk, push to
     *                      talk, wakeword etc.
     * @param keyword Wakeword.
     * @param keywordBeginIndex Starting index of wakeword in audio stream.
     * @param keywordEndIndex End index of wakeword in audio stream.
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendStartCapture(
            @NonNull String initiatorType, @Nullable String keyword, int keywordBeginIndex, int keywordEndIndex) {
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(AASBConstants.SpeechRecognizer.SPEECH_INITIATOR)
                                     .value(initiatorType)
                                     .key(AASBConstants.SpeechRecognizer.KEYWORD)
                                     .value(keyword == null ? "" : keyword)
                                     .key(AASBConstants.SpeechRecognizer.KEYWORD_BEGIN)
                                     .value(keywordBeginIndex)
                                     .key(AASBConstants.SpeechRecognizer.KEYWORD_END)
                                     .value(keywordEndIndex)
                                     .endObject()
                                     .toString();

            return mAACSSender.sendMessage(Topic.SPEECH_RECOGNIZER, Action.SpeechRecognizer.START_CAPTURE, payload);
        } catch (JSONException exception) {
            Log.w(TAG, "Failed to send start capture message. Error: " + exception);
            CompletableFuture<Boolean> future = new CompletableFuture<>();
            future.completeExceptionally(exception);
            return future;
        }
    }
}
