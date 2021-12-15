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

#ifndef AACE_LOCATION_LOCATION_PROVIDER_H
#define AACE_LOCATION_LOCATION_PROVIDER_H

#include <string>
#include <chrono>

#include "AACE/Core/PlatformInterface.h"
#include "LocationProviderEngineInterface.h"

#include "Location.h"

/** @file */

namespace aace {
namespace location {

/**
 * LocationProvider should be extended to report geolocation to the Engine.
 * 
 * @deprecated This platform interface is deprecated. 
 *             Use the Alexa Auto Services Bridge (AASB) message broker 
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
class LocationProvider : public aace::core::PlatformInterface {
protected:
    LocationProvider() = default;

public:
    using LocationServiceAccess = aace::location::LocationProviderEngineInterface::LocationServiceAccess;

    virtual ~LocationProvider();

    /**
     * Returns the current geolocation of the device
     *
     * @return The current location
     */
    virtual aace::location::Location getLocation() = 0;

    /**
     * Returns the ISO country code for the current geolocation of the device. If no country can
     * be determined, this method should return an empty string.
     *
     * @return The current country
     */
    virtual std::string getCountry();

    /**
     * Notifies the Engine of a change in location service access. Use this function when the device's
     * access to location service provider changes. E.g., system location access is not granted to the
     * application.
     *
     * @param [in] access Access to the location service
     */
    void locationServiceAccessChanged(LocationServiceAccess access);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<LocationProviderEngineInterface> locationProviderEngineInterface);

private:
    std::shared_ptr<LocationProviderEngineInterface> m_locationProviderEngineInterface;
};

}  // namespace location
}  // namespace aace

#endif  // AACE_LOCATION_LOCATION_PROVIDER_H
