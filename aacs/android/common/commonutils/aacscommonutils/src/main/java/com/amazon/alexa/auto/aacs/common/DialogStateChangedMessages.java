package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Dialog State Changed messages.
 */
public class DialogStateChangedMessages {
    private static final String TAG = DialogStateChangedMessages.class.getSimpleName();

    /**
     * Parse DialogStateChanged message.
     *
     * @param json Json to parse.
     * @return dialog state if available.
     */
    public static Optional<String> parseDialogState(@NonNull String json) {
        try {
            JSONObject obj = new JSONObject(json);
            String dialogState = obj.get("state").toString();
            return dialogState != null ? Optional.of(dialogState) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse dialog state from aacs message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }
}
