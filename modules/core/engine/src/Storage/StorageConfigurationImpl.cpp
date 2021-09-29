/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "AACE/Engine/Utils/JSON/JSON.h"

namespace aace {
namespace storage {
namespace config {

// json namespace alias
namespace json = aace::engine::utils::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.storage.config.StorageConfiguationImpl");

std::shared_ptr<aace::core::config::EngineConfiguration> StorageConfiguration::createLocalStorageConfig(
    const std::string& localStoragePath) {
    try {
        // clang-format off
        json::Value config = {
            {"aace.storage",{
                {"localStoragePath",localStoragePath}
            }}
        };
        // clang-format on

        return aace::core::config::StreamConfiguration::create(json::toStream(config));
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace config
}  // namespace storage
}  // namespace aace
