package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Connection Status Changed messages.
 */
public class ConnectionStatusChangedMessages {
    private static final String TAG = ConnectionStatusChangedMessages.class.getSimpleName();

    /**
     * Parse ConnectionStatusChanged message.
     *
     * @param json Json to parse.
     * @return connection status if available.
     */
    public static Optional<String> parseConnectionStatus(@NonNull String json) {
        try {
            JSONObject obj = new JSONObject(json);
            String connectionStatus = obj.get("status").toString();
            return connectionStatus != null ? Optional.of(connectionStatus) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse connection status from aacs message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }
}
