package com.amazon.alexaautoclientservice.modules.alexaClient;

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;

import org.json.JSONObject;

import java.util.HashSet;
import java.util.Set;

public class AlexaClientMessageHandler {
    private static final String TAG = AACSConstants.AACS + "-" + AlexaClientMessageHandler.class.getSimpleName();
    private static String sCurrentConnectionState = "";

    private Set<AuthStateObserver> mObservers = new HashSet<>();
    private String mAuthState = AASBConstants.AlexaClient.AUTH_STATE_UNINITIALIZED;

    public static String getCurrentConnectionState() {
        return sCurrentConnectionState;
    }

    public AlexaClientMessageHandler() {
        sCurrentConnectionState = "";
    }

    public void handleAlexaClientMessage(
            @NonNull String messageId, @NonNull String topic, @NonNull String action, @NonNull String payload) {
        Log.d(TAG, "handleAlexaClientMessage " + action + payload);
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
        synchronized (mObservers) {
            if (observer == null)
                return;
            mObservers.add(observer);
            observer.onAuthStateChanged(mAuthState);
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
        sCurrentConnectionState = status;
    }

    private void notifyAuthStateObservers(String authState) {
        synchronized (mObservers) {
            for (AuthStateObserver observer : mObservers) {
                observer.onAuthStateChanged(authState);
            }
        }
    }

    public void cleanUp() {
        synchronized (mObservers) {
            mObservers.clear();
        }
    }
}
