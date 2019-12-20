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

#ifndef AACE_ENGINE_CAR_CONTROL_ASSET_STORE_H
#define AACE_ENGINE_CAR_CONTROL_ASSET_STORE_H

#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace aace {
namespace engine {
namespace carControl {

class AssetStore {
public:
    AssetStore(const std::string& locale);
    ~AssetStore();
    bool addAssets(const std::string& path);
    bool addDefaultAssets();
    std::string getLocale();
    const std::vector<std::string>& getValues(const std::string& id);
    void print();

private:
    bool addAssets(std::istream& stream);
    void clear();
    std::unordered_map<std::string, std::vector<std::string>> m_assets;
    std::string m_locale;
    std::vector<std::string> m_empty;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_ASSET_STORE_H
