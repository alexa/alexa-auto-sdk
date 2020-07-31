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

#include "AACE/Engine/CarControl/RangeController.h"

#include <AVSCommon/AVS/CapabilitySemantics.h>
#include <RangeController/RangeControllerAttributeBuilder.h>

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.carControl.RangeController");

/// Aliased for readability
/// @{
using AlexaResponseType = alexaClientSDK::avsCommon::avs::AlexaResponseType;
using RangeControllerConfiguration =
    alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerInterface::RangeControllerConfiguration;
using RangeState = alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerInterface::RangeState;
/// @}

std::shared_ptr<RangeController> RangeController::create(
    const json& controllerConfig,
    const std::string& endpointId,
    const std::string& interface,
    const AssetStore& assetStore) {
    try {
        std::string instance = controllerConfig.at("instance");
        ThrowIf(instance.empty(), "missingInstance");

        auto attributeBuilder =
            alexaClientSDK::capabilityAgents::rangeController::RangeControllerAttributeBuilder::create();
        alexaClientSDK::avsCommon::avs::CapabilityResources capabilityResources;

        auto& friendlyNames = controllerConfig.at("capabilityResources").at("friendlyNames");
        for (auto& item : friendlyNames.items()) {
            auto& value = item.value().at("value");
            std::string assetId = value.at("assetId");
            const std::vector<AssetStore::NameLocalePair>& names = assetStore.getFriendlyNames(assetId);
            for (auto name = names.begin(); name != names.end(); ++name) {
                capabilityResources.addFriendlyNameWithText(name->first, name->second);
            }
        }
        attributeBuilder->withCapabilityResources(capabilityResources);
        auto& configuration = controllerConfig.at("configuration");
        if (configuration.contains("presets")) {
            auto& presets = configuration.at("presets");
            for (auto& preset : presets.items()) {
                alexaClientSDK::avsCommon::sdkInterfaces::rangeController::PresetResources presetResources;
                double rangeValue = preset.value().at("rangeValue");
                auto& friendlyNames = preset.value().at("presetResources").at("friendlyNames");
                std::vector<std::string> assetIds;
                for (auto& item : friendlyNames.items()) {
                    auto& value = item.value().at("value");
                    std::string assetId = value.at("assetId");
                    const std::vector<AssetStore::NameLocalePair>& names = assetStore.getFriendlyNames(assetId);
                    for (auto name = names.begin(); name != names.end(); ++name) {
                        presetResources.addFriendlyNameWithText(name->first, name->second);
                    }
                }
                attributeBuilder->addPreset({rangeValue, presetResources});
            }
        }
        if (configuration.contains("unitOfMeasure")) {
            auto unitOfMeasure = configuration.at("unitOfMeasure");
            attributeBuilder->withUnitOfMeasure(unitOfMeasure);
        }

        if (controllerConfig.contains("semantics")) {
            auto& semanticsJson = controllerConfig.at("semantics");
            alexaClientSDK::avsCommon::utils::Optional<alexaClientSDK::avsCommon::avs::CapabilitySemantics> semantics =
                getSemantics(semanticsJson);
            ThrowIfNot(semantics.hasValue(), "failedToParseSemanticsConfig");
            attributeBuilder->withSemantics(semantics.value());
        }

        auto attributes = attributeBuilder->build();
        ThrowIfNot(attributes.hasValue(), "invalidAttributes");

        double minimum, maximum, precision;
        auto& supportedRange = configuration.at("supportedRange");
        minimum = supportedRange.at("minimumValue");
        maximum = supportedRange.at("maximumValue");
        precision = supportedRange.at("precision");

        auto controller = std::shared_ptr<RangeController>(
            new RangeController(endpointId, interface, instance, attributes.value(), minimum, maximum, precision));
        ThrowIfNull(controller, "createRangeControllerFailed");
        return controller;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

RangeController::RangeController(
    const std::string& endpointId,
    const std::string& interface,
    const std::string& instance,
    RangeControllerAttributes attributes,
    double minimum,
    double maximum,
    double precision) :
        PrimitiveController(endpointId, interface, instance),
        m_attributes(attributes),
        m_minimum(minimum),
        m_maximum(maximum),
        m_precision(precision) {
}

void RangeController::build(
    std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
    std::unique_ptr<EndpointBuilder>& builder) {
    m_carControlServiceInterface = carControlServiceInterface;
    builder->withRangeController(shared_from_this(), getInstance(), m_attributes, false, false, false);
}

RangeControllerConfiguration RangeController::getConfiguration() {
    return {m_minimum, m_maximum, m_precision};
}

std::pair<alexaClientSDK::avsCommon::avs::AlexaResponseType, std::string> RangeController::setRangeValue(
    double rangeValue,
    const AlexaStateChangeCauseType cause) {
    try {
        AACE_DEBUG(LX(TAG)
                       .d("cause", alexaClientSDK::avsCommon::sdkInterfaces::alexaStateChangeCauseTypeToString(cause))
                       .sensitive("endpointId", getEndpointId())
                       .sensitive("instance", getInstance())
                       .sensitive("rangeValue", rangeValue));
        ThrowIfNot(
            m_carControlServiceInterface->setRangeControllerValue(getEndpointId(), getInstance(), rangeValue),
            "setRangeControllerValueFailed");
        return std::make_pair(AlexaResponseType::SUCCESS, "");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(AlexaResponseType::INTERNAL_ERROR, "Something went wrong");
    }
}

std::pair<alexaClientSDK::avsCommon::avs::AlexaResponseType, std::string> RangeController::adjustRangeValue(
    double rangeDelta,
    const AlexaStateChangeCauseType cause) {
    AACE_DEBUG(LX(TAG)
                   .d("cause", alexaClientSDK::avsCommon::sdkInterfaces::alexaStateChangeCauseTypeToString(cause))
                   .sensitive("endpointId", getEndpointId())
                   .sensitive("instance", getInstance())
                   .sensitive("rangeDelta", rangeDelta));
    try {
        ThrowIfNot(
            m_carControlServiceInterface->adjustRangeControllerValue(getEndpointId(), getInstance(), rangeDelta),
            "adjustRangeControllerValueFailed");
        return std::make_pair(AlexaResponseType::SUCCESS, "");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return std::make_pair(AlexaResponseType::INTERNAL_ERROR, "Something went wrong");
    }
}

std::pair<alexaClientSDK::avsCommon::avs::AlexaResponseType, alexaClientSDK::avsCommon::utils::Optional<RangeState>>
RangeController::getRangeState() {
    double value;
    if (m_carControlServiceInterface->getRangeControllerValue(getEndpointId(), getInstance(), value)) {
        return std::make_pair(
            AlexaResponseType::SUCCESS,
            alexaClientSDK::avsCommon::utils::Optional<RangeControllerInterface::RangeState>(
                alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerInterface::RangeState{
                    value, alexaClientSDK::avsCommon::utils::timing::TimePoint::now(), std::chrono::milliseconds(0)}));
    } else {
        return std::make_pair(
            AlexaResponseType::INTERNAL_ERROR,
            alexaClientSDK::avsCommon::utils::Optional<RangeControllerInterface::RangeState>());
    }
}

bool RangeController::addObserver(std::shared_ptr<RangeControllerObserverInterface> observer) {
    AACE_WARN(LX(TAG).m("Unexpected addObserver()"));
    return false;
}

void RangeController::removeObserver(const std::shared_ptr<RangeControllerObserverInterface>& observer) {
    AACE_WARN(LX(TAG).m("Unexpected removeObserver()"));
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
