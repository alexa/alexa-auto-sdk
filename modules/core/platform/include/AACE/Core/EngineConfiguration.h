/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * Abstract base class for providing @c JSON configuration data to the Engine.
 */
class EngineConfiguration {
public:
    virtual ~EngineConfiguration() = default;

    /**
     * @return Pointer to a @c std::istream object containing the @c JSON configuration data.
     */
    virtual std::shared_ptr<std::istream> getStream() = 0;
};

/**
 * Configuration class used to provide configuration data from a stream. The contents
 * of the stream should be @c JSON data with values corresponding to the stream content provided
 * by the other configuration objects.
 *
 * @sa [Configuring the Engine](index.html#configuring-the-engine)
 */
class StreamConfiguration : public EngineConfiguration {
private:
    StreamConfiguration() = default;

public:
    /**
     * Creates a new instance of @c StreamConfiguration by providing the input stream
     * containing @c JSON data.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] stream Input stream.
     */
    static std::shared_ptr<StreamConfiguration> create( std::shared_ptr<std::istream> stream );

    /**
     * @return Pointer to a @c std::istream object containing the @c JSON configuration data.
     */
    std::shared_ptr<std::istream> getStream() override;
    
private:
    std::shared_ptr<std::istream> m_stream;
};

/**
 * Configuration class used to provide configuration data from a file. The contents
 * of the file should be @c JSON data with values corresponding to the stream content provided
 * by the other configuration objects.
 *
 * @sa [Configuring the Engine](index.html#configuring-the-engine)
 */
class ConfigurationFile : public EngineConfiguration {
private:
    ConfigurationFile() = default;

public:
    /**
     * Creates a new instance of @c ConfigurationFile by providing the path to a configuration file
     * containing @c JSON data.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] configFilePath Path to the configuration file.
     */
    static std::shared_ptr<ConfigurationFile> create( const std::string& configFilePath );
    
    /**
     * @return Pointer to a @c std::istream object containing the @c JSON configuration data.
     */
    std::shared_ptr<std::istream> getStream() override;
    
private:
    std::shared_ptr<std::istream> m_stream;
};

} // aace::core::config
} // aace::core
} // aace

#endif // AACE_CORE_CONFIG_ENGINE_CONFIGURATION_H
