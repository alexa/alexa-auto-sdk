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
package com.amazon.alexa.auto.apis.apl;

import android.content.Context;
import android.view.View;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface for APL visual handling logics.
 */
public interface APLVisualController extends ScopedComponent {
    /**
     * Initialize APL presenter with APL characteristic configs.
     *
     * @param context Android Activity context
     * @param configs APL characteristic configs
     */
    void initializeAPLPresenter(Context context, String configs);

    /**
     * Set APL layout when view is created.
     *
     * @param view APL view
     */
    void setAPLLayout(View view);

    /**
     * Render APL visual card with document.
     *
     * @param jsonPayload document payload
     * @param token APL token
     * @param windowId APL window ID
     */
    void renderDocument(String jsonPayload, String token, String windowId);

    /**
     * Clear APL visual card.
     *
     * @param token APL token
     */
    void clearDocument(String token);

    /**
     * Execute APL command.
     *
     * @param payload execution payload
     * @param token APL token
     */
    void executeCommands(String payload, String token);

    /**
     * Handle APL runtime properties update.
     *
     * @param aplRuntimeProperties APL runtime properties
     */
    void handleAPLRuntimeProperties(String aplRuntimeProperties);

    /**
     * Send APL data source update.
     *
     * @param dataType data type
     * @param payload data source payload
     * @param token APL token
     */
    void handleAPLDataSourceUpdate(String dataType, String payload, String token);

    /**
     * Interrupt APL commands sequence.
     *
     * @param token APL token
     */
    void interruptCommandSequence(String token);

    /**
     * Interrupt APL execution.
     */
    void cancelExecution();
}
