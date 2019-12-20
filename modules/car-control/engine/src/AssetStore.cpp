/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/CarControl/AssetsDefault.h>
#include <AACE/Engine/CarControl/AssetStore.h>
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

AssetStore::AssetStore(const std::string& locale) : m_locale{locale} {
}

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

std::string AssetStore::getLocale() {
    return m_locale;
}

const std::vector<std::string>& AssetStore::getValues(const std::string& id) {
    if (m_assets.find(id) == m_assets.end()) {
        return m_empty;
    }
    return m_assets[id];
}

void AssetStore::print() {
    for (auto& item : m_assets) {
        std::cout << item.first << std::endl;
        for (auto value = item.second.begin(); value != item.second.end(); ++value) {
            std::cout << "'" + *value + "' ";
        }
        std::cout << std::endl;
    }
}

bool AssetStore::addAssets(std::istream& stream) {
    try {
        if (stream.good()) {
            json j = json::parse(stream);
            json items = j.at("assets");
            for (auto& item : items.items()) {
                std::vector<std::string> items;
                auto asset = item.value();
                std::string assetId = asset["assetId"];
                json values = asset["values"];
                for (auto& valueItem : values.items()) {
                    json value = valueItem.value();
                    std::vector<std::string> locales = value.at("locales");
                    if (std::find(locales.begin(), locales.end(), m_locale) == locales.end()) continue;
                    std::string defaultValue = value["defaultValue"];
                    json synonyms = value["synonyms"];
                    items.push_back(defaultValue);
                    for (auto& synonym : synonyms.items()) {
                        items.push_back(synonym.value());
                    }
                }
                ThrowIf(items.empty(), "noAssetFriendlyNameFor " + assetId);
                m_assets.emplace(assetId, items);
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

void AssetStore::clear() {
    for (auto value : m_assets) {
        value.second.clear();
    }
    m_assets.clear();
}

bool AssetStore::addDefaultAssets() {
    std::stringstream stream(ASSETS_DEFAULT);
    return addAssets(stream);
}

}  // namespace carControl
}  // namespace engine
}  // namespace aace
