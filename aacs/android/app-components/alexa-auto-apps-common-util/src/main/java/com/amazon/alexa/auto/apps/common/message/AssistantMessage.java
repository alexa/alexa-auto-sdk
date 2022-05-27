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
package com.amazon.alexa.auto.apps.common.message;

import androidx.annotation.Nullable;

/**
 * Assistant Message
 */
public class AssistantMessage {
    private String assistantName;
    private String action;
    private String payload;

    public AssistantMessage(String assistantName, String action, @Nullable String payload) {
        this.assistantName = assistantName;
        this.action = action;
        this.payload = payload;
    }

    public String getAssistantName() {
        return this.assistantName;
    }

    public String getAction() {
        return this.action;
    }

    public String getPayload() {
        return this.payload;
    }
}
