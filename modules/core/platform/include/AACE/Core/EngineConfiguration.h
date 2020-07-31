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

#ifndef AACE_CORE_CONFIG_ENGINE_CONFIGURATION_H
#define AACE_CORE_CONFIG_ENGINE_CONFIGURATION_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>

/** @file */

namespace aace {
namespace core {
namespace config {

/**
 * Base class for providing JSON configuration data to the Engine.
 */
class EngineConfiguration {
public:
    virtual ~EngineConfiguration() = default;

    /**
         * @return A pointer to a @c std::istream object containing the JSON configuration data
         */
    virtual std::shared_ptr<std::istream> getStream() = 0;
};

/**
 * Configuration class used to provide configuration data from a stream. The contents
 * of the stream should be JSON data with values corresponding to the stream content provided
 * by the other configuration objects.
 */
class StreamConfiguration : public EngineConfiguration {
private:
    StreamConfiguration() = default;

public:
    /**
         * Creates a new instance of @c StreamConfiguration by providing the input stream
         * containing JSON data
         *
         * @param [in] stream The input stream
         */
    static std::shared_ptr<StreamConfiguration> create(std::shared_ptr<std::istream> stream);

    /**
         * @return A pointer to a @c std::istream object containing the JSON configuration data
         */
    std::shared_ptr<std::istream> getStream() override;

private:
    std::shared_ptr<std::istream> m_stream;
};

/**
 * Configuration class used to provide configuration data from a file. The contents
 * of the file should be JSON data with values corresponding to the content provided
 * by the other configuration objects.
 */
class ConfigurationFile : public EngineConfiguration {
private:
    ConfigurationFile() = default;

public:
    /**
         * Creates a new instance of @c ConfigurationFile by providing the path to a configuration file
         * containing JSON data.
         *
         * @param [in] configFilePath The path to the configuration file
         */
    static std::shared_ptr<ConfigurationFile> create(const std::string& configFilePath);

    /**
         * @return A pointer to a @c std::istream object containing the JSON configuration data
         */
    std::shared_ptr<std::istream> getStream() override;

private:
    std::shared_ptr<std::istream> m_stream;
};

}  // namespace config
}  // namespace core
}  // namespace aace

#endif  // AACE_CORE_CONFIG_ENGINE_CONFIGURATION_H
