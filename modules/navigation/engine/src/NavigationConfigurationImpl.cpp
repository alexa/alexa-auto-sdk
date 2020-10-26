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

#include "AACE/Navigation/NavigationConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include <rapidjson/pointer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace aace {
namespace navigation {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.config.NavigationConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> NavigationConfiguration::createNavigationConfig(
    const std::string& providerName) {
    rapidjson::Document document(rapidjson::kObjectType);
    rapidjson::Value aaceNavigationElement(rapidjson::kObjectType);

    aaceNavigationElement.AddMember(
        "providerName",
        rapidjson::Value().SetString(providerName.c_str(), providerName.length()),
        document.GetAllocator());

    document.AddMember("aace.navigation", aaceNavigationElement, document.GetAllocator());

    return aace::core::config::StreamConfiguration::create(aace::engine::utils::json::toStream(document));
}

}  // namespace config
}  // namespace navigation
}  // namespace aace
