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

#include <AACE/Engine/CarControl/AssetStore.h>
#include <AACE/Engine/CarControl/AssetsDefault.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <sstream>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace aace {
namespace engine {
namespace carControl {

/// String to identify log entries originating from this file.
static const std::string TAG("aace.carControl.AssetStore");

AssetStore::~AssetStore() {
    clear();
}

bool AssetStore::addAssets(const std::string& path) {
    try {
        std::ifstream ifs(path);
        return addAssets(ifs);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AssetStore::addDefaultAssets() {
    std::stringstream stream(ASSETS_DEFAULT);
    return addAssets(stream);
}

bool AssetStore::addAssets(std::istream& stream) {
    try {
        if (stream.good()) {
            json j = json::parse(stream);
            json assetArray = j.at("assets");
            for (auto& assetItem : assetArray.items()) {
                // For each asset, add an entry to the asset map
                // 'names' will hold all synonyms for all locales for all values
                std::vector<NameLocalePair> names;
                auto assetObject = assetItem.value();
                std::string assetId = assetObject["assetId"];
                json valuesArray = assetObject["values"];
                for (auto& valueItem : valuesArray.items()) {
                    json valueObject = valueItem.value();
                    std::string defaultValue = valueObject["defaultValue"];
                    json synonyms = valueObject["synonyms"];
                    std::vector<std::string> locales = valueObject.at("locales");
                    // For every locale, add the defaultValue and each synonym
                    // to the list of names for this assetId
                    for (auto locale = locales.begin(); locale != locales.end(); ++locale) {
                        names.push_back({defaultValue, *locale});
                        for (auto& synonym : synonyms.items()) {
                            names.push_back({synonym.value(), *locale});
                        }
                    }
                }
                ThrowIf(names.empty(), "noAssetFriendlyNameFor " + assetId);
                m_assets.emplace(assetId, names);
            }
            return true;
        }
        return false;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        clear();
        return false;
    }
}

const std::vector<AssetStore::NameLocalePair>& AssetStore::getFriendlyNames(const std::string& assetId) const {
    auto iter = m_assets.find(assetId);
    if (iter != m_assets.end()) {
        return iter->second;
    } else {
        return m_empty;
    }
}

void AssetStore::clear() {
    for (auto value : m_assets) {
        value.second.clear();
    }
    m_assets.clear();
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
