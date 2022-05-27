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
package com.amazon.alexa.auto.lwa;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AuthStateChangedMessages;
import com.amazon.alexa.auto.aacs.common.ConnectionStatusChangedMessages;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;

import org.greenrobot.eventbus.EventBus;

public class AlexaClientReceiver extends BroadcastReceiver {
    private static final String TAG = AlexaClientReceiver.class.getSimpleName();

    private static final String ALEXA_CLIENT_CONNECTED = "CONNECTED";
    private static final String ALEXA_CLIENT_DISCONNECTED = "DISCONNECTED";
    private static final String AUTH_STATE_REFRESHED = "REFRESHED";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            if (Action.AlexaClient.CONNECTION_STATUS_CHANGED.equals(message.action)) {
                handleConnectionStateChanged(message);
            } else if (Action.AuthProvider.AUTH_STATE_CHANGED.equals(message.action)) {
                handleAuthStateChanged(message);
            }
        });
    }

    /**
     * Handles Alexa Client AASB message with ConnectionStatusChanged action.
     */
    private void handleConnectionStateChanged(AACSMessage aacsMessage) {
        if (aacsMessage.payload != null) {
            ConnectionStatusChangedMessages.parseConnectionStatus(aacsMessage.payload).ifPresent(message -> {
                if (message.equals(ALEXA_CLIENT_CONNECTED)) {
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Alexa_Client_Connected, null, null));
                } else if (message.equals(ALEXA_CLIENT_DISCONNECTED)) {
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Alexa_Client_Disconnected, null, null));
                }
            });
        }
    }

    /**
     * Handles Alexa Client AASB message with AuthStateChanged action.
     */
    private void handleAuthStateChanged(AACSMessage aacsMessage) {
        if (aacsMessage.payload != null) {
            AuthStateChangedMessages.parseAuthState(aacsMessage.payload).ifPresent(message -> {
                if (AUTH_STATE_REFRESHED.equals(message)) {
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Auth_State_Refreshed, null, null));
                }
            });
        }
    }
}
