/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.AlexaClient;

import android.app.Activity;
import android.view.View;
import android.widget.TextView;

import com.amazon.aace.alexa.AlexaClient;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.HashSet;
import java.util.Set;
// AutoVoiceChrome imports

public class AlexaClientHandler extends AlexaClient {
    private static final String TAG = AlexaClientHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private TextView mConnectionText, mAuthText, mDialogText;
    private ConnectionStatus mConnectionStatus = ConnectionStatus.DISCONNECTED;
    private View mLoginView, mLogoutView;

    // List of Authentication observers
    private Set<AuthStateObserver> mObservers;

    // Current AuthState and AuthError
    private AuthState mAuthState;
    private AuthError mAuthError;

    // AutoVoiceChrome controller

    public AlexaClientHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mObservers = new HashSet<>();
        setupGUI();
    }

    @Override
    public void dialogStateChanged(final DialogState state) {
        mLogger.postInfo(TAG, "Dialog State Changed. STATE: " + state);
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mDialogText.setText(state != null ? state.toString() : "");
            }
        });

        // Notify dialog state change to AutoVoiceChrome
    }

    @Override
    public void authStateChanged(final AuthState state, final AuthError error) {
        if (error == AuthError.NO_ERROR) {
            mLogger.postInfo(TAG, "Auth State Changed. STATE: " + state);
        } else {
            mLogger.postWarn(TAG, String.format("Auth State Changed. STATE: %s, ERROR: %s", state, error));
        }
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAuthText.setText(state != null ? state.toString() : "");
            }
        });

        notifyAuthStateObservers(state, error);
        mAuthState = state;
        mAuthError = error;
    }

    @Override
    public void connectionStatusChanged(final ConnectionStatus status, final ConnectionChangedReason reason) {
        mConnectionStatus = status;
        mLogger.postInfo(TAG, String.format("Connection Status Changed. STATUS: %s, REASON: %s", status, reason));
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mConnectionText.setText(status != null ? status.toString() : "");
            }
        });

        // Notify error state change to AutoVoiceChrome
    }

    public ConnectionStatus getConnectionStatus() {
        return mConnectionStatus;
    }

    public void registerAuthStateObserver(AuthStateObserver observer) {
        synchronized (mObservers) {
            if (observer == null)
                return;
            mObservers.add(observer);

            // notify newly registered observer with the current state
            observer.onAuthStateChanged(mAuthState, mAuthError);
        }
    }

    public void removeAuthStateObserver(AuthStateObserver observer) {
        synchronized (mObservers) {
            if (observer == null)
                return;
            mObservers.remove(observer);
        }
    }

    private void notifyAuthStateObservers(AuthState authState, AuthError authError) {
        synchronized (mObservers) {
            for (AuthStateObserver observer : mObservers) {
                observer.onAuthStateChanged(authState, authError);
            }
        }
    }

    private void setupGUI() {
        mConnectionText = mActivity.findViewById(R.id.connectionState);
        mAuthText = mActivity.findViewById(R.id.authState);
        mDialogText = mActivity.findViewById(R.id.dialogState);

        mConnectionText.setText(AlexaClient.ConnectionStatus.DISCONNECTED.toString());
        mAuthText.setText(AlexaClient.AuthState.UNINITIALIZED.toString());
        mDialogText.setText(AlexaClient.DialogState.IDLE.toString());
    }

    // AutoVoiceChrome related functions
}