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
package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import org.json.JSONObject;

/**
 * An interface that allows components to have access to assistant related information.
 */
public interface AssistantManager extends ScopedComponent {
    /**
     * Provide the current state of the assistant manager.
     * @return The state of the assistant manager.
     */
    String getAssistantsState();

    /**
     * Provide the assistants settings.
     * @return The settings of the assistants.
     */
    JSONObject getAssistantsSettings();

    /**
     * Provide the default assistant for PTT.
     * @return The assistant that is assigned to PTT.
     */
    String getDefaultAssistantForPTT();

    String getCoAssistantPolicy();
}
