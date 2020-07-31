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

#include <AACE/Engine/CarControl/PrimitiveController.h>
#include <AVSCommon/AVS/ActionsToDirectiveMapping.h>

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

alexaClientSDK::avsCommon::utils::Optional<CapabilitySemantics> PrimitiveController::getSemantics(
    const json& semantics) {
    try {
        ThrowIfNot(semantics.contains("actionMappings"), "missingActionMappings");
        alexaClientSDK::avsCommon::avs::CapabilitySemantics capabilitySemantics;
        auto& actionMappings = semantics.at("actionMappings");
        for (auto& item : actionMappings.items()) {
            ThrowIfNot(
                (item.value().contains("@type") && item.value().at("@type") == "ActionsToDirective"),
                "expectedActionsToDirectiveType");
            alexaClientSDK::avsCommon::avs::ActionsToDirectiveMapping actionMapping;

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
