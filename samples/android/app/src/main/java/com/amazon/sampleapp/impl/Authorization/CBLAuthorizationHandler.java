/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Authorization;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.view.View;

import com.amazon.aace.authorization.Authorization;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.core.AuthorizationHandlerInterface;
import com.amazon.sampleapp.core.AuthorizationHandlerObserverInterface;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONException;
import org.json.JSONObject;

public class CBLAuthorizationHandler implements AuthorizationHandlerObserverInterface {
    private static final String sTag = "CBLAuthorizationHandler";

    private AuthorizationHandlerInterface mAuthorizationHandler;

    private static final String CBL_AUTHORIZATION_SERVICE_NAME = "alexa:cbl";

    private final LoggerHandler mLogger;

    private final Activity mActivity;

    private final Handler mMainThreadHandler;

    private final SharedPreferences mPreferences;

    private View mLoginView, mCancelView, mLogoutView;

    private Authorization.AuthorizationState mState;

    public CBLAuthorizationHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
        mPreferences =
                activity.getSharedPreferences(activity.getString(R.string.preference_file_key), Context.MODE_PRIVATE);
        setupUI();
    }

    @Override
    public void initialize(AuthorizationHandlerInterface handler, JSONObject deviceConfig) {
        mAuthorizationHandler = handler;
        mAuthorizationHandler.addObserver(CBL_AUTHORIZATION_SERVICE_NAME, this);
    }

    @Override
    public void enable() {
        StartAuthorizationFlow();
    }

    @Override
    public void onEventReceived(String request) {
        try {
            JSONObject requestJson = new JSONObject(request);
            String type = requestJson.getString("type");
            if (type.equals("cbl-code")) {
                if (requestJson.has("payload")) {
                    JSONObject payload = requestJson.getJSONObject("payload");
                    JSONObject renderJSON = new JSONObject();
                    renderJSON.put("verification_uri", payload.getString("url"));
                    renderJSON.put("user_code", payload.getString("code"));
                    mLogger.postDisplayCard(renderJSON, LogRecyclerViewAdapter.CBL_CODE);
                } else {
                    mLogger.postError(sTag, "payloadMissing");
                }
            } else if (type.equals("user-profile")) {
                mLogger.postInfo(sTag, "User profile details updated.");
            }
        } catch (Exception e) {
            mLogger.postError(sTag, e.getMessage());
        }
    }

    @Override
    public void onAuthorizationStateChanged(Authorization.AuthorizationState state) {
        mState = state;
        if (mState == Authorization.AuthorizationState.AUTHORIZING) {
            showCancelButton();
        } else if (mState == Authorization.AuthorizationState.AUTHORIZED) {
            showLogoutButton();
        } else if (mState == Authorization.AuthorizationState.UNAUTHORIZED) {
            showLoginButton();
        }
    }

    @Override
    public void onAuthorizationError(String error, String message) {
        if (error == "CODE_PAIR_EXPIRED") {
            showLoginButton();
            try {
                JSONObject renderJSON = new JSONObject();
                String expiredMessage = "The code has expired. Retry to generate a new code.";
                renderJSON.put("message", expiredMessage);
                mLogger.postDisplayCard(renderJSON, LogRecyclerViewAdapter.CBL_CODE_EXPIRED);
            } catch (JSONException e) {
                mLogger.postError(sTag, e.getMessage());
            }
        } else if (error == "AUTHORIZATION_EXPIRED") {
            try {
                JSONObject renderJSON = new JSONObject();
                String expiredMessage = "The token has expired. Log in again.";
                renderJSON.put("message", expiredMessage);
                mLogger.postDisplayCard(renderJSON, LogRecyclerViewAdapter.CBL_CODE_EXPIRED);
            } catch (JSONException e) {
                mLogger.postError(sTag, e.getMessage());
            }
        } else {
            mLogger.postError(sTag, "onAuthorizationError:error=" + error + ":message=" + message);
        }
    }

    @Override
    public String onGetAuthorizationData(String key) {
        if (key.equals("refreshToken")) {
            String refreshToken = mPreferences.getString(mActivity.getString(R.string.preference_refresh_token), "");
            // If persisted refresh token was based on the CBL platform interface, then convert that to JSON
            // format to be used with the Authorization platform interface.
            if (!refreshToken.isEmpty()) {
                try {
                    new JSONObject(refreshToken);
                } catch (JSONException e) {
                    try {
                        JSONObject refreshTokenJson = new JSONObject();
                        refreshTokenJson.put("refreshToken", refreshToken);
                        mLogger.postInfo(sTag, "Converting existing CBL token string to json format");
                        refreshToken = refreshTokenJson.toString();
                    } catch (JSONException exception) {
                        refreshToken = "";
                        mLogger.postError(sTag, exception.getMessage());
                    }
                }
            }
            return refreshToken;
        } else {
            mLogger.postError(sTag, "unhandeledKey");
        }
        return "";
    }

    @Override
    public void onSetAuthorizationData(String key, String data) {
        if (key.equals("refreshToken")) {
            SharedPreferences.Editor editor = mPreferences.edit();
            editor.putString(mActivity.getString(R.string.preference_refresh_token), data);
            editor.apply();
        } else {
            mLogger.postInfo(sTag, "unknownKey");
        }
    }

    private void setupUI() {
        mLoginView = mActivity.findViewById(R.id.cblLogin);
        mLoginView.findViewById(R.id.cblLoginButton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mLogger.postInfo(sTag, "Starting CBL Authorization login flow...");
                StartAuthorizationFlow();
            }
        });

        mCancelView = mActivity.findViewById(R.id.cblCancel);
        mCancelView.findViewById(R.id.cblCancelButton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mLogger.postInfo(sTag, "Cancelling CBL Authorization login flow...");
                mAuthorizationHandler.cancelAuth(CBL_AUTHORIZATION_SERVICE_NAME);
                if ((mState == Authorization.AuthorizationState.AUTHORIZING)) {
                    showLoginButton();
                }
            }
        });

        mLogoutView = mActivity.findViewById(R.id.cblLogout);
        mLogoutView.findViewById(R.id.cblLogoutButton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mLogger.postInfo(sTag, "Resetting CBL Authorization...");
                mAuthorizationHandler.logoutAuth(CBL_AUTHORIZATION_SERVICE_NAME);
            }
        });
    }

    private void showLoginButton() {
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mLoginView.setVisibility(View.VISIBLE);
                mCancelView.setVisibility(View.GONE);
                mLogoutView.setVisibility(View.GONE);
            }
        });
    }

    private void showCancelButton() {
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mLoginView.setVisibility(View.GONE);
                mCancelView.setVisibility(View.VISIBLE);
                mLogoutView.setVisibility(View.GONE);
            }
        });
    }

    private void showLogoutButton() {
        mMainThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                mLoginView.setVisibility(View.GONE);
                mCancelView.setVisibility(View.GONE);
                mLogoutView.setVisibility(View.VISIBLE);
            }
        });
    }

    private void StartAuthorizationFlow() {
        String refreshToken = mPreferences.getString(mActivity.getString(R.string.preference_refresh_token), "");

        // If persisted refresh token was based on the CBL platform interface, then convert that to JSON
        // format to be used with the Authorization platform interface.
        if (!refreshToken.isEmpty()) {
            try {
                new JSONObject(refreshToken);
            } catch (JSONException e) {
                try {
                    JSONObject refreshTokenJson = new JSONObject();
                    refreshTokenJson.put("refreshToken", refreshToken);
                    mLogger.postInfo(sTag, "Converting existing CBL token string to json format");
                    refreshToken = refreshTokenJson.toString();
                } catch (JSONException exception) {
                    refreshToken = "";
                    mLogger.postError(sTag, exception.getMessage());
                }
            }
        }
        mAuthorizationHandler.startAuth(CBL_AUTHORIZATION_SERVICE_NAME, refreshToken);
    }
}
