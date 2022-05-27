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
package com.amazon.alexa.auto.templateruntime.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.DialogStateChangedMessages;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionViewController;

import org.greenrobot.eventbus.EventBus;

/**
 * Receives Alexa dialog state change broadcasts. When state changes to idle and weather template is
 * rendered on the screen, it publishes an event which is received by
 * {@link
 * com.amazon.alexa.auto.templateruntime.weather.WeatherDirectiveHandler#OnReceive(AlexaVoiceoverCompletedMessage)}
 * {@link
 * com.amazon.alexa.auto.templateruntime.bodytemplate.BodyTemplateDirectiveHandler#OnReceive(AlexaVoiceoverCompletedMessage)}
 * {@link
 * com.amazon.alexa.auto.templateruntime.listtemplate.ListDirectiveHandler#OnReceive(AlexaVoiceoverCompletedMessage)}
 */
public class AlexaStateChangeReceiver extends BroadcastReceiver {
    private static final String TAG = TemplateRuntimeReceiver.class.getSimpleName();
    public static final String ALEXA_VOICE_INTERACTION_STATE_IDLE = "IDLE";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceived");
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            if (Action.AlexaClient.DIALOG_STATE_CHANGED.equals(message.action)) {
                DialogStateChangedMessages.parseDialogState(message.payload)
                        .filter(state -> state.equals(ALEXA_VOICE_INTERACTION_STATE_IDLE))
                        .flatMap(state
                                -> AlexaApp.from(context).getRootComponent().getComponent(SessionViewController.class))
                        .flatMap(SessionViewController::getTemplateRuntimeViewContainer)
                        .ifPresent(viewGroup -> {
                            EventBus.getDefault().post(new AlexaVoiceoverCompletedMessage(System.currentTimeMillis()));
                        });
            }
        });
    }
}
