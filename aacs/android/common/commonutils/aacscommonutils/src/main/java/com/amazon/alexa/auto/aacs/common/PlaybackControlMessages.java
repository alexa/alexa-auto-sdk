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

import android.os.SystemClock;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.aacsconstants.Topic;

import org.json.JSONException;
import org.json.JSONStringer;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;

/**
 * Parse music domain messages and send music domain messages to AACS
 * with this helper.
 */
public class PlaybackControlMessages {
    private static final String TAG = PlaybackControlMessages.class.getSimpleName();

    @NonNull
    private AACSMessageSender mAACSSender;

    public PlaybackControlMessages(@NonNull AACSMessageSender aacsSender) {
        mAACSSender = aacsSender;
    }

    /**
     * Send Playback control Toggle command to AACS.
     *
     * @param toggle Toggle command to send. Must be one of the
     *               PlaybackConstants.ToggleButton constants.
     * @param selected Whether or not toggle is selected.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendToggleCommandToAACS(@NonNull String toggle, boolean selected) {
        Log.d(TAG, "Sending toggle command: " + toggle + " selected: " + selected);
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(PlaybackConstants.TOGGLE)
                                     .value(toggle)
                                     .key(PlaybackConstants.ACTION)
                                     .value(selected)
                                     .endObject()
                                     .toString();

            return mAACSSender.sendMessage(
                    Topic.PLAYBACK_CONTROLLER, Action.PlaybackController.TOGGLE_PRESSED, payload);
        } catch (JSONException exception) {
            Log.w(TAG, "Failed to send toggle command: " + toggle + ". Error: " + exception);
            CompletableFuture<Boolean> future = new CompletableFuture<>();
            future.completeExceptionally(exception);
            return future;
        }
    }

    /**
     * Send Playback control Button command to AACS.
     *
     * @param command Button command to send. Must be one of the
     *                PlaybackConstants.PlaybackButton constants.
     *
     * @return Future with status of send (true if send succeeded).
     */
    public Future<Boolean> sendButtonCommandToAACS(@NonNull String command) {
        Log.d(TAG, "Sending playback control button command: " + command);
        try {
            String payload =
                    new JSONStringer().object().key(PlaybackConstants.BUTTON).value(command).endObject().toString();

            return mAACSSender.sendMessage(
                    Topic.PLAYBACK_CONTROLLER, Action.PlaybackController.BUTTON_PRESSED, payload);
        } catch (JSONException exception) {
            Log.w(TAG, "Failed to send button command: " + command + ". Error: " + exception);
            CompletableFuture<Boolean> future = new CompletableFuture<>();
            future.completeExceptionally(exception);
            return future;
        }
    }

    public Future<Boolean> sendRequestMediaPlaybackToAACS() {
        Log.d(TAG, "Sending request media playback AASB message");
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(PlaybackConstants.RequestMediaPlayback.INVOCATION_REASON)
                                     .value(PlaybackConstants.RequestMediaPlayback.REASON_AUTOMOTIVE_STARTUP)
                                     .key(PlaybackConstants.RequestMediaPlayback.ELAPSED_BOOT_TIME)
                                     .value(SystemClock.elapsedRealtime())
                                     .endObject()
                                     .toString();

            return mAACSSender.sendMessage(
                    Topic.MEDIA_PLAYBACK_REQUESTER, Action.MediaPlaybackRequestor.REQUEST_MEDIA_PLAYBACK, payload);
        } catch (JSONException exception) {
            Log.w(TAG, "Failed to send button command: Error: " + exception);
            CompletableFuture<Boolean> future = new CompletableFuture<>();
            future.completeExceptionally(exception);
            return future;
        }
    }
}
