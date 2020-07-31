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

#include "AACE/Engine/Storage/StorageEngineService.h"
#include "AACE/Engine/Storage/SQLiteStorage.h"
#include "AACE/Engine/Storage/JSONStorage.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace engine {
namespace storage {

// String to identify log entries originating from this file.
static const std::string TAG("aace.storage.StorageEngineService");

// register the service
REGISTER_SERVICE(StorageEngineService)

StorageEngineService::StorageEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::core::EngineService(description) {
}

bool StorageEngineService::configure(std::shared_ptr<std::istream> configuration) {
    try {
        auto document = aace::engine::utils::json::parse(configuration);
        ThrowIfNull(document, "parseConfigurationStreamFailed");

        auto storageConfigRoot = document->GetObject();

        if (storageConfigRoot.HasMember("localStoragePath") && storageConfigRoot["localStoragePath"].IsString()) {
            ThrowIfNotNull(m_localStorage, "localStorageAlreadyConfigured");

            // create the local storage storage
            auto localStoragePath = storageConfigRoot["localStoragePath"].GetString();
            std::string type = storageConfigRoot.HasMember("storageType") && storageConfigRoot["storageType"].IsString()
                                   ? storageConfigRoot["storageType"].GetString()
                                   : "sqlite";

            if (type == "sqlite") {
                m_localStorage = SQLiteStorage::create(localStoragePath);
            } else if (type == "json") {
#ifdef DEBUG
                m_localStorage = JSONStorage::create(localStoragePath);
#else
                Throw("debugStorageTypeSpecified:" + type);
#endif
            } else {
                Throw("invalidStorageType:" + type);
            }
        }

        // register the local storage interface
        ThrowIfNull(m_localStorage, "localStorageNotConfigured");
        registerServiceInterface<LocalStorageInterface>(m_localStorage);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "configure").d("reason", ex.what()));
        return false;
    }
}

}  // namespace storage
}  // namespace engine
}  // namespace aace
