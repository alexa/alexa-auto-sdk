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
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;

import org.greenrobot.eventbus.EventBus;
import org.json.JSONException;
import org.json.JSONObject;

public class AuthReceiver extends BroadcastReceiver {
    private static final String TAG = AuthReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            try {
                JSONObject obj = new JSONObject(message.payload);
                String service = obj.getString(LWAAuthConstants.AUTH_SERVICE);

                if (service.equals(LWAAuthConstants.AUTH_PROVIDER_SERVICE_NAME)) {
                    switch (message.action) {
                        case Action.Authorization.EVENT_RECEIVED:
                            handleAuthorizationEvent(obj);
                            break;
                        case Action.Authorization.GET_AUTHORIZATION_DATA:
                            EventBus.getDefault().post(new AuthWorkflowData(
                                    AuthState.Auth_Provider_Authorization_Get_Data, null, message.messageId));
                            break;
                        case Action.Authorization.AUTHORIZATION_STATE_CHANGED:
                            handleAuthorizationStateChanged(obj);
                            break;
                        case Action.Authorization.AUTHORIZATION_ERROR:
                            handleAuthorizationError(obj);
                            break;
                    }
                }
            } catch (JSONException e) {
                Log.e(TAG, "Authorization event JSON cannot be parsed.");
            }
        });
    }

    private void handleAuthorizationEvent(JSONObject obj) {
        try {
            String event = obj.getString("event");

            if (event.contains("requestAuthorization")) {
                EventBus.getDefault().post(
                        new AuthWorkflowData(AuthState.Auth_Provider_Request_Authorization, null, null));
            } else if (event.contains("logout")) {
                EventBus.getDefault().post(new AuthWorkflowData(AuthState.Auth_Provider_Logout, null, null));
            }
        } catch (Exception e) {
            Log.e(TAG, "Authorization event JSON cannot be parsed.");
        }
    }

    private void handleAuthorizationStateChanged(JSONObject obj) {
        try {
            String authState = obj.getString("state");

            switch (authState) {
                case "AUTHORIZED":
                    // It means we have successfully received and saved the auth token on the device side.
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Auth_Provider_Token_Saved, null, null));
                    break;
                case "UNAUTHORIZED":
                    EventBus.getDefault().post(
                            new AuthWorkflowData(AuthState.Auth_Provider_Not_Authorized, null, null));
                    break;
                case "AUTHORIZING":
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.Auth_Provider_Authorizing, null, null));
                    break;
            }
        } catch (Exception e) {
            Log.e(TAG, "Authorization event JSON cannot be parsed.");
        }
    }
    private void handleAuthorizationError(JSONObject obj) {
        try {
            String error = obj.getString("error");

            switch (error) {
                case "AUTH_FAILURE":
                    Log.w(TAG, "Auth token failure.");
                    EventBus.getDefault().post(
                            new AuthWorkflowData(AuthState.Auth_Provider_Authorization_Error, null, null));
                    break;
                case "UNKNOWN_ERROR":
                    Log.e(TAG, "Unknown error Authorization message=");
                    break;
                case "START_AUTHORIZATION_FAILED":
                    Log.e(TAG, "Start Authorization Failed message");
                    break;
                case "LOGOUT_FAILED":
                    Log.e(TAG, "Logout Failed. message");
                    break;
            }
        } catch (Exception e) {
            Log.e(TAG, "Authorization event JSON cannot be parsed.");
        }
    }
}
