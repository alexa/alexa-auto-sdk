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
 * The @c Engine must be instantiated by the platform implementation. Along with the platform interfaces,
 * it is responsible for managing the interactions between the platform and AVS.
 *
 * The platform implementation should *not* extend Engine. It is currently extended in the SDK.
 */
class Engine {
public:
    /**
     * Creates a new instance of an Engine object.
     */
    static std::shared_ptr<Engine> create();

    /**
     * Destructor.
     * @internal
     */
    virtual ~Engine() = default;

     /**
     * Sets the Engine configuration to a set of configuration objects.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] configurationList Collection of @c aace::config::EngineConfiguration objects passed in as
     * @c std::initializer_list<aace::config::EngineConfiguration*>.
     * @return @c true if successful, else @c false.
     */
    virtual bool configure( std::initializer_list<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList ) = 0;
    
    /**
     * Sets the Engine configuration to a set of configuration objects.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] configurationList Collection of @c aace::config::EngineConfiguration objects passed in as
     * @c std::vector<aace::config::EngineConfiguration*>.
     * @return @c true if successful, else @c false.
     */
    virtual bool configure( std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList ) = 0;
    
    /**
     * Sets the Engine configuration to a single configuration object.
     *
     * @sa [Configuring the Engine](index.html#configuring-the-engine)
     * @param [in] configuration @c An aace::config::EngineConfiguration object.
     * @return @c true if successful, else @c false.
     */
    virtual bool configure( std::shared_ptr<aace::core::config::EngineConfiguration> configuration ) = 0;

    /**
     * Starts the Engine and attempts to establish a connection to AVS.
     *
     * @return @c true if successful, else @c false.
     * @sa stop()
     */
    virtual bool start() = 0;

    /**
     * Stops the Engine and shuts down the connection to AVS.
     *
     * @return @c true if successful, else @c false.
     * @sa start()
     */
    virtual bool stop() = 0;

    /**
     * Sets a property value in the Engine.
     *
     * @param [in] key @c The property key used by the Engine to identify the property.
     * @param [in] value @c The value to set in the Engine.
     * @return @c true if property value was set, else @c false.
     */
    virtual bool setProperty( const std::string& key, const std::string& value ) = 0;
    
    /**
     * Register a @c PlatformInterface instance with the Engine.
     *
     * The platform implementation must register any interfaces necessary for the application.
     *
     * @sa aace::core::PlatformInterface 
     * 
     * @param [in] platformInterface The @c PlatformInterface instance to register.
     * @return @c true if successful, else @c false.
     */
    virtual bool registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface ) = 0;

    /**
     * Register a list of @c PlatformInterface instances with the Engine.
     *
     * The platform implementation must register any interfaces necessary for the application.
     *
     * @sa aace::core::PlatformInterface 
     *      
     * @param [in] platformInterfaceList The @c list of PlatformInterface instances to register.
     * @return @c true if all interfaces were registered successfully, else @c false.
     * @sa registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
     */
    virtual bool registerPlatformInterface( std::initializer_list<std::shared_ptr<aace::core::PlatformInterface>> platformInterfaceList ) = 0;
};

} // aace::core
} // aace

#endif // AACE_CORE_ENGINE_H
