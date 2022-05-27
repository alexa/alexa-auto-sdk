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
package com.amazon.alexa.auto.aacs.common;

import android.util.Log;

import androidx.annotation.NonNull;

import org.json.JSONObject;

import java.util.Optional;

/**
 * Parse Auth Status Changed messages.
 */
public class AuthStateChangedMessages {
    private static final String TAG = AuthStateChangedMessages.class.getSimpleName();

    /**
     * Parse AuthStatusChanged message.
     *
     * @param json Json to parse.
     * @return auth status if available.
     */
    public static Optional<String> parseAuthState(@NonNull String json) {
        try {
            JSONObject obj = new JSONObject(json);
            String authstate = obj.get("state").toString();
            return authstate != null ? Optional.of(authstate) : Optional.empty();
        } catch (Exception exception) {
            Log.w(TAG, "Failed to parse auth state from aacs message: " + json + " error: " + exception);
            return Optional.empty();
        }
    }
}
