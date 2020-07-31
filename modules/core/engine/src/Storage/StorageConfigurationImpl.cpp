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

#include "AACE/Storage/StorageConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace aace {
namespace storage {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.storage.config.StorageConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> StorageConfiguration::createLocalStorageConfig(
    const std::string& localStoragePath) {
    rapidjson::Document document;

    document.SetObject();

    rapidjson::Value aaceStorageNode(rapidjson::kObjectType);

    aaceStorageNode.AddMember(
        "localStoragePath",
        rapidjson::Value().SetString(localStoragePath.c_str(), localStoragePath.length()),
        document.GetAllocator());

    document.AddMember("aace.storage", aaceStorageNode, document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    document.Accept(writer);

    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(buffer.GetString()));
}

}  // namespace config
}  // namespace storage
}  // namespace aace
