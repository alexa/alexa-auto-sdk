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

/**
 * An interface to handle custom earcon. A component can provide custom earcon that will be played.
 */
public interface EarconProvider extends ScopedComponent {
    /**
     * Provide custom start voice earcon
     * @return The resource id of the custom start voice earcon.
     */
    int getAudioCueStartVoice();

    /**
     * Provide custom end of request earcon
     * @return The resource id of the custom end of request earcon.
     */
    int getAudioCueEnd();

    /**
     * Check if custom start voice earcon should be used.
     * @param signal Information for whether custom start voice earcon should be used. For example,
     *               the detected wake word.
     * @return true if custom start voice earcon should be used. Otherwise, return false.
     */
    boolean shouldUseAudioCueStartVoice(String signal);

    /**
     * Check if custom end of request earcon should be used.
     * @param signal Information for whether custom end of request earcon should be used.
     * @return true if custom end of request earcon should be used. Otherwise, return false.
     */
    boolean shouldUseAudioCueEnd(String signal);
}
