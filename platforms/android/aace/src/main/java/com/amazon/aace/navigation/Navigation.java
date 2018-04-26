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

// aace/navigation/Navigation.java
// This is an automatically generated file.

package com.amazon.aace.navigation;

import com.amazon.aace.core.PlatformInterface;

/**
 * PRELIMINARY
 * The @c Navigation class should be extended by the platform implementation to handle navigation directives and events from AVS.
 */
abstract public class Navigation extends PlatformInterface
{
    public Navigation() {
    }

    /**
     * Called when the platform implementation should handle a @c setDestination directive.
     *
     * @param [in] payload @c JSON data containing the destination information.
     * @code    {.json})
     * "destination": {
     *    "coordinate": {
     *        "latitudeInDegrees": {{DOUBLE}},
     *        "longitudeInDegrees": {{DOUBLE}}
     *    },
     *    "name": "{{STRING}}",
     *    "singleLineDisplayAddress": "{{STRING}}"
     *    "multipleLineDisplayAddress": "{{STRING}}",
     * }
     * @endcode
     */
    public boolean setDestination( String payload ) {
        return false;
    }

}

// END OF FILE
