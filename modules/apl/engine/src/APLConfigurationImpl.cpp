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

#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/JSON/JSON.h>

#include "AACE/APL/APLConfiguration.h"

namespace aace {
namespace apl {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.apl.config.APLConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> APLConfiguration::createAlexaPresentationTimeoutConfig(
    const std::vector<AlexaPresentationTimeout>& timeoutList) {
    rapidjson::Document document(rapidjson::kObjectType);
    rapidjson::Value aaceAlexaElement(rapidjson::kObjectType);
    rapidjson::Value avsDeviceSDKElement(rapidjson::kObjectType);
    rapidjson::Value alexaPresentationCapabilityAgentElement(rapidjson::kObjectType);

    using TimeoutType = aace::apl::config::APLConfiguration::AlexaPresentationTimeoutType;

    for (auto next : timeoutList) {
        std::string name;

        switch (next.first) {
            case TimeoutType::DISPLAY_DOCUMENT_INTERACTION_IDLE_TIMEOUT:
                name = "displayDocumentInteractionIdleTimeout";
                break;
        }

        alexaPresentationCapabilityAgentElement.AddMember(
            rapidjson::Value().SetString(name.c_str(), name.length(), document.GetAllocator()),
            rapidjson::Value().SetInt(next.second.count()),
            document.GetAllocator());
    }

    avsDeviceSDKElement.AddMember(
        "alexaPresentationCapabilityAgent", alexaPresentationCapabilityAgentElement, document.GetAllocator());

    aaceAlexaElement.AddMember("avsDeviceSDK", avsDeviceSDKElement, document.GetAllocator());
    document.AddMember("aace.alexa", aaceAlexaElement, document.GetAllocator());  // alexa dependency

    return aace::core::config::StreamConfiguration::create(aace::engine::utils::json::toStream(document));
}

}  // namespace config
}  // namespace apl
}  // namespace aace
