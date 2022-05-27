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
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.auth.AuthState;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.auth.AuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.UserIdentity;
import com.amazon.alexa.auto.apps.common.util.FileUtil;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.core.ObservableEmitter;
import io.reactivex.rxjava3.core.Single;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Auth controller for Alexa authorization.
 */
public class LWAAuthController implements AuthController {
    private static final String TAG = LWAAuthController.class.getSimpleName();

    private WeakReference<Context> context;
    private BehaviorSubject<AuthStatus> authStatusColdStream;
    private AACSMessageSender messageSender;
    private AlexaClientEventReceiver mAlexaClientEventReceiver;
    private boolean mIsAlexaConnected;
    private boolean mEnableUserProfile;
    private BroadcastReceiver mCBLAuthReceiver;
    /**
     * Constructs the LWAAuthController.
     */
    public LWAAuthController(WeakReference<Context> context) {
        this.context = context;
        authStatusColdStream = BehaviorSubject.create();
        messageSender = new AACSMessageSender(context, new AACSSender());

        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), getUserIdentity()));

        mIsAlexaConnected = false;

        subscribeAlexaClientConnectionStatus();

        mCBLAuthReceiver = new CBLAuthReceiver();

        IntentFilter filter = new IntentFilter();
        filter.addAction(AACSConstants.ACTION_STATE_CHANGE);

        (context.get()).registerReceiver(mCBLAuthReceiver, filter);

        FileUtil.readAACSConfigurationAsync(context.get()).subscribe(this::isCBLUserProfileEnabled);
    }

    @Override
    public void setAuthMode(AuthMode authMode) {
        Log.v(TAG, "setAuthMode =" + authMode);
        AuthModeStore.persistAuthMode(context.get(), authMode);
    }

    @Override
    public AuthMode getAuthMode() {
        return AuthModeStore.getPersistentAuthMode(context.get());
    }

    @Override
    public boolean isAuthenticated() {
        Context contextStrong = context.get();
        if (contextStrong == null) {
            throw new RuntimeException("Context not valid any more.");
        }

        try {
            return TokenStore.getRefreshToken(contextStrong).isPresent();
        } catch (GeneralSecurityException | IOException e) {
            Log.e(TAG, "Failed to get refresh token.");
            return false;
        }
    }

    @Override
    public void setAuthState(AuthStatus authStatus) {
        authStatusColdStream.onNext(authStatus);
    }

    @Override
    public Observable<AuthWorkflowData> newAuthenticationWorkflow() {
        return Observable.create(emitter -> {
            class AuthEventReceiver {
                @Subscribe
                public void OnReceive(AuthWorkflowData data) {
                    emitter.onNext(data);
                    switch (data.getAuthState()) {
                        case Auth_Provider_Auth_Started:
                            setAuthMode(AuthMode.AUTH_PROVIDER_AUTHORIZATION);
                            if (!startLoginWorkflow()) {
                                emitter.onError(new Exception("Failed to start Login workflow"));
                            }
                            break;
                        case Auth_State_Refreshed:
                            // When Alexa is connected, and the auth token has also been saved, we will send the auth
                            // finished/authorized event to subscribers.
                            if (isAuthenticated()) {
                                publishAuthFinishedStatus(emitter);
                            }
                            break;
                        case CBL_Auth_User_Identity_Saved:
                        case Auth_Provider_Token_Saved:
                            // When CBL user identity or other authorization provider's auth token is saved, we want to
                            // make sure Alexa connection is also established with the user identity or token, then we
                            // send the auth finished/authorized event to subscribers.
                            if (mIsAlexaConnected) {
                                publishAuthFinishedStatus(emitter);
                            }
                            break;
                        case CBL_Auth_Token_Saved:
                            if (mEnableUserProfile) {
                                // If user profile is enabled, we need to make sure we have also saved user identity on
                                // the device, so that we can get the user name and display it on the screen when
                                // needed.
                                Log.d(TAG,
                                        "User profile is enabled, waiting for user identity to be saved successfully.");
                            } else {
                                if (mIsAlexaConnected) {
                                    // If user profile is disabled, we can send the CBL_Auth_Finished event now.
                                    emitter.onNext(new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null));
                                    authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), getUserIdentity()));
                                }
                            }
                            break;
                    }
                }
            }

            AuthEventReceiver eventReceiver = new AuthEventReceiver();
            EventBus.getDefault().register(eventReceiver);

            if (getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
                emitter.onNext(new AuthWorkflowData(AuthState.CBL_Auth_Not_Started, null, null));
            }
            if (!startLoginWorkflow()) {
                emitter.onError(new Exception("Failed to start Login workflow"));
            }

            emitter.setCancellable(() -> {
                EventBus.getDefault().unregister(eventReceiver);

                // When subscriber has cancelled the workflow.
                if (!isAuthenticated()) {
                    cancelLoginWorkflow();
                }
            });
        });
    }

    @Override
    public Observable<AuthStatus> observeAuthChangeOrLogOut() {
        return authStatusColdStream;
    }

    @Override
    public void logOut() {
        Context strongContext = context.get();
        String service = "";

        if (strongContext == null) {
            Log.w(TAG, "Cannot cancel login workflow. Context is invalid");
            return;
        }

        Log.d(TAG, "Resetting the login state");

        if (getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_CBL_SERVICE_NAME;
        } else if (getAuthMode().equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_PROVIDER_SERVICE_NAME;
        }
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(LWAAuthConstants.AUTH_SERVICE)
                                     .value(service)
                                     .endObject()
                                     .toString();
            messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.LOGOUT, payload);
        } catch (JSONException e) {
            Log.e(TAG, "Fail to generate authorization logout message.");
        }

        try {
            TokenStore.resetRefreshToken(strongContext);
        } catch (GeneralSecurityException | IOException e) {
            Log.e(TAG, "Failed to reset refresh token.");
        }

        AuthModeStore.resetPersistentAuthMode(context.get());

        try {
            UserIdentityStore.resetUserIdentity(strongContext);
        } catch (GeneralSecurityException | IOException e) {
            Log.e(TAG, "Failed to reset user identity.");
        }

        authStatusColdStream.onNext(new AuthStatus(false, null));
    }

    @Override
    public void cancelLogin(AuthMode mode) {
        Context strongContext = context.get();
        String service = "";

        if (strongContext == null) {
            Log.w(TAG, "Cannot cancel login workflow. Context is invalid");
            return;
        }

        Log.d(TAG, "Cancelling the login state");

        if (getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_CBL_SERVICE_NAME;
        } else if (getAuthMode().equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_PROVIDER_SERVICE_NAME;
        }

        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(LWAAuthConstants.AUTH_SERVICE)
                                     .value(service)
                                     .endObject()
                                     .toString();
            messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.CANCEL_AUTHORIZATION, payload);
        } catch (JSONException e) {
            Log.e(TAG, "Fail to generate authorization logout message.");
        }

        AuthModeStore.resetPersistentAuthMode(context.get());
    }

    @Override
    public UserIdentity getUserIdentity() {
        if (getAuthMode() != null) {
            if (isAuthenticated() && getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
                try {
                    String userName = UserIdentityStore.getUserIdentity(context.get());

                    if (userName != null) {
                        return new UserIdentity(userName);
                    }
                    return null;
                } catch (GeneralSecurityException | IOException e) {
                    Log.e(TAG, "Fail to get user identity data.");
                    return null;
                }
            } else {
                Log.w(TAG, "Device is not authenticated or it is not for CBL login, we cannot get user identity data.");
                return null;
            }
        }
        return null;
    }

    private boolean startLoginWorkflow() {
        Context strongContext = context.get();
        String service = "";

        if (strongContext == null) {
            Log.w(TAG, "Cannot start login workflow. Context is invalid");
            return false;
        }

        // Then send a Start request.
        Log.i(TAG, "Starting new Login workflow");

        if (getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_CBL_SERVICE_NAME;
        } else if (getAuthMode().equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_PROVIDER_SERVICE_NAME;
        }

        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(LWAAuthConstants.AUTH_SERVICE)
                                     .value(service)
                                     .key(LWAAuthConstants.AUTH_DATA)
                                     .value("")
                                     .endObject()
                                     .toString();
            messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.START_AUTHORIZATION, payload);
        } catch (JSONException e) {
            Log.e(TAG, "Fail to generate start authorization message.");
        }
        return true;
    }

    private void cancelLoginWorkflow() {
        Context strongContext = context.get();
        String service = "";

        if (strongContext == null) {
            Log.w(TAG, "Cannot start login workflow. Context is invalid");
            return;
        }

        Log.i(TAG, "Cancelling the login workflow (if prior login workflow exists).");

        if (getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_CBL_SERVICE_NAME;
        } else if (getAuthMode().equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
            service = LWAAuthConstants.AUTH_PROVIDER_SERVICE_NAME;
        }

        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(LWAAuthConstants.AUTH_SERVICE)
                                     .value(service)
                                     .endObject()
                                     .toString();
            messageSender.sendMessage(Topic.AUTHORIZATION, Action.Authorization.CANCEL_AUTHORIZATION, payload);
        } catch (JSONException e) {
            Log.e(TAG, "Fail to generate cancel authorization message.");
        }
    }

    private void isCBLUserProfileEnabled(@NonNull String configs) {
        try {
            JSONObject config = new JSONObject(configs);
            String mUserProfileConfig = config.getJSONObject("aacs.cbl").getString("enableUserProfile");
            mEnableUserProfile = mUserProfileConfig.equals("true");
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse enableUserProfile config" + e);
        }
    }

    private void publishAuthFinishedStatus(ObservableEmitter emitter) {
        if (getAuthMode().equals(AuthMode.CBL_AUTHORIZATION)) {
            emitter.onNext(new AuthWorkflowData(AuthState.CBL_Auth_Finished, null, null));
        } else {
            emitter.onNext(new AuthWorkflowData(AuthState.Auth_Provider_Authorized, null, null));
        }
        authStatusColdStream.onNext(new AuthStatus(isAuthenticated(), getUserIdentity()));
    }

    /**
     * Subscribe Alexa client connection event during the app life cycle, user can choose to finish CBL login
     * with phone and skip the CBL login steps from the head unit app. When user finishes CBL login with phone,
     * and Alexa client connection state changes to CONNECTED, the Alexa client connected event will be sent and
     * captured by AlexaClientEventReceiver, and the remaining setup steps will be triggered from the app.
     */
    public void subscribeAlexaClientConnectionStatus() {
        if (mAlexaClientEventReceiver == null) {
            mAlexaClientEventReceiver = new AlexaClientEventReceiver();
            EventBus.getDefault().register(mAlexaClientEventReceiver);
        }
    }

    class AlexaClientEventReceiver {
        @Subscribe
        public void OnReceive(AuthWorkflowData data) {
            if (data.getAuthState().equals(AuthState.Alexa_Client_Connected)) {
                setAuthState(new AuthStatus(isAuthenticated(), getUserIdentity()));
                mIsAlexaConnected = true;
            } else if (data.getAuthState().equals(AuthState.Alexa_Client_Disconnected)) {
                mIsAlexaConnected = false;
            } else if (data.getAuthState().equals(AuthState.Alexa_Client_Auth_Unintialized)) {
                AuthMode authMode = getAuthMode();

                if (authMode.equals(AuthMode.CBL_AUTHORIZATION)) {
                    Log.w(TAG, "AuthState.Alexa_Client_Auth_Unintialized authmode = " + getAuthMode());
                    // check if we have a valid refresh token then send start authorization
                    try {
                        Context contextStrong = context.get();
                        if (contextStrong == null) {
                            throw new RuntimeException("Context not valid any more.");
                        }

                        Optional<String> refreshToken = TokenStore.getRefreshToken(contextStrong);

                        if (refreshToken.isPresent()) {
                            Log.w(TAG, "refreshToken.isPresent() ");
                            try {
                                JSONObject payloadJson;
                                payloadJson = new JSONObject();
                                payloadJson.put("refreshToken", refreshToken.orElse(""));
                                Log.w(TAG, "refreshToken.isPresent() refreshToken=" + refreshToken);
                                String payload = new JSONStringer()
                                                         .object()
                                                         .key(LWAAuthConstants.AUTH_SERVICE)
                                                         .value(LWAAuthConstants.AUTH_CBL_SERVICE_NAME)
                                                         .key(LWAAuthConstants.AUTH_DATA)
                                                         .value(payloadJson.toString())
                                                         .endObject()
                                                         .toString();
                                messageSender.sendMessage(
                                        Topic.AUTHORIZATION, Action.Authorization.START_AUTHORIZATION, payload);
                            } catch (JSONException e) {
                                Log.e(TAG, "Fail to generate start authorization message.");
                            }
                        }
                    } catch (GeneralSecurityException | IOException e) {
                        Log.e(TAG, "Failed to get refresh token.");
                    }
                } else if (authMode.equals(AuthMode.AUTH_PROVIDER_AUTHORIZATION)) {
                    try {
                        String payload = new JSONStringer()
                                                 .object()
                                                 .key(LWAAuthConstants.AUTH_SERVICE)
                                                 .value(LWAAuthConstants.AUTH_PROVIDER_SERVICE_NAME)
                                                 .key(LWAAuthConstants.AUTH_DATA)
                                                 .value("")
                                                 .endObject()
                                                 .toString();
                        messageSender.sendMessage(
                                Topic.AUTHORIZATION, Action.Authorization.START_AUTHORIZATION, payload);
                    } catch (JSONException e) {
                        Log.e(TAG, "Fail to generate start authorization message.");
                    }
                }
            } else if (data.getAuthState().equals(AuthState.Auth_Provider_Authorization_Expired)) {
                Log.d(TAG, "Authorization is expired, start logging out");
                AlexaApp.from(context.get()).getRootComponent().getAlexaSetupController().setSetupCompleteStatus(false);
                logOut();
            }
        }
    }
}
