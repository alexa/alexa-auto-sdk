/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <tuple>
#include <unordered_map>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"
#include "AACE/Engine/PropertyManager/PropertyManagerEngineImpl.h"

namespace aace {
namespace engine {
namespace propertyManager {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.PropertyManagerEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "PropertyManagerEngineImpl";

/// Counter metrics for PropertyManager Platform APIs
static const std::string METRIC_PROPERTY_MANAGER_SET_PROPERTY = "setProperty";
static const std::string METRIC_PROPERTY_MANAGER_PROPERTY_STATE_CHANGED = "propertyStateChanged";
static const std::string METRIC_PROPERTY_MANAGER_GET_PROPERTY = "getProperty";
static const std::string METRIC_PROPERTY_MANAGER_PROPERTY_CHANGED = "propertyChanged";

PropertyManagerEngineImpl::PropertyManagerEngineImpl(
    std::shared_ptr<aace::propertyManager::PropertyManager> platformPropertyManagerInterface) :
        m_platformPropertyManagerInterface(platformPropertyManagerInterface) {
}

bool PropertyManagerEngineImpl::initialize(
    std::shared_ptr<PropertyManagerServiceInterface> propertyManagerServiceInterface) {
    try {
        m_propertyManagerServiceInterface = propertyManagerServiceInterface;
        return true;

    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<PropertyManagerEngineImpl> PropertyManagerEngineImpl::create(
    std::shared_ptr<aace::propertyManager::PropertyManager> platformPropertyManagerInterface,
    std::shared_ptr<PropertyManagerServiceInterface> propertyManagerServiceInterface) {
    try {
        ThrowIfNull(platformPropertyManagerInterface, "nullPropertyManagerPlatformInterface");
        ThrowIfNull(propertyManagerServiceInterface, "nullPropertyManagerServiceInterface");
        auto propertyManagerEngineImpl =
            std::shared_ptr<PropertyManagerEngineImpl>(new PropertyManagerEngineImpl(platformPropertyManagerInterface));

        ThrowIfNot(
            propertyManagerEngineImpl->initialize(propertyManagerServiceInterface),
            "initializePropertyManagerEngineImplFailed");

        // Set the Engine Interface reference
        platformPropertyManagerInterface->setEngineInterface(propertyManagerEngineImpl);

        return propertyManagerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool PropertyManagerEngineImpl::onSetProperty(const std::string& name, const std::string& value) {
    try {
        emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onSetProperty", {METRIC_PROPERTY_MANAGER_SET_PROPERTY, name});
        auto m_propertyManagerServiceInterface_lock = m_propertyManagerServiceInterface.lock();
        ThrowIfNull(m_propertyManagerServiceInterface_lock, "invalidPropertyManagerServiceInterfaceInstance");
        ThrowIfNot(m_propertyManagerServiceInterface_lock->setProperty(name, value, true), "setPropertyFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string PropertyManagerEngineImpl::onGetProperty(const std::string& name) {
    try {
        emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onGetProperty", {METRIC_PROPERTY_MANAGER_GET_PROPERTY, name});
        auto m_propertyManagerServiceInterface_lock = m_propertyManagerServiceInterface.lock();
        ThrowIfNull(m_propertyManagerServiceInterface_lock, "invalidPropertyManagerServiceInterfaceInstance");
        return m_propertyManagerServiceInterface_lock->getProperty(name);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::string();
    }
}

void PropertyManagerEngineImpl::handlePropertyChanged(const std::string& name, const std::string& value) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "handlePropertyChanged", {METRIC_PROPERTY_MANAGER_PROPERTY_CHANGED, name});
    if (m_platformPropertyManagerInterface != nullptr) {
        m_platformPropertyManagerInterface->propertyChanged(name, value);
    }
}

void PropertyManagerEngineImpl::propertyStateChanged(
    const std::string& name,
    const std::string& value,
    const aace::propertyManager::PropertyManager::PropertyState state) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "propertyStateChanged", {METRIC_PROPERTY_MANAGER_PROPERTY_STATE_CHANGED, name});
    if (m_platformPropertyManagerInterface != nullptr) {
        m_platformPropertyManagerInterface->propertyStateChanged(name, value, state);
    }
}

void PropertyManagerEngineImpl::shutdown() {
    if (m_platformPropertyManagerInterface != nullptr) {
        m_platformPropertyManagerInterface->setEngineInterface(nullptr);
        m_platformPropertyManagerInterface.reset();
    }
}

}  // namespace propertyManager
}  // namespace engine
}  // namespace aace
