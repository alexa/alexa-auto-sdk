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

import org.json.JSONException;
import org.json.JSONObject;

import lombok.Builder;
import lombok.Getter;

/**
 * Represents a Render Document directive payload. The directive looks like the following:
 * <pre>
 *     {@code
 *  {
 *           "header":{
 *              "namespace":"Alexa.Presentation.APL",
 *               "name":"executeCommands",
 *               "eventId":<STRING>,
 *               "messageId":<STRING>
 *           },
 *           "payload":{
 *               "presentationToken": STRING, //eg.
 * amzn1.as-tt.v1.ThirdPartySdkSpeechlet#TID#a974fcff-963f-4585-8d6e-5597b2eaadab "commands": <ARRAY>
 *           }
 *   }
 * }
 * </pre>
 */
@Getter
public class ExecuteCommandPayload extends APLPayload {
    /**
     * Commands in the payload.
     */
    private final String mCommands;
    public static final String FIELD_COMMANDS = "commands";

    @Builder
    public ExecuteCommandPayload(final String presentationToken, final String commands) {
        super(presentationToken);
        mCommands = commands;
    }

    public JSONObject toJson() throws JSONException {
        return super.toJson().put(FIELD_COMMANDS, mCommands);
    }

    /**
     * TODO move this into {@link ExecuteCommandPayload} and add tests
     */
    public static ExecuteCommandPayload convertToExecuteCommand(final String payload) throws JSONException {
        JSONObject jsonPayload = new JSONObject(payload);
        return ExecuteCommandPayload.builder()
                .presentationToken(jsonPayload.optString("presentationToken", ""))
                .commands(jsonPayload.optString("commands", ""))
                .build();
    }
}
