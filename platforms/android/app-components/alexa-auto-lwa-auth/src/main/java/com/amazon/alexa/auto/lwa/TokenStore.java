package com.amazon.alexa.auto.lwa;

import android.content.Context;
import android.content.SharedPreferences;

import androidx.annotation.NonNull;

import java.util.Optional;

/**
 * Store for LWA Auth Refresh Token.
 */
public class TokenStore {
    private static final String REFRESH_TOKEN_KEY = "com.amazon.alexa.lwa.key";

    /**
     * Fetch refresh token if available.
     *
     * @param context Android Context.
     * @return Refresh token if available, null otherwise.
     */
    static Optional<String> getRefreshToken(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(REFRESH_TOKEN_KEY, Context.MODE_PRIVATE);
        String refreshToken = preferences.getString(REFRESH_TOKEN_KEY, null);

        return refreshToken != null ? Optional.of(refreshToken) : Optional.empty();
    }

    static void saveRefreshToken(@NonNull Context context, @NonNull String refreshToken) {
        SharedPreferences preferences = context.getSharedPreferences(REFRESH_TOKEN_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(REFRESH_TOKEN_KEY, refreshToken);
        editor.apply();
    }

    static void resetRefreshToken(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(REFRESH_TOKEN_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.remove(REFRESH_TOKEN_KEY);
        editor.apply();
    }
}
