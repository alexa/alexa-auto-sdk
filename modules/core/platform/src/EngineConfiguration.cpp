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

#include "AACE/Core/EngineConfiguration.h"

namespace aace {
namespace core {
namespace config {

//
// StreamConfiguration
//

std::shared_ptr<StreamConfiguration> StreamConfiguration::create(std::shared_ptr<std::istream> stream) {
    auto config = std::shared_ptr<StreamConfiguration>(new StreamConfiguration());

    config->m_stream = stream;

    return config;
}

std::shared_ptr<std::istream> StreamConfiguration::getStream() {
    return m_stream;
}

//
// ConfigurationFile
//

std::shared_ptr<ConfigurationFile> ConfigurationFile::create(const std::string& configFilePath) {
    auto config = std::shared_ptr<ConfigurationFile>(new ConfigurationFile());

    config->m_stream = std::make_shared<std::ifstream>(configFilePath);

    return config;
}

std::shared_ptr<std::istream> ConfigurationFile::getStream() {
    return m_stream;
}

}  // namespace config
}  // namespace core
}  // namespace aace
