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

#ifndef AACE_ALEXA_TEMPLATE_RUNTIME_H
#define AACE_ALEXA_TEMPLATE_RUNTIME_H

#include "AACE/Core/PlatformInterface.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * TemplateRuntime should be extended to handle directives from the Engine for 
 * rendering visual metadata with display cards. 
 *
 * Metadata is provided as structured JSON and should be parsed by the platform implementation
 * for rendering cards that adhere to design guidelines
 * for the platform device type. For screen-specific design guidance, see the AVS UX Design Overview:
 * https://developer.amazon.com/docs/alexa-voice-service/ux-design-overview.html#displaycards
 */    
class TemplateRuntime : public aace::core::PlatformInterface {
protected:
    TemplateRuntime() = default;

public:
    virtual ~TemplateRuntime();

    /**
     * Provides visual metadata associated with a user request to Alexa.
     * The platform implementation should parse the template metadata and render
     * a display card for the user.
     *
     * For supported templates and rendering guidelines, see https://alexa.design/DevDocRenderTemplate
     *
     * @param [in] payload Renderable template metadata in structured JSON format
     */
    virtual void renderTemplate( const std::string& payload ) = 0;
    
    /**
     * Notifies the platform implementation to dismiss the template display card
     */
    virtual void clearTemplate() = 0;

    /**
     * Provides visual metadata associated with a user request to Alexa for audio playback. 
     * The platform implementation should parse the player info metadata and render
     * a display card for the user.
     *
     * For rendering guidelines, see https://amzn.to/DevDocTemplatePlayerInfo
     *
     * @param [in] payload Renderable player info metadata in structured JSON format
     * @sa PlaybackController
     * @sa AudioPlayer
     */
    virtual void renderPlayerInfo( const std::string& payload ) = 0;
    
    /**
     * Notifies the platform implementation to dismiss the player info display card
     */
    virtual void clearPlayerInfo() = 0;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_TEMPLATE_RUNTIME_H
