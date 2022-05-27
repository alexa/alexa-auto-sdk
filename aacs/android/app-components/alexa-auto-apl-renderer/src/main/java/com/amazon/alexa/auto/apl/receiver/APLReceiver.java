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
package com.amazon.alexa.auto.apl.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.fragment.app.Fragment;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionActivityController;
import com.amazon.alexa.auto.apl.APLDirective;
import com.amazon.alexa.auto.apl.APLFragment;
import com.amazon.alexa.auto.apl.Constants;

import org.greenrobot.eventbus.EventBus;

public class APLReceiver extends BroadcastReceiver {
    private static final String TAG = APLReceiver.class.getSimpleName();

    @RequiresApi(api = Build.VERSION_CODES.N)
    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            Log.d(TAG, "APL msg.action " + message.action + " payload: " + message.payload);
            if (Action.APL.RENDER_DOCUMENT.equals(message.action)) {
                handleRenderDocument(context, message);
            } else {
                APLDirective directive = new APLDirective(message);
                EventBus.getDefault().post(directive);
            }
        });
    }

    private void handleRenderDocument(@NonNull Context context, @NonNull final AACSMessage message) {
        AlexaApp app = AlexaApp.from(context);

        app.getRootComponent().getComponent(SessionActivityController.class).ifPresent(sessionActivityController -> {
            Fragment aplFragment;
            Bundle args = new Bundle();
            args.putString(Constants.PAYLOAD, message.payload);
            if (!sessionActivityController.isFragmentAdded()) {
                aplFragment = new APLFragment();
                aplFragment.setArguments(args);
                sessionActivityController.addFragment(aplFragment);
            } else {
                APLDirective directive = new APLDirective(message);
                EventBus.getDefault().post(directive);
            }
        });
    }
}
