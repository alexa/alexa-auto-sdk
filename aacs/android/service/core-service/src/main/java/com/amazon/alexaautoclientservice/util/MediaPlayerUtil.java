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
package com.amazon.alexaautoclientservice.util;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexaautoclientservice.modules.audioOutput.mediaPlayer.EventReceiver;

import org.json.JSONException;
import org.json.JSONObject;

public class MediaPlayerUtil {
    private static final String TAG = AACSConstants.AACS + "-" + MediaPlayerUtil.class.getSimpleName();

    public static void sendMediaStateChangedMessage(@NonNull EventReceiver eventReceiver, @NonNull String channel,
            @NonNull String token, @NonNull String state) {
        JSONObject payload = new JSONObject();
        try {
            payload.put(MediaConstants.CHANNEL, channel);
            payload.put(MediaConstants.TOKEN, token);
            payload.put(MediaConstants.STATE, state);
        } catch (Exception e) {
            Log.e(TAG, "Failed to create mediaStateChanged JSON payload.");
        }
        sendEvent(eventReceiver, "", Topic.AUDIO_OUTPUT, Action.AudioOutput.MEDIA_STATE_CHANGED, payload.toString(),
                channel);
    }

    public static void sendMediaErrorMessage(@NonNull EventReceiver eventReceiver, @NonNull String token,
            @NonNull String error, @NonNull String description, @NonNull String channel) {
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put(MediaConstants.TOKEN, token);
            jsonObject.put(MediaConstants.MEDIA_ERROR, error);
            jsonObject.put(MediaConstants.ERROR_DESCRIPTION, description);
            jsonObject.put(MediaConstants.CHANNEL, channel);
            sendEvent(eventReceiver, "", Topic.AUDIO_OUTPUT, Action.AudioOutput.MEDIA_ERROR, jsonObject.toString(),
                    channel);
        } catch (JSONException e) {
            Log.e(TAG, "Failed to create MediaError JSON payload.");
        }
    }

    public static void sendEvent(@NonNull EventReceiver eventReceiver, @NonNull String replyToId, @NonNull String topic,
            @NonNull String action, @NonNull String payload, @NonNull String channel) {
        Log.v(TAG,
                String.format(
                        "sendEvent: topic=%s, action=%s, payload=%s, channel=%s", topic, action, payload, channel));
        eventReceiver.onEvent(replyToId, topic, action, payload);
    }

    public static void sendAudioFocusEvent(@NonNull EventReceiver eventReceiver, @NonNull String channel,
            @NonNull String token, boolean isClientSideDuckingStarted) {
        JSONObject payload = new JSONObject();
        try {
            payload.put(MediaConstants.CHANNEL, channel);
            payload.put(MediaConstants.TOKEN, token);
            payload.put(MediaConstants.FOCUS_ACTION,
                    isClientSideDuckingStarted ? MediaConstants.AudioFocusEvent.REPORT_DUCKING_STARTED
                                               : MediaConstants.AudioFocusEvent.REPORT_DUCKING_STOPPED);
            Log.d(TAG,
                    "publishing ducking state to AACS. state:"
                            + (isClientSideDuckingStarted ? "REPORT_DUCKING_STARTED" : "REPORT_DUCKING_STOPPED"));
            sendEvent(eventReceiver, "", Topic.AUDIO_OUTPUT, Action.AudioOutput.AUDIO_FOCUS_EVENT, payload.toString(),
                    channel);
        } catch (Exception e) {
            Log.e(TAG, "failed to create AudioFocusEvent JSON payload.");
        }
    }
}
