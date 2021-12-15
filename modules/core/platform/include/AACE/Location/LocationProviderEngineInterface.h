/*
* Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_LOCATION_LOCATION_PROVIDER_ENGINE_INTERFACE_H
#define AACE_LOCATION_LOCATION_PROVIDER_ENGINE_INTERFACE_H

/** @file */

#include <iostream>

namespace aace {
namespace location {

/**
 * LocationProviderEngineInterface
 */
class LocationProviderEngineInterface {
public:
    virtual ~LocationProviderEngineInterface() = default;

    /**
     * Describes the access to the geolocation service on the device.
     */
    enum class LocationServiceAccess {

        /**
         * The location service on the device is disabled (e.g., GPS is turned off).
         */
        DISABLED,
        /**
         * The location service on the device is enabled (e.g., GPS is turned on).
         */
        ENABLED
    };

    virtual void onLocationServiceAccessChanged(LocationServiceAccess access) = 0;
};

inline std::ostream& operator<<(
    std::ostream& stream,
    const LocationProviderEngineInterface::LocationServiceAccess& access) {
    switch (access) {
        case LocationProviderEngineInterface::LocationServiceAccess::DISABLED:
            stream << "DISABLED";
            break;
        case LocationProviderEngineInterface::LocationServiceAccess::ENABLED:
            stream << "ENABLED";
            break;
    }
    return stream;
}

}  // namespace location
}  // namespace aace

#endif  //AACE_LOCATION_LOCATION_PROVIDER_ENGINE_INTERFACE_H
