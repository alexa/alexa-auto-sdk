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

#include <AACE/Engine/CarControl/PrimitiveController.h>
#include <AVSCommon/AVS/CapabilitySemantics/ActionsToDirectiveMapping.h>

#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.carControl.PrimitiveController");

PrimitiveController::PrimitiveController(
    const std::string& endpointId,
    const std::string& interface,
    const std::string& instance) :
        CapabilityController(endpointId, interface), m_instance(instance) {
}

PrimitiveController::~PrimitiveController() {
}

std::string PrimitiveController::getId() {
    return getInterface() + "#" + getInstance();
}
std::string PrimitiveController::getInstance() {
    return m_instance;
}

alexaClientSDK::avsCommon::utils::Optional<CapabilityResources> PrimitiveController::getResources(
    const json& resources,
    const AssetStore& assetStore) {
    try {
        ThrowIfNot(resources.contains("friendlyNames"), "missingFriendlyNames");
        alexaClientSDK::avsCommon::avs::CapabilityResources capabilityResources;
        auto& friendlyNames = resources.at("friendlyNames");
        for (auto& item : friendlyNames.items()) {
            auto& type = item.value().at("@type");
            // aace.carControl config only allows "asset" type labels.
            // (https://developer.amazon.com/en-US/docs/alexa/device-apis/resources-and-assets.html#label-object)
            ThrowIfNot(type == "asset", "expectedAssetTypeResource");
            auto& value = item.value().at("value");
            std::string assetId = value.at("assetId");
            // Expand assets present in the AssetStore. Use the asset ID for assets that are absent
            const std::vector<AssetStore::NameLocalePair>& names = assetStore.getFriendlyNames(assetId);
            if (names.empty()) {
                capabilityResources.addFriendlyNameWithAssetId(assetId);
            } else {
                AACE_DEBUG(LX(TAG).m("expandingAssetToText").d("assetID", assetId));
                for (auto name = names.begin(); name != names.end(); ++name) {
                    capabilityResources.addFriendlyNameWithText(name->first, name->second);
                }
            }
        }
        return alexaClientSDK::avsCommon::utils::Optional<CapabilityResources>(capabilityResources);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::Optional<CapabilityResources>();
    }
}

alexaClientSDK::avsCommon::utils::Optional<CapabilitySemantics> PrimitiveController::getSemantics(
    const json& semantics) {
    try {
        ThrowIfNot(semantics.contains("actionMappings"), "missingActionMappings");
        alexaClientSDK::avsCommon::avs::capabilitySemantics::CapabilitySemantics capabilitySemantics;
        auto& actionMappings = semantics.at("actionMappings");
        for (auto& item : actionMappings.items()) {
            ThrowIfNot(
                (item.value().contains("@type") && item.value().at("@type") == "ActionsToDirective"),
                "expectedActionsToDirectiveType");
            alexaClientSDK::avsCommon::avs::capabilitySemantics::ActionsToDirectiveMapping actionMapping;

            ThrowIfNot(item.value().contains("actions"), "actionMappingMissingActions");
            auto& actions = item.value().at("actions");
            ThrowIf(actions.empty(), "actionMappingHasEmptyActions");
            for (auto& action : actions.items()) {
                actionMapping.addAction(action.value().get<std::string>());
            }

            ThrowIfNot(item.value().contains("directive"), "actionMappingMissingDirective");
            auto& directive = item.value().at("directive");
            ThrowIfNot(directive.contains("name"), "actionMappingDirectiveMissingName");
            ThrowIfNot(directive.contains("payload"), "actionMappingDirectiveMissingPayload");
            std::string name = directive.at("name").get<std::string>();
            std::string payload = directive.at("payload").dump();
            actionMapping.setDirective(name, payload);

            capabilitySemantics.addActionsToDirectiveMapping(actionMapping);
        }
        return alexaClientSDK::avsCommon::utils::Optional<CapabilitySemantics>(capabilitySemantics);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return alexaClientSDK::avsCommon::utils::Optional<CapabilitySemantics>();
    }
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
