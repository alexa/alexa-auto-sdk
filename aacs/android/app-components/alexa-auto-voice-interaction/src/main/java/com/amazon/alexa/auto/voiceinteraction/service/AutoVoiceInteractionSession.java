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
package com.amazon.alexa.auto.voiceinteraction.service;

import android.app.assist.AssistContent;
import android.app.assist.AssistStructure;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.service.voice.VoiceInteractionSession;
import android.util.Log;

import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.voice.ui.VoiceActivity;
import com.amazon.alexa.auto.voice.ui.common.Constants;
import com.amazon.alexa.auto.voiceinteraction.common.AutoVoiceInteractionMessage;

import org.jetbrains.annotations.NotNull;

/**
 * Active Alexa Auto voice interaction session, providing a facility for starting a new assistant activity
 * displayed on top of other activities in the system.
 */
public class AutoVoiceInteractionSession extends VoiceInteractionSession {
    private static final String TAG = AutoVoiceInteractionSession.class.getCanonicalName();

    public AutoVoiceInteractionSession(@NotNull Context context) {
        super(context);
    }

    @Override
    public void onShow(Bundle args, int showFlags) {
        Log.d(TAG, "onShow");
        super.onShow(args, showFlags);
        Intent intent = new Intent(getContext(), VoiceActivity.class);
        if (showFlags == SHOW_WITH_ASSIST) {
            String msgTopic = args.getString(AASBConstants.TOPIC, null);
            if (Topic.SPEECH_RECOGNIZER.equals(msgTopic) || Constants.TOPIC_ALEXA_CONNECTION.equals(msgTopic)) {
                Log.d(TAG, "Parsing message from VIS... Sending to VA.");
            } else {
                Log.e(TAG, "onShow called without message from VIS, VA will not be started.");
                return;
            }
        } else {
            Log.d(TAG, "SpeechRecognizer: PTT is detected...");
            args.putString(AASBConstants.TOPIC, Topic.SPEECH_RECOGNIZER);
            args.putString(AASBConstants.ACTION, Action.SpeechRecognizer.START_CAPTURE);
            args.putString(AASBConstants.PAYLOAD, "");
        }

        intent.putExtras(args);
        startAssistantActivity(intent);
    }

    @Override
    public void onPrepareShow(Bundle args, int showFlags) {
        Log.d(TAG, "onPrepareShow");
        super.onPrepareShow(args, showFlags);
        setUiEnabled(false);
    }

    @Override
    public void onHandleAssist(
            @Nullable Bundle data, @Nullable AssistStructure structure, @Nullable AssistContent content) {
        Log.d(TAG, "onHandleAssist");
        super.onHandleAssist(data, structure, content);
    }
}