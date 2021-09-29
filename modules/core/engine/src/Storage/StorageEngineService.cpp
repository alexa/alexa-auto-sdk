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

#include <string>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Storage/StorageEngineService.h"
#include "AACE/Engine/Storage/SQLiteStorage.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Utils/String/StringUtils.h"

namespace aace {
namespace engine {
namespace storage {

namespace json = aace::engine::utils::json;

// String to identify log entries originating from this file.
static const std::string TAG("aace.storage.StorageEngineService");

// register the service
REGISTER_SERVICE(StorageEngineService)

StorageEngineService::StorageEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool StorageEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto root = json::toJson(configuration);
        ThrowIfNull(root, "parseConfigurationFailed");

        auto localStoragePath = json::get(root, "/localStoragePath", json::Type::string);
        if (localStoragePath != nullptr) {
            std::string type = json::get(root, "/storageType", "sqlite");
            if (aace::engine::utils::string::equal(type, "sqlite", false)) {
                m_localStorage = SQLiteStorage::create(localStoragePath);
                ThrowIfNull(m_localStorage, "createLocalStorageFailed");
            } else {
                Throw("invalidStorageType:" + type);
            }
        }
        // register the local storage interface
        ThrowIfNot(registerServiceInterface<LocalStorageInterface>(m_localStorage), "registerServiceInterfaceFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

}  // namespace storage
}  // namespace engine
}  // namespace aace
