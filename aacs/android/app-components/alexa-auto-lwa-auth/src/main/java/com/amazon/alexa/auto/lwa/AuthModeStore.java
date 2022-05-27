/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.lwa;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.auth.AuthMode;

/**
 * Store for AuthMode in Shared Preferences.
 */
public class AuthModeStore {
    private static final String AUTH_MODE_KEY = "com.amazon.alexa.auth.authMode.key";

    private static final String TAG = AuthReceiver.class.getSimpleName();

    private static AuthMode getAuthModeEnum(String authModeString) {
        AuthMode mode = AuthMode.CBL_AUTHORIZATION;

        try {
            if (authModeString != null)
                mode = AuthMode.valueOf(authModeString);

        } catch (Exception e) {
            Log.e(TAG, "Failed to convert authmode string  to enum" + e);
        }
        return mode;
    }

    /**
     * Persist AuthMode that is selected by user in either setup or settings screen.
     *
     * @param context	  Android context
     * @param AuthMode  authmode enum
     */
    public static void persistAuthMode(@NonNull Context context, AuthMode authMode) {
        SharedPreferences preferences = context.getSharedPreferences(AUTH_MODE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();

        editor.putString(AUTH_MODE_KEY, authMode.name());
        editor.apply();
    }

    /**
     * Get AuthMode that is selected by user in either setup or settings screen.
     *
     * @param context Android context
     * @return AuthMode authmode enum
     */
    public static AuthMode getPersistentAuthMode(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AUTH_MODE_KEY, Context.MODE_PRIVATE);

        String authModeString = preferences.getString(AUTH_MODE_KEY, null);
        return getAuthModeEnum(authModeString);
    }

    /**
     * Remove the persistent AuthMode
     * @param context Android context
     */
    public static void resetPersistentAuthMode(@NonNull Context context) {
        SharedPreferences preferences = context.getSharedPreferences(AUTH_MODE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.remove(AUTH_MODE_KEY);
        editor.apply();
    }
}
