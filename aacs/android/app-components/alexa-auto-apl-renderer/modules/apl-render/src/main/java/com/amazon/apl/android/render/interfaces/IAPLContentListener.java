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

package com.amazon.apl.android.render.interfaces;

import androidx.annotation.NonNull;

/**
 * Interface for proving APL directive content such render document and
 * execute commands payload to the runtime.
 */
public interface IAPLContentListener {
    /**
     * Notifies that a @c RenderDocument directive has been received. Once called, the
     * client should render the document based on the APL specification in the payload in structured JSON format.
     *
     * @note The payload may contain customer sensitive information and should be used with utmost care.
     * Failure to do so may result in exposing or mishandling of customer data.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.RenderDocument directive which follows the APL
     *         specification.
     * @param [in] token The APL presentation token associated with the document in the payload.
     * @param [in] windowId The target windowId.
     */
    public void onRenderDocument(String jsonPayload, String token, String windowId);

    /**
     * Notifies when the client should clear the APL display card.
     * Once the card is cleared, the platform implementation should call clearCard().
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void onClearDocument(String token);

    /**
     * Notifies that an ExecuteCommands directive has been received.
     *
     * @param [in] jsonPayload The payload of the Alexa.Presentation.APL.ExecuteCommands directive in structured JSON
     *         format.
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void onExecuteCommands(String jsonPayload, String token);

    /**
     * Notifies of a dynamic data source update. Please refer to
     * APL documentation for more information.
     *
     * https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-data-source.html
     *
     * @param [in] sourceType DataSource type.
     * @param [in] jsonPayload The payload of the directive in structured JSON format.
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void onDataSourceUpdate(String sourceType, String jsonPayload, String token);

    /**
     * Notifies that a command execution sequence should be interrupted.
     *
     * @param [in] token The APL presentation token associated with the current rendered document.
     */
    public void onInterruptCommandSequence(String token);

    /**
     * Notifies that APL runtime properties should be changed on the rendered document.
     * @param properties JSON string containing one or more properties.
     * @code{.json}
     * {
     *      "drivingState" : "parked|moving",
     *      "theme" : "light|light-gray1|light-gray2|dark|dark-black|dark-gray",
     *      "video" : "enabled|disabled"
     * }
     * @endcode
     */
    public void onAPLRuntimeProperties(String properties);
}
