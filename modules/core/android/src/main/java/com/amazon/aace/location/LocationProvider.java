/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.location;

import com.amazon.aace.core.PlatformInterface;

/**
 * LocationProvider should be extended to report geolocation to the Engine.
 *
 * @deprecated This platform interface is deprecated.
 *             Use the Alexa Auto Services Bridge (AASB) message broker
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
abstract public class LocationProvider extends PlatformInterface {
    /**
     * Describes the access to the geolocation service on the device.
     */
    public enum LocationServiceAccess {
        /**
         * The location service on the device is disabled (e.g., GPS is turned off).
         * @hideinitializer
         */
        DISABLED("DISABLED"),
        /**
         * The location service on the device is enabled (e.g., GPS is turned on).
         * @hideinitializer
         */
        ENABLED("ENABLED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private LocationServiceAccess(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }
    ;

    /**
     * Returns the current geolocation of the device
     *
     * @return The current location
     */
    abstract public Location getLocation();

    /**
     * Returns the ISO country code for the current geolocation of the device. If no country can
     * be determined, this method should return an empty string.
     *
     * @return The current country
     */
    public String getCountry() {
        return "";
    }

    /**
     * Notifies the Engine of a change in location service access. Use this method when the device's
     * access to location service provider changes. E.g., system location access is not granted to the
     * application.
     *
     * @param access Access to the location service
     */
    public void locationServiceAccessChanged(LocationServiceAccess access) {
        locationServiceAccessChanged(getNativeRef(), access);
    }

    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void locationServiceAccessChanged(long nativeRef, LocationServiceAccess access);
}
