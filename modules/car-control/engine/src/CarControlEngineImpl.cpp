/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/CarControl/CarControlEngineImpl.h"

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace carControl {

static const std::string MODE_CONTROLLER_TAG("aace.engine.carControl.ModeController");
static const std::string POWER_CONTROLLER_TAG("aace.engine.carControl.PowerController");
static const std::string RANGE_CONTROLLER_TAG("aace.engine.carControl.RangeController");
static const std::string TOGGLE_CONTROLLER_TAG("aace.engine.carControl.ToggleController");

std::shared_ptr<CarControlEngineImpl> CarControlEngineImpl::create(
    std::shared_ptr<aace::carControl::CarControl> platformInterface) {
    return std::make_shared<CarControlEngineImpl>(platformInterface);
}

CarControlEngineImpl::CarControlEngineImpl(std::shared_ptr<aace::carControl::CarControl> platformInterface) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown("CarControlEngineImpl"),
        m_platformInterface(platformInterface) {
}

bool CarControlEngineImpl::turnPowerControllerOn(const std::string& endpointId) {
    AACE_DEBUG(LX(POWER_CONTROLLER_TAG).sensitive("endpoint", endpointId).sensitive("name", "TurnOn"));
    return m_platformInterface->turnPowerControllerOn(endpointId);
}

bool CarControlEngineImpl::turnPowerControllerOff(const std::string& endpointId) {
    AACE_DEBUG(LX(POWER_CONTROLLER_TAG).sensitive("endpoint", endpointId).sensitive("name", "TurnOff"));
    return m_platformInterface->turnPowerControllerOff(endpointId);
}

bool CarControlEngineImpl::isPowerControllerOn(const std::string& endpointId, bool& isOn) {
    return m_platformInterface->isPowerControllerOn(endpointId, isOn);
}

bool CarControlEngineImpl::turnToggleControllerOn(const std::string& endpointId, const std::string& instance) {
    AACE_DEBUG(LX(TOGGLE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "TurnOn")
                   .sensitive("instance", instance));
    return m_platformInterface->turnToggleControllerOn(endpointId, instance);
}

bool CarControlEngineImpl::turnToggleControllerOff(const std::string& endpointId, const std::string& instance) {
    AACE_DEBUG(LX(TOGGLE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "TurnOff")
                   .sensitive("instance", instance));
    return m_platformInterface->turnToggleControllerOff(endpointId, instance);
}

bool CarControlEngineImpl::isToggleControllerOn(
    const std::string& endpointId,
    const std::string& instance,
    bool& isOn) {
    return m_platformInterface->isToggleControllerOn(endpointId, instance, isOn);
}

bool CarControlEngineImpl::setRangeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    double value) {
    AACE_DEBUG(LX(RANGE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "SetRangeValue")
                   .sensitive("instance", instance)
                   .sensitive("rangeValue", value));
    return m_platformInterface->setRangeControllerValue(endpointId, instance, value);
}

bool CarControlEngineImpl::adjustRangeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    double delta) {
    AACE_DEBUG(LX(RANGE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "AdjustRangeValue")
                   .sensitive("instance", instance)
                   .sensitive("rangeValueDelta", delta));
    return m_platformInterface->adjustRangeControllerValue(endpointId, instance, delta);
}

bool CarControlEngineImpl::getRangeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    double& value) {
    return m_platformInterface->getRangeControllerValue(endpointId, instance, value);
}

bool CarControlEngineImpl::setModeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    const std::string& value) {
    AACE_DEBUG(LX(MODE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "SetMode")
                   .sensitive("instance", instance)
                   .sensitive("mode", value));
    return m_platformInterface->setModeControllerValue(endpointId, instance, value);
}

bool CarControlEngineImpl::adjustModeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    int delta) {
    AACE_DEBUG(LX(MODE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "AdjustMode")
                   .sensitive("instance", instance)
                   .sensitive("modeDelta", delta));
    return m_platformInterface->adjustModeControllerValue(endpointId, instance, delta);
}

bool CarControlEngineImpl::getModeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    std::string& value) {
    return m_platformInterface->getModeControllerValue(endpointId, instance, value);
}

void CarControlEngineImpl::doShutdown() {
    if (m_platformInterface != nullptr) {
        m_platformInterface.reset();
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
