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

#ifndef AACE_ENGINE_LOCATION_LOCATION_SERVICE_OBSERVER_INTERFACE_H
#define AACE_ENGINE_LOCATION_LOCATION_SERVICE_OBSERVER_INTERFACE_H

#include "AACE/Location/LocationProviderEngineInterface.h"

namespace aace {
namespace engine {
namespace location {

class LocationServiceObserverInterface {
public:
    virtual ~LocationServiceObserverInterface() = default;

    using LocationServiceAccess = aace::location::LocationProviderEngineInterface::LocationServiceAccess;

    /**
     * Notifies the observer of a change in location service access.
     */
    virtual void onLocationServiceAccessChanged(LocationServiceAccess access) = 0;
};

}  // namespace location
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOCATION_LOCATION_SERVICE_OBSERVER_INTERFACE_H
