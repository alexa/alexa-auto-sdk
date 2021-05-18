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

#ifndef AACE_ENGINE_CAR_CONTROL_ASSET_STORE_H
#define AACE_ENGINE_CAR_CONTROL_ASSET_STORE_H

#include <istream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace aace {
namespace engine {
namespace carControl {

/**
 * The AssetStore ingests assets from a file and stores the friendly name /
 * locale pairs associated with each asset definition in the file.
 * Friendly name / locale pairs of assets may be retrieved by asset ID when
 * constructing a discovery message with assets expanded to text.
 */
class AssetStore {
public:
    /// Alias for readability. Pair of friendly name literal text to its locale
    using NameLocalePair = std::pair<std::string, std::string>;

    /// Destructor
    ~AssetStore();

    /**
     * Ingest the assets file at the given path and populate the AssetStore
     * with the text/locale pairs. The contents of the file must contain the
     * assets JSON in the expected schema.
     *
     * @param path The path of the assets file to ingest
     * @return @c true if the assets were ingested successfully; @c false if 
     * there was an issue such as malformed or missing values
     */
    bool addAssets(const std::string& path);

    /**
     * Get the literal friendly names and locales associated with the given 
     * asset ID.
     *
     * @param The ID of the asset
     * @return A list of pairs of friendly name and locale strings for the asset
     */
    const std::vector<NameLocalePair>& getFriendlyNames(const std::string& assetId) const;

    /**
     * Clear the contents of the AssetStore
     */
    void clear();

private:
    /**
     * Ingest the assets from the istream and populate the AssetStore with the
     * friendly name text / locale pairs. The contents of the stream must 
     * contain the assets JSON in the expected schema.
     *
     * @param stream The istream containing the assets to ingest
     * @return true if the assets were ingested successfully; false if there was
     * an issue such as malformed or missing values
     */
    bool addAssets(std::istream& stream);

    /**
     * A map of asset ID to a list of friendly name text / locale pairs used to
     * describe the asset. It contains an entry for all assets ingested by the
     * AssetStore.
     */
    std::unordered_map<std::string, std::vector<NameLocalePair>> m_assets;

    /**
     * An empty list to be returned from @c getFriendlyNames() if the specified ID
     * isn't present in the AssetStore
     */
    std::vector<NameLocalePair> m_empty;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_ASSET_STORE_H
