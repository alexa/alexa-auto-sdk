package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Wake word detected messages.
 */
public class WakewordDetectedMessages {
    private static final String TAG = WakewordDetectedMessages.class.getCanonicalName();

    /**
     * Parse WakewordDetected message.
     *
     * @param json Json to parse.
     * @return wakeword if available.
     */
    public static Optional<String> parseWakeword(@NonNull String json) {
        try {
            Log.d(TAG, "parse wake word from aacs message: " + json);
            JSONObject obj = new JSONObject(json);
            String wakeword = obj.get("wakeword").toString();
            return wakeword != null ? Optional.of(wakeword) : Optional.empty();
        } catch (JSONException exception) {
            Log.w(TAG, "Failed to parse wakeword from aacs message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }
}
