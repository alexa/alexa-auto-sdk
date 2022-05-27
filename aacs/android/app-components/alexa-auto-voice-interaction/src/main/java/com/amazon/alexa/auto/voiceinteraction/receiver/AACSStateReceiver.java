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
package com.amazon.alexa.auto.voiceinteraction.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.aacs_annotation_api.ContextBroadcastReceiver;
import com.amazon.alexa.auto.apis.app.AlexaApp;

@ContextBroadcastReceiver(actions = {AACSConstants.ACTION_STATE_CHANGE})
public class AACSStateReceiver extends BroadcastReceiver {
    private static final String TAG = AACSStateReceiver.class.getSimpleName();
    private static AACSStateReceiver INSTANCE;

    private AACSStateReceiver() {}
    public static AACSStateReceiver getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new AACSStateReceiver();
        }
        return INSTANCE;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }
        if (AACSConstants.ACTION_STATE_CHANGE.equals(intent.getAction())) {
            String newState = "";
            if (intent.hasExtra("state")) {
                newState = intent.getStringExtra("state");
                Log.i(TAG, "AACS state changed. new state: " + newState);
                if (newState.equals(AACSConstants.State.ENGINE_INITIALIZED.name())) {
                    AlexaApp.from(context).getRootComponent().getAlexaSetupController().setAACSReadiness(true);
                } else if (newState.equals(AACSConstants.State.STOPPED.name())
                        || newState.equals(AACSConstants.State.STARTED.name())) {
                    AlexaApp.from(context).getRootComponent().getAlexaSetupController().setAACSReadiness(false);
                }
            }
        }
    }
}
