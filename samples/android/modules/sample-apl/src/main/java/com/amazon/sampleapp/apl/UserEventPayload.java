/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.apl;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Map;

/**
 * Represents the payload of User Event.
 * https://developer.amazon.com/docs/alexa-presentation-language/apl-standard-commands.html#userevent
 */
public class UserEventPayload {
    private static final String PRESENTATION_TOKEN = "presentationToken";
    private static final String ARGUMENTS = "arguments";
    private static final String SOURCE = "source";
    private static final String COMPONENTS = "components";
    private static final String TYPE = "type";
    private static final String TYPE_VALUE = "Alexa.Presentation.APL.UserEvent";

    private final String mToken;
    private final JSONArray mArguments;
    private final JSONObject mSource;
    private final JSONObject mComponents;
    private static int requestId = 1;

    public UserEventPayload(final String token, final Object[] args, final Map<String, Object> components,
            final Map<String, Object> sources) throws Exception {
        mToken = token;
        mArguments = new JSONArray((args));
        mComponents = new JSONObject(components);
        mSource = new JSONObject(sources);
    }

    public JSONObject toJson() throws JSONException {
        return new JSONObject()
                .put(TYPE, TYPE_VALUE)
                .put(PRESENTATION_TOKEN, mToken)
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
