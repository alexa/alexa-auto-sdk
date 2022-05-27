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

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;

import org.greenrobot.eventbus.EventBus;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.security.GeneralSecurityException;
import java.util.Optional;

public class CBLAuthReceiver extends BroadcastReceiver {
    private static final String TAG = CBLAuthReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        // if intent is aacs state and not embedded intent handle it here
        if (AACSConstants.ACTION_STATE_CHANGE.equals(intent.getAction())) {
            // check if it is engine initialized and send start auth data if refresh token is present
            String newState = "";

            if (intent.hasExtra("state")) {
                newState = intent.getStringExtra("state");

                if (newState.equals(AACSConstants.State.ENGINE_INITIALIZED.name())) {
                    EventBus.getDefault().post(
                            new AuthWorkflowData(AuthState.Alexa_Client_Auth_Unintialized, null, null));
                }
            }

        } else {
            AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
                try {
                    JSONObject obj = new JSONObject(message.payload);
                    String service = obj.getString(LWAAuthConstants.AUTH_SERVICE);

                    if (service.equals(LWAAuthConstants.AUTH_CBL_SERVICE_NAME)) {
                        switch (message.action) {
                            case Action.Authorization.EVENT_RECEIVED:
                                handleAuthorizationEvent(context, obj);
                                break;
                            case Action.Authorization.AUTHORIZATION_STATE_CHANGED:
                                handleAuthorizationStateChanged(obj);
                                break;
                            case Action.Authorization.AUTHORIZATION_ERROR:
                                handleAuthorizationError(obj);
                                break;
                            case Action.Authorization.SET_AUTHORIZATION_DATA:
                                handleSetAuthorizationData(context, obj);
                                break;
                            case Action.Authorization.GET_AUTHORIZATION_DATA:
                                handleGetAuthorizationData(context, message);
                                break;
                        }
                    }
                } catch (JSONException e) {
                    Log.e(TAG, "Authorization event JSON cannot be parsed.");
                }
            });
        }
    }

    private void handleAuthorizationEvent(Context context, JSONObject obj) {
        try {
            String event = obj.getString("event");

            JSONObject eventObj = new JSONObject(event);
            String typeValue = eventObj.getString("type");

            if (!typeValue.isEmpty()) {
                JSONObject payloadObj = new JSONObject(eventObj.getString("payload"));

                if (typeValue.equals("cbl-code")) {
                    String url = payloadObj.getString("url");
                    String code = payloadObj.getString("code");

                    if (code.isEmpty() || url.isEmpty()) {
                        Log.w(TAG, "CBL code or URL empty.");
                    }
                    EventBus.getDefault().post(
                            new AuthWorkflowData(AuthState.CBL_Auth_CodePair_Received, new CodePair(url, code), null));
                } else if (typeValue.equals("user-profile")) {
                    String email = payloadObj.getString("email");
                    String name = payloadObj.getString("name");

                    if (email.isEmpty() || name.isEmpty()) {
                        Log.w(TAG, "email  or name empty.");
                    } else {
                        UserIdentityStore.saveUserIdentity(context, name);
                        // Need to check if we need to store email as well
                        EventBus.getDefault().post(
                                new AuthWorkflowData(AuthState.CBL_Auth_User_Identity_Saved, null, null));
                    }
                }
            }
        } catch (Exception e) {
            Log.e(TAG, "Authorization event JSON cannot be parsed.");
        }
    }

    private void handleAuthorizationStateChanged(JSONObject obj) {
        try {
            String authState = obj.getString("state");

            switch (authState) {
                case "AUTHORIZING":
                    EventBus.getDefault().post(new AuthWorkflowData(AuthState.CBL_Auth_Started, null, null));
                    break;
            }
        } catch (Exception e) {
            Log.e(TAG, "handleAuthorizationStateChanged JSONException.");
        }
    }

    private void handleAuthorizationError(JSONObject obj) {
        try {
            String error = obj.getString("error");

            switch (error) {
                case "AUTH_FAILURE":
                    Log.w(TAG, "Auth token failure.");
                    break;
                case "UNKNOWN_ERROR":
                    Log.e(TAG, "Unknown error AuthError message");
                    break;
                case "START_AUTHORIZATION_FAILED":
                    Log.e(TAG, "Start Authorization Failed");
                    break;
                case "LOGOUT_FAILED":
                    Log.e(TAG, "Logout Failed message");
                    break;
                case "AUTHORIZATION_EXPIRED":
                    EventBus.getDefault().post(
                            new AuthWorkflowData(AuthState.Auth_Provider_Authorization_Expired, null, null));
                    break;
            }
        } catch (Exception e) {
            Log.e(TAG, "Authorization event JSON cannot be parsed.");
        }
    }

    private void handleSetAuthorizationData(Context context, JSONObject obj) {
        String refreshToken = "";
        try {
            String data = obj.getString("data");
            if (data.isEmpty()) {
                Log.d(TAG, "handle clear authorization data");
                TokenStore.resetRefreshToken(context);
            } else {
                JSONObject dataObj = new JSONObject(data);
                refreshToken = dataObj.getString("refreshToken");

                if (refreshToken.isEmpty()) {
                    Log.w(TAG, "refreshToken is empty.");
                }

                TokenStore.saveRefreshToken(context, refreshToken);
                EventBus.getDefault().post(new AuthWorkflowData(AuthState.CBL_Auth_Token_Saved, null, null));
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to handle SetAuthorizationData. Exception: ");
            return;
        }
    }

    private void handleGetAuthorizationData(Context context, AACSMessage message) {
        try {
            Optional<String> refreshToken = TokenStore.getRefreshToken(context);
            JSONObject payloadJson;

            payloadJson = new JSONObject();
            payloadJson.put("refreshToken", refreshToken.orElse(""));

            String payload =
                    new JSONStringer().object().key("data").value(payloadJson.toString()).endObject().toString();
            new AACSMessageSender(new WeakReference<>(context), new AACSSender())
                    .sendReplyMessage(message.messageId, Topic.AUTHORIZATION,
                            Action.Authorization.GET_AUTHORIZATION_DATA, payload);

        } catch (Exception e) {
            Log.e(TAG, "Failed to handle GetAuthorizationData. Exception: " + e.getMessage());
            return;
        }
    }
}
