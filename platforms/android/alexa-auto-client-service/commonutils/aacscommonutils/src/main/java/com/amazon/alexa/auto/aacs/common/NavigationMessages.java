package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import com.squareup.moshi.Moshi;

import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Navigation messages with this helper.
 */
public class NavigationMessages {
    private static final String TAG = NavigationMessages.class.getSimpleName();

    /**
     * Parse Navigation StartNavigation directive.
     *
     * @param json Json to parse.
     * @return RenderPlayerInfo if available.
     */
    public static Optional<StartNavigation> parseStartNavigationDirective(@NonNull String json) {
        Moshi moshi = new Moshi.Builder().build();
        StartNavigationJsonAdapter jsonAdapter = new StartNavigationJsonAdapter(moshi);

        try {
            String messagePayloadString = new JSONObject(json).getString("payload");
            StartNavigation startNavigation = jsonAdapter.fromJson(messagePayloadString);
            return startNavigation != null ? Optional.of(startNavigation) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse StartNavigation message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }
}
