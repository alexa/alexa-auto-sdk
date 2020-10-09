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

#ifndef AACE_CORE_ENGINE_H
#define AACE_CORE_ENGINE_H

#include <vector>
#include <memory>

#include "PlatformInterface.h"
#include "EngineConfiguration.h"

/** @file */

namespace aace {
namespace core {

/**
 * The Engine must be instantiated by the platform implementation. Along with the platform interfaces,
 * it is responsible for managing the interactions between the platform and AVS.
 *
 * The platform implementation should *not* extend Engine; it is extended in the SDK.
 */
class Engine {
public:
    static std::shared_ptr<Engine> create();

    virtual ~Engine() = default;

    /**
     * Sets the Engine configuration to a set of configuration objects
     *
     * @param [in] configurationList A collection of @c aace::core::config::EngineConfiguration objects as an      
     * @c std::initializer_list<aace::core::config::EngineConfiguration*>
     * @return @c true if the Engine configuration was successful, else @c false
     */
    virtual bool configure(
        std::initializer_list<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList) = 0;

    /**
     * Sets the Engine configuration to a set of configuration objects
     *
     * @param [in] configurationList A collection of @c aace::core::config::EngineConfiguration objects as an
     * @c std::vector<aace::core::config::EngineConfiguration*>
     * @return @c true if the Engine configuration was successful, else @c false
     */
    virtual bool configure(std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList) = 0;

    /**
     * Sets the Engine configuration to a single configuration object
     *
     * @param [in] configuration An @c aace::core::config::EngineConfiguration object
     * @return @c true if the Engine configuration was successful, else @c false
     */
    virtual bool configure(std::shared_ptr<aace::core::config::EngineConfiguration> configuration) = 0;

    /**
     * Starts the Engine and attempts to establish a connection to AVS
     *
     * @return @c true if the Engine was started, else @c false
     *
     * @sa stop()
     */
    virtual bool start() = 0;

    /**
     * Stops the Engine and shuts down the connection to AVS
     *
     * @return @c true if the Engine was stopped, else @c false
     *
     * @sa start()
     */
    virtual bool stop() = 0;

    /**
     * Shuts down the Engine and releases all of its resources
     *
     * @return @c true if the Engine was shut down, else @c false
     */
    virtual bool shutdown() = 0;

    /**
     * Registers a @c PlatformInterface instance with the Engine
     *
     * The platform implementation must register each interface required by the application.
     * 
     * @param [in] platformInterface The @c PlatformInterface instance to register
     * @return @c true if the @c PlatformInterface instance was registered, else @c false
     *
     * @sa aace::core::PlatformInterface 
     */
    virtual bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) = 0;

    /**
     * Registers a list of @c PlatformInterface instances with the Engine
     *
     * The platform implementation must register each interface required by the application.
     *      
     * @param [in] platformInterfaceList The list of @c PlatformInterface instances to register
     * @return @c true if all @c PlatformInterface instances were registered, else @c false
     *
     * @sa aace::core::PlatformInterface 
     * @sa registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
     */
    virtual bool registerPlatformInterface(
        std::initializer_list<std::shared_ptr<aace::core::PlatformInterface>> platformInterfaceList) = 0;
};

}  // namespace core
}  // namespace aace

#endif  // AACE_CORE_ENGINE_H
