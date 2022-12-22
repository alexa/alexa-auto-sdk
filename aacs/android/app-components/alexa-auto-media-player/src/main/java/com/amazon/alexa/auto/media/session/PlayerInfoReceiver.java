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
package com.amazon.alexa.auto.media.session;

import static com.amazon.aacsconstants.AACSConstants.AACS;
import static com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages.parseRenderPlayerInfo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.media.MediaSourceInfo;
import com.amazon.alexa.auto.apis.media.PlayerInfoCache;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Optional;

public class PlayerInfoReceiver extends BroadcastReceiver {
    private static final String TAG = AACS + "-" + PlayerInfoReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceive " + intent.getAction());
        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            @NonNull
            final PlayerInfoCache playerInfoCache = AlexaApp.from(context).getRootComponent().getPlayerInfoCache();

            if (Topic.AUDIO_OUTPUT.equals(message.topic)) {
                String payload = message.payload;
                if (payload == null) {
                    return;
                }
                try {
                    JSONObject payloadJson = new JSONObject(payload);
                    String channel = payloadJson.optString(AASBConstants.AudioOutput.CHANNEL, "");
                    if (!AASBConstants.AudioOutput.Channel.AUDIO_PLAYER.equals(channel)) {
                        return;
                    }

                    if (Action.AudioOutput.PREPARE.equals(message.action)) {
                        String url = payloadJson.optString(AASBConstants.AudioOutput.URL, "");
                        if (url.isEmpty()) {
                            Log.d(TAG, "AudioOutput.Prepare is missing URL");
                            return;
                        }
                        boolean repeating = payloadJson.optBoolean(AASBConstants.AudioOutput.REPEATING, false);
                        String token = payloadJson.optString(AASBConstants.AudioOutput.TOKEN, "");
                        playerInfoCache.setMediaSourceInfo(Optional.of(new MediaSourceInfo(url, token, repeating)));
                        return;
                    }

                } catch (JSONException e) {
                    Log.d(TAG, "Error parsing JSON in onReceive: " + e.toString());
                    return;
                }

            } else if (Topic.TEMPLATE_RUNTIME.equals(message.topic)) {
                if (Action.TemplateRuntime.RENDER_PLAYER_INFO.equals(message.action)) {
                    Log.v(TAG, "renderPlayerInfo payload: " + message.payload);
                    playerInfoCache.setPlayerInfo(parseRenderPlayerInfo(message.payload));
                } else if (Action.TemplateRuntime.CLEAR_PLAYER_INFO.equals(message.action)) {
                    playerInfoCache.setPlayerInfo(Optional.empty());
                    playerInfoCache.setMediaSourceInfo(Optional.empty());
                }
            }
        });
    }
}