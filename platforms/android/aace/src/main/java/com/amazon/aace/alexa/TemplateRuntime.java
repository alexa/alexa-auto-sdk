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

// aace/alexa/TemplateRuntime.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c TemplateRuntime class should be extended by the platform implementation to handle GUI template data from AVS.
 */
abstract public class TemplateRuntime extends PlatformInterface
{
    public TemplateRuntime() {
    }

    /**
     * Called when the platform implementation should handle a GUI template.
     *
     * @param [in] payload @c JSON data containing the template information to render.
     * @see https://alexa.design/DevDocRenderTemplate
     */
    public void renderTemplate( String payload ) {
    }

    /**
     * Called when the platform implementation should handle audio player renderable content.
     *
     * @param [in] payload @c JSON data containing the player information to render.
     * @see https://amzn.to/DevDocTemplatePlayerInfo
     */
    public void renderPlayerInfo( String payload ) {
    }

}

// END OF FILE
