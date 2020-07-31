/*
* Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/CBL/CBLConfiguration.h"
#include "AACE/Engine/Utils/JSON/JSON.h"

namespace aace {
namespace cbl {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.config.CBLConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> CBLConfiguration::createCBLConfig(const int seconds) {
    rapidjson::Document document(rapidjson::kObjectType);
    rapidjson::Value aaceCBLNode(rapidjson::kObjectType);

    aaceCBLNode.AddMember("requestTimeout", rapidjson::Value().SetInt(seconds), document.GetAllocator());

    document.AddMember("aace.cbl", aaceCBLNode, document.GetAllocator());

    return aace::core::config::StreamConfiguration::create(aace::engine::utils::json::toStream(document));
}

std::shared_ptr<aace::core::config::EngineConfiguration> CBLConfiguration::createCBLUserProfileConfig(
    bool enableUserProfile) {
    rapidjson::Document document(rapidjson::kObjectType);
    rapidjson::Value aaceCBLNode(rapidjson::kObjectType);

    aaceCBLNode.AddMember("enableUserProfile", rapidjson::Value().SetBool(enableUserProfile), document.GetAllocator());

    document.AddMember("aace.cbl", aaceCBLNode, document.GetAllocator());

    return aace::core::config::StreamConfiguration::create(aace::engine::utils::json::toStream(document));
}

}  // namespace config
}  // namespace cbl
}  // namespace aace
