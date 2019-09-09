/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <unordered_map>
#include <forward_list>
#ifndef NO_SIGPIPE
#include <csignal>
#endif

#include "AACE/Engine/Core/EngineImpl.h"
#include "AACE/Engine/Core/EngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Core/EngineVersion.h"
#include "AACE/Engine/Core/CoreMetrics.h"
#include "AACE/Engine/Utils/JSON/JSON.h"
#include "AACE/Core/CoreProperties.h"

// default Engine constructor
std::shared_ptr<aace::core::Engine> aace::core::Engine::create() {
    return aace::engine::core::EngineImpl::create();
}

namespace aace {
namespace engine {
namespace core {

// default user agent constant
static const std::string USER_AGENT_NAME = "AlexaAutoSDK";

// String to identify log entries originating from this file.
static const std::string TAG("aace.core.EngineImpl");

std::shared_ptr<EngineImpl> EngineImpl::create() 
{
    try
    {
        auto engine = std::shared_ptr<EngineImpl>( new EngineImpl() );
        
        ThrowIfNull( engine, "createEngineFailed" );
        ThrowIfNot( engine->initialize(), "initializeFailed" );

        return engine;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

EngineImpl::~EngineImpl()
{
    // shutdown the engine if initialized
    if( m_initialized ) {
        shutdown();
    }
}

bool EngineImpl::initialize()
{
    try
    {
        AACE_INFO(LX(TAG,"initialize").d("engineVersion",aace::engine::core::version::getEngineVersion()));
    #ifndef NO_SIGPIPE
        AACE_VERBOSE(LX(TAG,"initialize").d("signal","SIGPIPE").d("value","SIG_IGN"));
        ThrowIf( std::signal( SIGPIPE, SIG_IGN ) == SIG_ERR, "setSignalFailed" );
    #endif

        ThrowIf( m_initialized, "engineAlreadyInitialized" );
        ThrowIfNot( checkServices(), "checkServicesFailed" );

        // iterate through registered engine services and call initialize() for each module
        for( auto next : m_orderedServiceList ) {
            ThrowIfNot( next->handleInitializeEngineEvent( shared_from_this() ), "handleInitializeEngineEventFailed" );
        }

        // set initialize flag
        m_initialized = true;
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

bool EngineImpl::shutdown()
{
    try
    {
        if( m_initialized == false ) {
            AACE_WARN(LX(TAG,"shutdown").m("Attempting to shutdown engine that is not initialized - doing nothing."));
            return true;
        }
        
        // engine must be stopped before shutdown, but continue with shutdown if failed...
        if( stop() == false ) {
            AACE_ERROR(LX(TAG,"shutdown").d("reason","stopEngineFailed"));
        }
        
        AACE_DEBUG(LX(TAG,"shutdown").m("EngineShutdown"));

        // iterate through registered engine services and call shutdown() for each module
        for( auto next : m_orderedServiceList )
        {
            AACE_DEBUG(LX(TAG,"shutdown").m(next->getDescription().getType()));

            // if shutting down the service failed throw an error but continue with
            // shutting down remaining services
            if( next->handleShutdownEngineEvent() == false ) {
                AACE_ERROR(LX(TAG,"shutdown").d("reason","handleShutdownEngineEventFailed").d("service",next->getDescription().getType()));
            }
        }
        
        // reset the engine state
        m_orderedServiceList.clear();
        m_registeredServiceMap.clear();
        m_initialized = false;
        m_configured = false;
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"shutdown").d("reason", ex.what()));
        return false;
    }
}

bool EngineImpl::configure( std::shared_ptr<aace::core::config::EngineConfiguration> configuration )
{
    try
    {
        ThrowIfNull( configuration, "invalidConfiguration" );
        return configure( { configuration } );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"configure").d("reason", ex.what()));
        return false;
    }
}

bool EngineImpl::configure( std::initializer_list<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList ) {
    return configure( std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>>( configurationList ) );
}

bool EngineImpl::configure( std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList )
{
    try
    {
        AACE_DEBUG(LX(TAG,"configure").m("EngineConfigure"));

        ThrowIfNot( m_initialized, "engineNotInitialized" );
        ThrowIf( m_running, "engineRunning" );
        ThrowIf( m_configured, "engineAlreadyConfigured" );
        ThrowIf( configurationList.empty(), "invalidConfigurationList" );

        // iterate through configuration objects and get streams for sdk initialization and
        // merge all configuration stream together before calling service config methods
        rapidjson::Document configuration( rapidjson::kObjectType );
        auto root = configuration.GetObject();

        for( auto next : configurationList )
        {
            ThrowIfNull( next, "invalidConfiguration" );
            
            // parse the next configuration stream
            auto document = aace::engine::utils::json::parse( next->getStream() );
            ThrowIfNull( document, "parseConfigurationStreamFailed" );
            
            // merge the document with the main configuration
            ThrowIfNot( document->IsObject(), "invalidConfigurationStream" );
            ThrowIfNot( aace::engine::utils::json::merge( root, document->GetObject(), configuration.GetAllocator() ), "mergeConfigurationFailed" );
        }
        
        // iterate through registered engine services and call configure() for each module
        for( auto nextService : m_orderedServiceList )
        {
            auto config = root.FindMember( nextService->getDescription().getType().c_str() );
        
            if( config != root.end() )
            {
                rapidjson::Document subDocument;
                
                subDocument.CopyFrom( config->value, subDocument.GetAllocator() );
            
                nextService->handleConfigureEngineEvent( aace::engine::utils::json::toStream( subDocument ) );
            }
        }

        m_configured = true;
        
        // iterate through registered engine modules and call handlePreRegisterEngineEvent() for each module
        for( auto next : m_orderedServiceList ) {
            ThrowIfNot( next->handlePreRegisterEngineEvent(), "handlePreRegisterEngineEvent" );
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"configure").d("reason", ex.what()));
        return false;
    }
}

bool EngineImpl::checkServices()
{
    try
    {
        std::unordered_map<std::string,std::shared_ptr<ServiceFactory>> registeredServiceFactoryMap;
        std::vector<std::shared_ptr<ServiceFactory>> orderedServiceFactoryList;
        std::forward_list<std::shared_ptr<ServiceFactory>> unresolvedDependencyList;

        for( auto it = EngineServiceManager::registryBegin(); it != EngineServiceManager::registryEnd(); it++ )
        {
            bool success = true;
            auto serviceFactory = it->second;
            auto desc = serviceFactory->getDescription();
        
            for( auto next : desc.getDependencies() )
            {
                auto it = registeredServiceFactoryMap.find( next.getType() );
            
                if( it != registeredServiceFactoryMap.end() )
                {
                    auto serviceFactory = it->second;
                    auto desc = serviceFactory->getDescription();
                    auto v1 = desc.getVersion();
                    auto v2 = next.getVersion();
                    
                    ThrowIf( v1 < v2, "invalidDependencyVersion" );
                }
                else
                {
                    success = false;
                    break;
                }
            }
            
            if( success )
            {
                orderedServiceFactoryList.push_back( serviceFactory );
                
                // add the service to the registered service map so we can resolve dependencies
                registeredServiceFactoryMap[desc.getType()] = serviceFactory;
            }
            else {
                unresolvedDependencyList.push_front( serviceFactory );
            }
        }
        
        while( unresolvedDependencyList.empty() == false )
        {
            bool updated = false;
            auto it = unresolvedDependencyList.begin();
            auto prev = unresolvedDependencyList.before_begin();
            
            while( it != unresolvedDependencyList.end() )
            {
                bool success = true;
                auto serviceFactory = *it;
                auto desc = serviceFactory->getDescription();

                for( auto next : desc.getDependencies() )
                {
                    auto it = registeredServiceFactoryMap.find( next.getType() );

                    if( it != registeredServiceFactoryMap.end() )
                    {
                        auto serviceFactory = it->second;
                        auto desc = serviceFactory->getDescription();
                        auto v1 = desc.getVersion();
                        auto v2 = next.getVersion();
                        
                        ThrowIf( v1 < v2, "invalidDependencyVersion" );
                    }
                    else
                    {
                        success = false;
                        break;
                    }
                }
                
                if( success )
                {
                    orderedServiceFactoryList.push_back( serviceFactory );
                
                    // add the service to the registered service map so we can resolve dependencies
                    registeredServiceFactoryMap[desc.getType()] = serviceFactory;

                    // remove the item from the list
                    it = unresolvedDependencyList.erase_after( prev );
                    
                    updated = true;
                }
                else {
                    prev = it++;
                }
            }
            
            // fail if we were not able to resolve any of the remaining dependencies
            ThrowIfNot( updated, "failedToResolveServiceDependencies" );
        }
        
        // instantiate the engine service objects
        for( auto next : orderedServiceFactoryList )
        {
            auto service = next->newInstance();
            ThrowIfNull( service, "createNewServiceInstanceFailed" );
            
            m_orderedServiceList.push_back( service );
            m_registeredServiceMap[service->getDescription().getType()] = service;
        }
        
        // dump list of services to log
        for( auto next : m_orderedServiceList ) {
            auto desc = next->getDescription();
            auto version = desc.getVersion();
            AACE_INFO(LX(TAG,"checkServices").m(desc.getType()).d("v",version.toString()));
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"checkServices").d("reason", ex.what()));
        m_orderedServiceList.clear();
        m_registeredServiceMap.clear();
        return false;
    }
}

bool EngineImpl::start()
{
    try
    {
        AACE_DEBUG(LX(TAG,"start").m("EngineStart"));
        CORE_METRIC(LX(TAG,"start"), aace::engine::core::CoreMetrics::Location::ENGINE_START_BEGIN);

        ThrowIf( m_running, "engineAlreadyRunning" );
        ThrowIfNot( m_initialized, "engineNotInitialized" );
        ThrowIfNot( m_configured, "engineNotConfigured" );
        
        // postRegister and setup are called for each service the first time the engine is started
        if( m_setup == false )
        {
            // iterate through registered engine modules and call handlePostRegisterEngineEvent() for each module
            for( auto next : m_orderedServiceList ) {
                ThrowIfNot( next->handlePostRegisterEngineEvent(), "handlePostRegisterEngineEvent" );
            }

            // iterate through registered engine modules and call handleSetupEngineEvent() for each module
            for( auto next : m_orderedServiceList ) {
                ThrowIfNot( next->handleSetupEngineEvent(), "handleSetupEngineEventFailed" );
            }
            
            // set the engine setup flag to true
            m_setup = true;
        }
        
        // iterate through registered engine modules and call handleStartEngineEvent() for each module
        for( auto next : m_orderedServiceList ) {
            ThrowIfNot( next->handleStartEngineEvent(), "handleStartEngineEventFailed" );
        }

        // set the engine running flag to true
        m_running = true;
        
        CORE_METRIC(LX(TAG,"start"), aace::engine::core::CoreMetrics::Location::ENGINE_START_END);
        return true;
    }
    catch( std::exception& ex ) {
        CORE_METRIC(LX(TAG,"start"), aace::engine::core::CoreMetrics::Location::ENGINE_START_EXCEPTION);
        AACE_ERROR(LX(TAG,"start").d("reason", ex.what()));
        return false;
    }
}

bool EngineImpl::stop()
{
    try
    {
        AACE_DEBUG(LX(TAG,"stop").m("EngineStop"));
        CORE_METRIC(LX(TAG,"stop"), aace::engine::core::CoreMetrics::Location::ENGINE_STOP_BEGIN);

        if( m_running == false ) {
            AACE_WARN(LX(TAG,"stop").m("Attempting to stop engine that is not running - doing nothing."));
            return true;
        }
    
        // iterate through registered engine modules and call stop() for each module
        for( auto next : m_orderedServiceList ) {
            ThrowIfNot( next->handleStopEngineEvent(), "handleStopEngineEventFailed" );
        }

        // set the engine running and configured flag to false - the engine must be reconfigured before starting again
        m_running = false;

        CORE_METRIC(LX(TAG,"stop"), aace::engine::core::CoreMetrics::Location::ENGINE_STOP_END);
        return true;
    }
    catch( std::exception& ex ) {
        CORE_METRIC(LX(TAG,"stop"), aace::engine::core::CoreMetrics::Location::ENGINE_STOP_EXCEPTION);
        AACE_ERROR(LX(TAG,"stop").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<EngineServiceContext> EngineImpl::getService( const std::string& type ) {
    auto it = m_registeredServiceMap.find( type );
    return it != m_registeredServiceMap.end() ? std::make_shared<EngineServiceContext>( it->second ) : nullptr;
}

std::shared_ptr<EngineService> EngineImpl::getServiceFromPropertyKey( const std::string& key )
{
    for( auto next : m_orderedServiceList )
    {
        std::string kcmp = next->getDescription().getType() + ".";

        if( key.compare( 0, kcmp.length(), kcmp ) == 0 ) {
            return next;
        }
    }
    
    return nullptr;
}

bool EngineImpl::setProperty( const std::string& key, const std::string& value )
{
    try
    {
        ThrowIf( key.empty(), "invalidPropertyKey" );
    
        auto service = getServiceFromPropertyKey( key );
        
        // if the property starts with a service id then delegate the setProperty() call to
        // the specified service implementation
        if( service != nullptr ) {
            return service->setProperty( key, value );
        }
        
        // check core properties
        else if( key.compare( aace::core::property::VERSION ) == 0 ) {
            Throw( "readOnlyProperty" );
        }
        
        // iterate through registered engine modules and call setProperty() for each module
        else
        {
            for( auto next : m_orderedServiceList ) {
                ReturnIf( next->setProperty( key, value ), true );
            }
        }
        
        Throw( "unhandledProperty" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"setProperty").d("reason", ex.what()).d("key",key).d("value",value));
        return false;
    }
}

std::string EngineImpl::getProperty( const std::string& key )
{
    try
    {
        ThrowIf( key.empty(), "invalidPropertyKey" );

        auto service = getServiceFromPropertyKey( key );
        
        // if the property starts with a service id then delegate the getProperty() call to
        // the specified service implementation
        if( service != nullptr ) {
            return service->getProperty( key );
        }
    
        // core properties
        else if( key.compare( aace::core::property::VERSION ) == 0 ) {
            return aace::engine::core::version::getEngineVersion().toString();
        }
        
        // iterate through registered engine modules and call getProperty() for each module
        else
        {
            std::string result;

            for( auto next : m_orderedServiceList ) {
                result = next->getProperty( key );
                ReturnIfNot( result.empty(), result );
            }

            Throw( "unhandledProperty" );
        }
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"getProperty").d("reason", ex.what()).d("key",key));
        return std::string();
    }
}

bool EngineImpl::registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
{
    try
    {
        ThrowIfNot( m_configured, "engineNotConfigured" );
        ThrowIf( m_setup, "engineHasAlreadyBeenStarted" );
        ThrowIfNull( platformInterface, "invalidPlatformInterface" );
    
        // iterate through registered engine modules and call registerPlatformInterface() for each module
        for( auto next : m_orderedServiceList ) {
            ReturnIf( next->handleRegisterPlatformInterfaceEngineEvent( platformInterface ), true );
        }

        Throw( "platformInterfaceNotRegistered" );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool EngineImpl::registerPlatformInterface( std::initializer_list<std::shared_ptr<aace::core::PlatformInterface>> platformInterfaceList )
{
    try
    {
        ThrowIf( platformInterfaceList.size() == 0, "invalidPlatformInterfaceList" );
    
        for( auto next : platformInterfaceList ) {
            ThrowIfNot( registerPlatformInterface( next ), "registerPlatformInterfaceFailed" );
        }
        
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::core
} // aace::engine
} // aace

