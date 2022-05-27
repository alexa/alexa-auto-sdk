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
package com.amazon.alexa.auto.settings;

import static com.amazon.aacsconstants.Action.DoNotDisturb.SET_DO_NOT_DISTURB;
import static com.amazon.alexa.auto.apps.common.util.DNDSettingsProvider.DO_NOT_DISTURB_JSON_KEY;
import static com.amazon.alexa.auto.apps.common.util.DNDSettingsProvider.isDNDSettingEnabled;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;

import org.greenrobot.eventbus.EventBus;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Receives Do Not Disturb intents that are broadcast by AACS
 */
public class DNDReceiver extends BroadcastReceiver {
    private static final String TAG = DNDReceiver.class.getCanonicalName();

    /**
     * Listens for Do not disturb messages and updates Shared Preferences accordingly
     *
     * @param context
     * @param intent
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "onReceived");
        if (intent == null || intent.getAction() == null) {
            return;
        }
        Log.d(TAG, intent.getAction());

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            if (SET_DO_NOT_DISTURB.equals(message.action)) {
                if (message.payload == null || message.payload.isEmpty()) {
                    Log.w(TAG, "Received invalid payload");
                    return;
                }

                try {
                    JSONObject messagePayload = new JSONObject(message.payload);
                    boolean value = messagePayload.getBoolean(DO_NOT_DISTURB_JSON_KEY);
                    if (isDNDSettingEnabled(context) != value) {
                        EventBus.getDefault().post(new DNDChangeMessage(value));
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "Error while parsing do not disturb intent" + e);
                }
            } else {
                Log.d(TAG, "Intent action not handled. Action:" + message.action);
            }
        });
    }
}
