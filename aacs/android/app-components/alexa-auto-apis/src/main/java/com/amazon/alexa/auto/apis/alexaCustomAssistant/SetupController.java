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

import io.reactivex.rxjava3.core.Observable;

/**
 * An interface that allow components to interact with setup flow related to Alexa Custom Assistant.
 */

public interface SetupController extends ScopedComponent {
    /**
     * Get the current flow in progress.
     * @return The current flow in progress.
     */
    String getCurrentSetupFlow();

    /**
     * Set the current setup flow to both-assistant, Alexa only or Non-Alexa only flow.
     * @param currentFlow The current flow in progress.
     */
    void setCurrentSetupFlow(String currentFlow);
}
