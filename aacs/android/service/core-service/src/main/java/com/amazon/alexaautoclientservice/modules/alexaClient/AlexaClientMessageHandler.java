/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexaautoclientservice.modules.alexaClient;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.DialogStateChangedMessages;

import org.json.JSONObject;

import java.util.HashSet;
import java.util.Optional;
import java.util.Set;

public class AlexaClientMessageHandler {
    private static final String TAG = AACSConstants.AACS + "-" + AlexaClientMessageHandler.class.getSimpleName();
    private static String sCurrentConnectionState = "";
    private static String sCurrentDialogState = "";

    private Set<AuthStateObserver> mAuthObservers = new HashSet<>();
    private Set<ConnectionStateObserver> mConnectionObservers = new HashSet<>();
    private Set<DialogStateObserver> mDialogStateObservers = new HashSet<>();
    private String mAuthState = AASBConstants.AlexaClient.AUTH_STATE_UNINITIALIZED;

    public static String getCurrentConnectionState() {
        return sCurrentConnectionState;
    }

    public AlexaClientMessageHandler() {
        sCurrentConnectionState = "";
        sCurrentDialogState = "";
    }

    public void handleAlexaClientMessage(
            @NonNull String messageId, @NonNull String topic, @NonNull String action, @NonNull String payload) {
        Log.d(TAG, "handleAlexaClientMessage " + action + payload);
        if (Action.AlexaClient.DIALOG_STATE_CHANGED.equals(action)) {
            handleDialogStateChanged(payload);
        }

        if (Action.AlexaClient.AUTH_STATE_CHANGED.equals(action)) {
            handleAuthStateChanged(payload);
        }

        if (Action.AlexaClient.CONNECTION_STATUS_CHANGED.equals(action)) {
            handleConnectionStatusChanged(payload);
        }
    }

    public void handleAuthStateChanged(@NonNull String payload) {
        JSONObject payloadJson;
        String state;
        if (payload.isEmpty()) {
            Log.e(TAG, "failed to parse AuthStateChanged message because of empty payload.");
            return;
        }
        try {
            payloadJson = new JSONObject(payload);
            state = payloadJson.getString(AASBConstants.AlexaClient.AUTH_STATE);
        } catch (Exception e) {
            Log.e(TAG, String.format("Failed to parse payload. Error=%s", e.getMessage()));
            return;
        }
        notifyAuthStateObservers(state);
        mAuthState = state;
    }

    public void registerAuthStateObserver(AuthStateObserver observer) {
        synchronized (mAuthObservers) {
            if (observer == null)
                return;
            mAuthObservers.add(observer);
            observer.onAuthStateChanged(mAuthState);
        }
    }

    public void registerConnectionStateObserver(ConnectionStateObserver observer) {
        synchronized (mConnectionObservers) {
            if (observer == null)
                return;
            mConnectionObservers.add(observer);
            observer.onConnectionStateChanged(sCurrentConnectionState);
        }
    }

    public void registerDialogStateObserver(DialogStateObserver observer) {
        synchronized (mDialogStateObservers) {
            if (observer == null)
                return;
            mDialogStateObservers.add(observer);
            observer.onDialogStateChanged(sCurrentDialogState);
        }
    }

    private void handleConnectionStatusChanged(@NonNull String payload) {
        JSONObject payloadJson;
        String status;
        if (payload.isEmpty()) {
            Log.e(TAG, "failed to parse ConnectionStatusChanged message because of empty payload.");
            return;
        }
        try {
            payloadJson = new JSONObject(payload);
            status = payloadJson.getString("status");
        } catch (Exception e) {
            Log.e(TAG, String.format("Failed to parse payload. Error=%s", e.getMessage()));
            return;
        }
        notifyConnectionStateObservers(status);
        sCurrentConnectionState = status;
    }

    private void handleDialogStateChanged(@NonNull String payload) {
        if (payload.isEmpty()) {
            Log.e(TAG, "failed to parse DialogStateChanged message because of empty payload.");
            return;
        }
        Optional<String> state = DialogStateChangedMessages.parseDialogState(payload);
        if (state.isPresent()) {
            notifyDialogStateObservers(state.get());
            sCurrentDialogState = state.get();
        }
    }

    private void notifyAuthStateObservers(String authState) {
        synchronized (mAuthObservers) {
            for (AuthStateObserver observer : mAuthObservers) {
                observer.onAuthStateChanged(authState);
            }
        }
    }

    private void notifyConnectionStateObservers(String connectionState) {
        synchronized (mConnectionObservers) {
            for (ConnectionStateObserver observer : mConnectionObservers) {
                observer.onConnectionStateChanged(connectionState);
            }
        }
    }

    private void notifyDialogStateObservers(String dialogState) {
        synchronized (mDialogStateObservers) {
            for (DialogStateObserver observer : mDialogStateObservers) {
                observer.onDialogStateChanged(dialogState);
            }
        }
    }

    public void cleanUp() {
        synchronized (mAuthObservers) {
            mAuthObservers.clear();
        }
        synchronized (mConnectionObservers) {
            mConnectionObservers.clear();
        }
        synchronized (mDialogStateObservers) {
            mDialogStateObservers.clear();
        }
    }
}
