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

// aace/location/LocationProvider.java
// This is an automatically generated file.

package com.amazon.aace.location;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c LocationProvider class should be extended by the platform implementation to handle geolocation services.
 */
abstract public class LocationProvider extends PlatformInterface {
    /**
     * Called when the Engine needs the current geolocation.
     *
     * @return @c aace::location::Location of the platform implementation.
     */
    abstract public Location getLocation();
}

// END OF FILE
