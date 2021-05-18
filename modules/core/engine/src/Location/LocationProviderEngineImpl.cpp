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

#include "AACE/Engine/Location/LocationProviderEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace location {

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.LocationProviderEngineImpl");

std::shared_ptr<LocationProviderEngineImpl> LocationProviderEngineImpl::create(
    std::shared_ptr<aace::location::LocationProvider> platformInterface) {
    try {
        ThrowIfNull(platformInterface, "locationProviderPlatformInterfaceIsNull");
        auto locationProviderEngineImpl =
            std::shared_ptr<LocationProviderEngineImpl>(new LocationProviderEngineImpl(platformInterface));
        return locationProviderEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

LocationProviderEngineImpl::LocationProviderEngineImpl(
    std::shared_ptr<aace::location::LocationProvider> platformInterface) :
        m_locationProviderPlatformInterface(platformInterface) {
}

void LocationProviderEngineImpl::addObserver(std::shared_ptr<LocationServiceObserverInterface> observer) {
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_observers.insert(observer);
    } else {
        AACE_ERROR(LX(TAG).d("reason", "ObserverIsNull"));
    }
}

void LocationProviderEngineImpl::removeObserver(std::shared_ptr<LocationServiceObserverInterface> observer) {
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_observers.erase(observer);
    } else {
        AACE_ERROR(LX(TAG).d("reason", "ObserverIsNull"));
    }
}

void LocationProviderEngineImpl::onLocationServiceAccessChanged(LocationServiceAccess access) {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& next : m_observers) {
        next->onLocationServiceAccessChanged(access);
    }
}

aace::location::Location LocationProviderEngineImpl::getLocation() {
    if (m_locationProviderPlatformInterface != nullptr) {
        return m_locationProviderPlatformInterface->getLocation();
    } else {
        AACE_WARN(LX(TAG).m("locationQueriedAfterShutdown"));
        return aace::location::Location();
    }
}

void LocationProviderEngineImpl::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.clear();
    m_locationProviderPlatformInterface.reset();
}

}  // namespace location
}  // namespace engine
}  // namespace aace
