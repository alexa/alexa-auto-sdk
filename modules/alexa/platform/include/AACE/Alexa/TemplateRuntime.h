/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * The @c TemplateRuntime class should be extended by the platform implementation to handle GUI template data from AVS.
 */
class TemplateRuntime : public aace::core::PlatformInterface {
protected:
    TemplateRuntime() = default;

public:
    virtual ~TemplateRuntime() = default;

    /**
     * Called when the platform implementation should handle a GUI template.
     *
     * @param [in] payload @c JSON data containing the template information to render.
     * @see https://alexa.design/DevDocRenderTemplate
     */
    virtual void renderTemplate( const std::string& payload ) = 0;

    /**
     * Called when the platform implementation should handle audio player renderable content.
     *
     * @param [in] payload @c JSON data containing the player information to render.
     * @see https://amzn.to/DevDocTemplatePlayerInfo
     */
    virtual void renderPlayerInfo( const std::string& payload ) = 0;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_TEMPLATE_RUNTIME_H
