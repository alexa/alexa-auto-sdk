/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <nlohmann/json.hpp>

#include <AVSCommon/AVS/CapabilitySemantics/CapabilitySemantics.h>
#include <ModeController/ModeControllerAttributeBuilder.h>

#include "AACE/Engine/CarControl/ModeController.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.carControl.ModeController");

/// Alias for readability
/// @{
using ModeControllerConfiguration =
    alexaClientSDK::avsCommon::sdkInterfaces::modeController::ModeControllerInterface::ModeControllerConfiguration;
/// @}

std::shared_ptr<ModeController> ModeController::create(
    const json& controllerConfig,
    const std::string& endpointId,
    const std::string& interface,
    const AssetStore& assetStore) {
    try {
        std::string instance = controllerConfig.at("instance");
        ThrowIf(instance.empty(), "missingInstance");

        ThrowIfNot(controllerConfig.contains("capabilityResources"), "missingCapabilityResources");
        alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::avs::CapabilityResources>
            capabilityResources = getResources(controllerConfig.at("capabilityResources"), assetStore);
        ThrowIfNot(capabilityResources.hasValue(), "failedToParseCapabilityResourcesConfig");
        auto attributeBuilder =
            alexaClientSDK::capabilityAgents::modeController::ModeControllerAttributeBuilder::create();
        attributeBuilder->withCapabilityResources(capabilityResources.value());

        auto& configuration = controllerConfig.at("configuration");
        if (configuration.contains("ordered")) {
            bool ordered = configuration.at("ordered");
            attributeBuilder->setOrdered(ordered);
        }

        auto& supportedModes = configuration.at("supportedModes");
        for (auto& supportedMode : supportedModes.items()) {
            std::string value = supportedMode.value().at("value");
            ThrowIfNot(supportedMode.value().contains("modeResources"), "missingModeResources");
            // Note: ModeResources is a type alias for CapabilityResources
            alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::avs::CapabilityResources>
                modeResources = getResources(supportedMode.value().at("modeResources"), assetStore);
            ThrowIfNot(modeResources.hasValue(), "failedToParseModeResourcesConfig");
            attributeBuilder->addMode(value, modeResources.value());
        }

        if (controllerConfig.contains("semantics")) {
            auto& semanticsJson = controllerConfig.at("semantics");
            alexaClientSDK::avsCommon::utils::Optional<
                alexaClientSDK::avsCommon::avs::capabilitySemantics::CapabilitySemantics>
                semantics = getSemantics(semanticsJson);
            ThrowIfNot(semantics.hasValue(), "failedToParseSemanticsConfig");
            attributeBuilder->withSemantics(semantics.value());
        }

        auto attributes = attributeBuilder->build();
        ThrowIfNot(attributes.hasValue(), "invalidAttributes");
        ThrowIf(attributes.value().modes.empty(), "emptyModes");
        auto controller =
            std::shared_ptr<ModeController>(new ModeController(endpointId, interface, instance, attributes.value()));
        ThrowIfNull(controller, "createModeControllerFailed");
        return controller;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

ModeController::ModeController(
    const std::string& endpointId,
    const std::string& interface,
    const std::string& instance,
    ModeControllerAttributes attributes) :
        PrimitiveController(endpointId, interface, instance), m_attributes(attributes) {
    for (auto item : attributes.modes) {
        m_supportedModes.push_back(item.first);
    }
}

void ModeController::build(
    std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
    std::unique_ptr<EndpointBuilderInterface>& builder) {
    m_carControlServiceInterface = carControlServiceInterface;
    builder->withModeController(shared_from_this(), getInstance(), m_attributes, false, false, false);
}

ModeControllerConfiguration ModeController::getConfiguration() {
    return {m_supportedModes};
}

std::pair<alexaClientSDK::avsCommon::avs::AlexaResponseType, std::string> ModeController::setMode(
    const std::string& mode,
    const AlexaStateChangeCauseType cause) {
    try {
        AACE_DEBUG(LX(TAG)
                       .d("cause", alexaClientSDK::avsCommon::sdkInterfaces::alexaStateChangeCauseTypeToString(cause))
                       .sensitive("endpointId", getEndpointId())
                       .sensitive("instance", getInstance())
                       .sensitive("mode", mode));
        std::string newMode;
        ThrowIfNot(
            m_carControlServiceInterface->setModeControllerValue(getEndpointId(), getInstance(), mode),
            "setModeControllerValue");
        return std::make_pair(AlexaResponseType::SUCCESS, "");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(AlexaResponseType::INTERNAL_ERROR, "Something went wrong");
    }
}

std::pair<alexaClientSDK::avsCommon::avs::AlexaResponseType, std::string> ModeController::adjustMode(
    int modeDelta,
    const AlexaStateChangeCauseType cause) {
    try {
        AACE_DEBUG(LX(TAG)
                       .d("cause", alexaClientSDK::avsCommon::sdkInterfaces::alexaStateChangeCauseTypeToString(cause))
                       .sensitive("endpointId", getEndpointId())
                       .sensitive("instance", getInstance())
                       .sensitive("modeDelta", modeDelta));
        std::string newMode;
        ThrowIfNot(
            m_carControlServiceInterface->adjustModeControllerValue(getEndpointId(), getInstance(), modeDelta),
            "adjustModeFailed");
        return std::make_pair(AlexaResponseType::SUCCESS, "");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(AlexaResponseType::INTERNAL_ERROR, "Something went wrong");
    }
}

std::pair<
    alexaClientSDK::avsCommon::avs::AlexaResponseType,
    alexaClientSDK::avsCommon::utils::Optional<ModeController::ModeState>>
ModeController::getMode() {
    std::string mode;
    if (m_carControlServiceInterface->getModeControllerValue(getEndpointId(), getInstance(), mode)) {
        return std::make_pair(
            AlexaResponseType::SUCCESS,
            alexaClientSDK::avsCommon::utils::Optional<ModeController::ModeState>(ModeController::ModeState{
                mode, alexaClientSDK::avsCommon::utils::timing::TimePoint::now(), std::chrono::milliseconds(0)}));
    } else {
        return std::make_pair(
            AlexaResponseType::INTERNAL_ERROR, alexaClientSDK::avsCommon::utils::Optional<ModeController::ModeState>());
    }
}

bool ModeController::addObserver(std::shared_ptr<ModeController::ModeControllerObserverInterface> observer) {
    AACE_WARN(LX(TAG).m("Unexpected addObserver()"));
    return false;
}

void ModeController::removeObserver(const std::shared_ptr<ModeController::ModeControllerObserverInterface>& observer) {
    AACE_WARN(LX(TAG).m("Unexpected removeObserver()"));
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
