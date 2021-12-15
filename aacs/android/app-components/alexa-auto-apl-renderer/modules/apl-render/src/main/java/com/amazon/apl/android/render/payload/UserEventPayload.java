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

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import lombok.Getter;

/**
 * Represents the payload of User Event.
 */
@Getter
public class UserEventPayload {
    private static final String PRESENTATION_TOKEN = "presentationToken";
    private static final String ARGUMENTS = "arguments";
    private static final String SOURCE = "source";
    private static final String COMPONENTS = "components";

    private final String mPresentationToken;
    private final JSONArray mArguments;
    private final JSONObject mSource;
    private final JSONObject mComponents;

    public static UserEventPayload fromJson(final JSONObject object) throws JSONException {
        return new UserEventPayload(object.getString(PRESENTATION_TOKEN), object.optJSONArray(ARGUMENTS),
                object.optJSONObject(SOURCE), object.optJSONObject(COMPONENTS));
    }

    public UserEventPayload(final String presentationToken, final JSONArray arguments, final JSONObject components,
            final JSONObject source) {
        mPresentationToken = presentationToken;
        mArguments = arguments;
        mComponents = components;
        mSource = source;
    }

    public JSONObject toJson() throws JSONException {
        return new JSONObject()
                .put(PRESENTATION_TOKEN, mPresentationToken)
                .put(ARGUMENTS, mArguments)
                .put(SOURCE, mSource)
                .put(COMPONENTS, mComponents);
    }

    @Override
    public String toString() {
        try {
            return toJson().toString();
        } catch (JSONException e) {
            return "{\"warning\":\"serialization error\"}";
        }
    }
}
