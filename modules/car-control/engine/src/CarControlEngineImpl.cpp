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
#include <AACE/Engine/Utils/Metrics/Metrics.h>

namespace aace {
namespace engine {
namespace carControl {

using namespace aace::engine::utils::metrics;

static const std::string MODE_CONTROLLER_TAG("aace.engine.carControl.ModeController");
static const std::string POWER_CONTROLLER_TAG("aace.engine.carControl.PowerController");
static const std::string RANGE_CONTROLLER_TAG("aace.engine.carControl.RangeController");
static const std::string TOGGLE_CONTROLLER_TAG("aace.engine.carControl.ToggleController");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "CarControlEngineImpl";

/// Counter metric names for CarControl Platform APIs
static const std::string METRIC_CAR_CONTROL_TURN_POWERCONTROLLER_ON = "TurnPowerControllerOn";
static const std::string METRIC_CAR_CONTROL_TURN_POWERCONTROLLER_OFF = "TurnPowerControllerOff";
static const std::string METRIC_CAR_CONTROL_IS_POWERCONTROLLER_ON = "IsPowerControllerOn";
static const std::string METRIC_CAR_CONTROL_TURN_TOGGLECONTROLLER_ON = "TurnToggleControllerOn";
static const std::string METRIC_CAR_CONTROL_TURN_TOGGLECONTROLLER_OFF = "TurnToggleControllerOff";
static const std::string METRIC_CAR_CONTROL_IS_TOGGLECONTROLLER_ON = "IsToggleControllerOn";
static const std::string METRIC_CAR_CONTROL_SET_RANGECONTROLLER_VALUE = "SetRangeControllerValue";
static const std::string METRIC_CAR_CONTROL_ADJUST_RANGECONTROLLER_VALUE = "AdjustRangeControllerValue";
static const std::string METRIC_CAR_CONTROL_GET_RANGECONTROLLER_VALUE = "GetRangeControllerValue";
static const std::string METRIC_CAR_CONTROL_SET_MODECONTROLLER_VALUE = "SetModeControllerValue";
static const std::string METRIC_CAR_CONTROL_ADJUST_MODECONTROLLER_VALUE = "AdjustModeControllerValue";
static const std::string METRIC_CAR_CONTROL_GET_MODECONTROLLER_VALUE = "GetModeControllerValue";

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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "turnPowerControllerOn", METRIC_CAR_CONTROL_TURN_POWERCONTROLLER_ON, 1);
    return m_platformInterface->turnPowerControllerOn(endpointId);
}

bool CarControlEngineImpl::turnPowerControllerOff(const std::string& endpointId) {
    AACE_DEBUG(LX(POWER_CONTROLLER_TAG).sensitive("endpoint", endpointId).sensitive("name", "TurnOff"));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "turnPowerControllerOff", METRIC_CAR_CONTROL_TURN_POWERCONTROLLER_OFF, 1);
    return m_platformInterface->turnPowerControllerOff(endpointId);
}

bool CarControlEngineImpl::isPowerControllerOn(const std::string& endpointId, bool& isOn) {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "isPowerControllerOn", METRIC_CAR_CONTROL_IS_POWERCONTROLLER_ON, 1);
    return m_platformInterface->isPowerControllerOn(endpointId, isOn);
}

bool CarControlEngineImpl::turnToggleControllerOn(const std::string& endpointId, const std::string& instance) {
    AACE_DEBUG(LX(TOGGLE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "TurnOn")
                   .sensitive("instance", instance));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "turnToggleControllerOn", METRIC_CAR_CONTROL_TURN_TOGGLECONTROLLER_ON, 1);
    return m_platformInterface->turnToggleControllerOn(endpointId, instance);
}

bool CarControlEngineImpl::turnToggleControllerOff(const std::string& endpointId, const std::string& instance) {
    AACE_DEBUG(LX(TOGGLE_CONTROLLER_TAG)
                   .sensitive("endpoint", endpointId)
                   .sensitive("name", "TurnOff")
                   .sensitive("instance", instance));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "turnToggleControllerOff", METRIC_CAR_CONTROL_TURN_TOGGLECONTROLLER_OFF, 1);
    return m_platformInterface->turnToggleControllerOff(endpointId, instance);
}

bool CarControlEngineImpl::isToggleControllerOn(
    const std::string& endpointId,
    const std::string& instance,
    bool& isOn) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "isToggleControllerOn", METRIC_CAR_CONTROL_IS_TOGGLECONTROLLER_ON, 1);
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "setRangeControllerValue", METRIC_CAR_CONTROL_SET_RANGECONTROLLER_VALUE, 1);
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "adjustRangeControllerValue", METRIC_CAR_CONTROL_ADJUST_RANGECONTROLLER_VALUE, 1);
    return m_platformInterface->adjustRangeControllerValue(endpointId, instance, delta);
}

bool CarControlEngineImpl::getRangeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    double& value) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "getRangeControllerValue", METRIC_CAR_CONTROL_GET_RANGECONTROLLER_VALUE, 1);
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "setModeControllerValue", METRIC_CAR_CONTROL_SET_MODECONTROLLER_VALUE, 1);
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
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "adjustModeControllerValue", METRIC_CAR_CONTROL_ADJUST_MODECONTROLLER_VALUE, 1);
    return m_platformInterface->adjustModeControllerValue(endpointId, instance, delta);
}

bool CarControlEngineImpl::getModeControllerValue(
    const std::string& endpointId,
    const std::string& instance,
    std::string& value) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "getModeControllerValue", METRIC_CAR_CONTROL_GET_MODECONTROLLER_VALUE, 1);
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
