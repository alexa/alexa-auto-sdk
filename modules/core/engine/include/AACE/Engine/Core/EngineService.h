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

#ifndef AACE_ENGINE_CORE_ENGINE_SERVICE_H
#define AACE_ENGINE_CORE_ENGINE_SERVICE_H

#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <istream>

#include "AACE/Engine/Core/ServiceDescription.h"
#include "AACE/Core/PlatformInterface.h"

namespace aace {
namespace engine {
namespace core {

class EngineImpl;
class EngineContext;
class ServiceDescription;

class EngineService {
protected:
    EngineService( const aace::engine::core::ServiceDescription& description );

public:
    virtual ~EngineService();

    bool isRunning();
    bool isInitialized();
    bool isConfigured();

    const ServiceDescription& getDescription();

protected:
    virtual bool initialize();
    virtual bool configure( const std::vector<std::shared_ptr<std::istream>>& configuration );
    virtual bool shutdown();
    virtual bool start();
    virtual bool stop();
    virtual bool registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface );
    virtual bool setProperty( const std::string& key, const std::string& value );
    virtual std::string getProperty( const std::string& key );

    std::shared_ptr<aace::engine::core::EngineContext> getContext();

private:
    bool handleInitializeEngineEvent( std::shared_ptr<aace::engine::core::EngineContext> context );
    bool handleConfigureEngineEvent( const std::vector<std::shared_ptr<std::istream>>& configuration );
    bool handleShutdownEngineEvent();
    bool handleStartEngineEvent();
    bool handleStopEngineEvent();
    bool handleRegisterPlatformInterfaceEngineEvent( std::shared_ptr<aace::core::PlatformInterface> platformInterface );

private:
    std::shared_ptr<aace::engine::core::EngineContext> m_context;
    
    ServiceDescription m_description;
    
    bool m_initialized;
    bool m_running;
    
    // allow the EngineImpl call private functions in this class
    friend class aace::engine::core::EngineImpl;
};

//
// EngineContext
//

class EngineContext {
public:
    template <class T>
    std::shared_ptr<T> getService() {
        return std::dynamic_pointer_cast<T>( getService( T::getServiceDescription().getType() ) );
    }
    
protected:
    virtual std::shared_ptr<EngineService> getService( const std::string& type ) = 0;
};

} // aace::engine::core
} // aace::engine
} // aace

#endif // AACE_ENGINE_CORE_ENGINE_SERVICE_H
