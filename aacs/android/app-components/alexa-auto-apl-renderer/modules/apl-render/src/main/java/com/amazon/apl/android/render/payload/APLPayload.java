/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.payload;

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * Base class for an APL payload.
 */
@AllArgsConstructor
@Getter
public abstract class APLPayload {
    private static final String TAG = APLPayload.class.getSimpleName();

    public static final String FIELD_PRESENTATION_TOKEN = "presentationToken";

    /**
     * Identifier for the payload.
     */
    private final String mPresentationToken;

    protected JSONObject toJson() throws JSONException {
        return new JSONObject().put(FIELD_PRESENTATION_TOKEN, mPresentationToken);
    }

    public String toString() {
        try {
            return toJson().toString();
        } catch (JSONException e) {
            Log.wtf(TAG, e);
            return "";
        }
    }
}
