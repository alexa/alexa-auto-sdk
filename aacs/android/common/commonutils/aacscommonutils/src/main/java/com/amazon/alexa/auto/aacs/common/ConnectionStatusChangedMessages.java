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
