/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <nlohmann/json.hpp>

#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Metrics/MetricsConfiguration.h"

using json = nlohmann::json;

namespace aace {
namespace metrics {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.engine.metrics.MetricsConfiguration");

std::shared_ptr<aace::core::config::EngineConfiguration> MetricsConfiguration::createMetricsTagConfig(
    const std::string& metricDeviceIdTag) {
    if (metricDeviceIdTag.empty()) {
        return nullptr;
    }
    json config = {{"aace.metrics", {{"metricDeviceIdTag", metricDeviceIdTag}}}};
    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(config.dump()));
}

std::shared_ptr<aace::core::config::EngineConfiguration> MetricsConfiguration::createMetricsStorageConfig(
    const std::string& storagePath) {
    if (storagePath.empty()) {
        return nullptr;
    }
    json config = {{"aace.metrics", {{"metricStoragePath", storagePath}}}};
    return aace::core::config::StreamConfiguration::create(std::make_shared<std::stringstream>(config.dump()));
}

}  // namespace config
}  // namespace metrics
}  // namespace aace