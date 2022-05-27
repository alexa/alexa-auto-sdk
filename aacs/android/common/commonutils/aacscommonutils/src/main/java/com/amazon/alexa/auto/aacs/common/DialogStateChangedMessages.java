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
