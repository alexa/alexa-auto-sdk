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

#ifndef AACE_ENGINE_LOCATION_LOCATION_PROVIDER_ENGINE_IMPL_H
#define AACE_ENGINE_LOCATION_LOCATION_PROVIDER_ENGINE_IMPL_H

#include <unordered_set>
#include <mutex>
#include <memory>

#include "AACE/Engine/Location/LocationServiceInterface.h"
#include "AACE/Location/LocationProviderEngineInterface.h"

#include "LocationServiceObserverInterface.h"

namespace aace {
namespace engine {
namespace location {

class LocationProviderEngineImpl
        : public aace::location::LocationProviderEngineInterface
        , public LocationServiceInterface {
private:
    LocationProviderEngineImpl(std::shared_ptr<aace::location::LocationProvider> platfromInterface);

public:
    static std::shared_ptr<LocationProviderEngineImpl> create(
        std::shared_ptr<aace::location::LocationProvider> platformInterface);
    virtual ~LocationProviderEngineImpl() = default;

    // aace::engine::location::LocationServiceInterface
    void addObserver(std::shared_ptr<LocationServiceObserverInterface> observer) override;
    void removeObserver(std::shared_ptr<LocationServiceObserverInterface> observer) override;
    aace::location::Location getLocation() override;

    // LocationProviderEngineInterface
    virtual void onLocationServiceAccessChanged(LocationServiceAccess access) override;

    void shutdown();

private:
    std::unordered_set<std::shared_ptr<LocationServiceObserverInterface>> m_observers;
    std::shared_ptr<aace::location::LocationProvider> m_locationProviderPlatformInterface;
    std::mutex m_mutex;
};

}  // namespace location
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOCATION_LOCATION_PROVIDER_ENGINE_IMPL_H
