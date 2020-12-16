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

import com.amazon.aace.authorization.Authorization;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.core.AuthorizationHandlerInterface;
import com.amazon.sampleapp.core.AuthorizationHandlerObserverInterface;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.HashMap;

public class AuthorizationHandler extends Authorization implements AuthorizationHandlerInterface {
    private static final String sTag = "Authorization";

    private final LoggerHandler mLogger;

    private final Activity mActivity;

    private final Handler mMainThreadHandler;

    private final SharedPreferences mPreferences;

    private final static HashMap<String, AuthorizationHandlerObserverInterface> mObservers = new HashMap<>();

    public AuthorizationHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
        mPreferences = mActivity.getSharedPreferences(
                activity.getString(R.string.preferred_authorization), Context.MODE_PRIVATE);
    }

    @Override
    public void addObserver(String service, AuthorizationHandlerObserverInterface observer) {
        mObservers.put(service, observer);
        String preferredAuthorization =
                mPreferences.getString(mActivity.getString(R.string.preferred_authorization), "");
        if (!preferredAuthorization.isEmpty()) {
            if (preferredAuthorization.equals(service)) {
                mLogger.postInfo(sTag, "Starting previously authorized " + service + " authorization flow");
                mMainThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mObservers.get(service).enable();
                    }
                });
            }
        }
    }

    @Override
    public void startAuth(String service, String data) {
        startAuthorization(service, data);
    }

    @Override
    public void cancelAuth(String service) {
        cancelAuthorization(service);
    }

    @Override
    public void logoutAuth(String service) {
        logout(service);
    }

    @Override
    public void authEvent(String service, String data) {
        sendEvent(service, data);
    }

    @Override
    public void eventReceived(String service, String request) {
        mObservers.get(service).onEventReceived(request);
    }

    @Override
    public void authorizationStateChanged(String service, Authorization.AuthorizationState state) {
        mObservers.get(service).onAuthorizationStateChanged(state);

        SharedPreferences.Editor editor = mPreferences.edit();
        if (state == Authorization.AuthorizationState.AUTHORIZED) {
            editor.putString(mActivity.getString(R.string.preferred_authorization), service);
        } else if (state == Authorization.AuthorizationState.UNAUTHORIZED) {
            editor.putString(mActivity.getString(R.string.preferred_authorization), "");
        }
        editor.apply();
    }

    @Override
    public void authorizationError(String service, String error, String message) {
        mObservers.get(service).onAuthorizationError(error, message);
    }

    @Override
    public String getAuthorizationData(String service, String key) {
        return mObservers.get(service).onGetAuthorizationData(key);
    }

    @Override
    public void setAuthorizationData(String service, String key, String data) {
        mObservers.get(service).onSetAuthorizationData(key, data);
    }
}
