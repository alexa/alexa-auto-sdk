/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "Location.h"

/** @file */

namespace aace {
namespace location {

/**
 * LocationProvider should be extended to report geolocation to the Engine.
 */
class LocationProvider : public aace::core::PlatformInterface {
protected:
    LocationProvider() = default;

public:
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
};

}  // namespace location
}  // namespace aace

#endif  // AACE_LOCATION_LOCATION_PROVIDER_H
