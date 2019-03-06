/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * TemplateRuntime should be extended to handle directives from the Engine for
 * rendering visual metadata with display cards.
 * Metadata is provided as structured JSON and should be parsed by the platform implementation
 * for rendering cards that adhere to design guidelines
 * for the platform device type. For screen-specific design guidance, see the AVS UX Design Overview:
 * https://developer.amazon.com/docs/alexa-voice-service/ux-design-overview.html#displaycards
 */
abstract public class TemplateRuntime extends PlatformInterface
{
    public TemplateRuntime() {
    }

    /**
     * Provides visual metadata associated with a user request to Alexa.
     * The platform implementation should parse the template metadata and render
     * a display card for the user.
     * For supported templates and rendering guidelines, see https://alexa.design/DevDocRenderTemplate
     *
     * @param  payload Renderable template metadata in structured JSON format
     */
    public void renderTemplate( String payload ) {}

    /**
     * Provides visual metadata associated with a user request to Alexa for audio playback.
     * The platform implementation should parse the player info metadata and render
     * a display card for the user.
     * For rendering guidelines, see https://amzn.to/DevDocTemplatePlayerInfo
     *
     * @param  payload Renderable player info metadata in structured JSON format
     *
     * @sa PlaybackController
     *
     * @sa AudioPlayer
     */
    public void renderPlayerInfo( String payload ) {}

    /**
     * Notifies the platform implementation to dismiss the template display card
     */
    public void clearTemplate() {}

    /**
     * Notifies the platform implementation to dismiss the player info display card
     */
    public void clearPlayerInfo() {}

}

// END OF FILE
