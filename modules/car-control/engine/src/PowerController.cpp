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

#include "AACE/Engine/CarControl/PowerController.h"

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.carControl.PowerController");

/// Alias for readability
/// @{
using AlexaResponseType = alexaClientSDK::avsCommon::avs::AlexaResponseType;
using AlexaStateChangeCauseType = alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType;
/// @}

std::shared_ptr<PowerController> PowerController::create(const std::string& endpointId, const std::string& interface) {
    try {
        return std::shared_ptr<PowerController>(new PowerController(endpointId, interface));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

PowerController::PowerController(const std::string& endpointId, const std::string& interface) :
        CapabilityController(endpointId, interface) {
}

void PowerController::build(
    std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
    std::unique_ptr<EndpointBuilder>& builder) {
    m_carControlServiceInterface = carControlServiceInterface;
    builder->withPowerController(shared_from_this(), false, false);
}

std::pair<AlexaResponseType, std::string> PowerController::setPowerState(
    const bool state,
    const AlexaStateChangeCauseType cause) {
    try {
        AACE_DEBUG(LX(TAG)
                       .d("cause", alexaClientSDK::avsCommon::sdkInterfaces::alexaStateChangeCauseTypeToString(cause))
                       .sensitive("endpointId", getEndpointId())
                       .sensitive("state", state));
        if (state) {
            ThrowIfNot(
                m_carControlServiceInterface->turnPowerControllerOn(getEndpointId()), "turnPowerControllerOnFailed");
        } else {
            ThrowIfNot(
                m_carControlServiceInterface->turnPowerControllerOff(getEndpointId()), "turnPowerControllerOffFailed");
        }
        return std::make_pair(AlexaResponseType::SUCCESS, "");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(AlexaResponseType::INTERNAL_ERROR, "Something went wrong");
    }
}

std::pair<
    alexaClientSDK::avsCommon::avs::AlexaResponseType,
    alexaClientSDK::avsCommon::utils::Optional<PowerController::PowerState>>
PowerController::getPowerState() {
    bool state;
    if (m_carControlServiceInterface->isPowerControllerOn(getEndpointId(), state)) {
        return std::make_pair(
            AlexaResponseType::SUCCESS,
            alexaClientSDK::avsCommon::utils::Optional<PowerState>(PowerState{
                state, alexaClientSDK::avsCommon::utils::timing::TimePoint::now(), std::chrono::milliseconds(0)}));
    } else {
        return std::make_pair(
            AlexaResponseType::INTERNAL_ERROR, alexaClientSDK::avsCommon::utils::Optional<PowerState>());
    }
}

bool PowerController::addObserver(std::shared_ptr<PowerControllerObserverInterface> observer) {
    AACE_WARN(LX(TAG).m("Unexpected addObserver()"));
    return false;
}

void PowerController::removeObserver(const std::shared_ptr<PowerControllerObserverInterface>& observer) {
    AACE_WARN(LX(TAG).m("Unexpected removeObserver()"));
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
