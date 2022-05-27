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
package com.amazon.alexa.auto.navigation.receiver;

import static com.amazon.aacsconstants.AACSConstants.PAYLOAD;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSReplyMessage;
import com.amazon.alexa.auto.navigation.dependencies.AndroidModule;
import com.amazon.alexa.auto.navigation.dependencies.DaggerNaviComponent;
import com.amazon.alexa.auto.navigation.handlers.NaviDirectiveHandler;

import java.util.HashSet;
import java.util.Optional;
import java.util.Set;

import javax.inject.Inject;

public class NaviReceiver extends BroadcastReceiver {
    private static final String TAG = NaviReceiver.class.getSimpleName();
    private static final Set<String> mSupportedActions = new HashSet<>();
    {
        // Navigation
        mSupportedActions.add(Action.Navigation.ANNOUNCE_MANEUVER);
        mSupportedActions.add(Action.Navigation.ANNOUNCE_ROAD_REGULATION);
        mSupportedActions.add(Action.Navigation.CANCEL_NAVIGATION);
        mSupportedActions.add(Action.Navigation.CONTROL_DISPLAY);
        mSupportedActions.add(Action.Navigation.GET_NAVIGATION_STATE);
        mSupportedActions.add(Action.Navigation.NAVIGATE_TO_PREVIOUS_WAYPOINT);
        mSupportedActions.add(Action.Navigation.SHOW_ALTERNATIVE_ROUTES);
        mSupportedActions.add(Action.Navigation.SHOW_PREVIOUS_WAYPOINTS);
        mSupportedActions.add(Action.Navigation.START_NAVIGATION);
    }

    @Inject
    NaviDirectiveHandler mNaviDirectiveHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (mNaviDirectiveHandler == null) {
            DaggerNaviComponent.builder()
                    .androidModule(new AndroidModule(context))
                    .build()
                    .injectNavigationReceiver(this);
            (new Handler()).postDelayed(() -> onReceive(context, intent), 200);
            return;
        }

        if (intent.getExtras() != null) {
            AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
                if (Topic.NAVIGATION.equals(message.topic)) {
                    if (mSupportedActions.contains(message.action)) {
                        mNaviDirectiveHandler.handleNavigationCommand(message);
                    }
                }
            });
        }

        Bundle payloadBundle = intent.getBundleExtra(PAYLOAD);
        String replyMessage =
                payloadBundle != null ? payloadBundle.getString(IPCConstants.AACS_IPC_EMBEDDED_MESSAGE) : null;
        Optional<AACSReplyMessage> messageOptional = AACSMessageBuilder.parseReplyMessage(replyMessage);
        if (messageOptional.isPresent()) {
            AACSReplyMessage message = messageOptional.get();
            if (Topic.ADDRESS_BOOK.equals(message.topic)) {
                mNaviDirectiveHandler.handleNavigationCommand(message);
            }
        }
    }
}
