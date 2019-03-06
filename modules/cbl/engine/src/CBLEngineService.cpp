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

#include <typeinfo>
#include <climits>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include "AACE/Engine/CBL/CBLEngineService.h"
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace cbl {

// String to identify log entries originating from this file.
static const std::string TAG("aace.cbl.CBLEngineService");

// Default for code pair request timeout 
static const std::chrono::minutes DEFAULT_REQUEST_TIMEOUT = std::chrono::minutes(1);

// Default for configured base URL for LWA requests.
static const std::string DEFAULT_CBL_ENDPOINT = "https://api.amazon.com/auth/O2/";

// register the service
REGISTER_SERVICE(CBLEngineService);

CBLEngineService::CBLEngineService( const aace::engine::core::ServiceDescription& description ) : 
    aace::engine::core::EngineService( description ),
    m_codePairRequestTimeout( DEFAULT_REQUEST_TIMEOUT ),
    m_endpoint( DEFAULT_CBL_ENDPOINT ) {
}

bool CBLEngineService::configure ( const std::vector< std::shared_ptr<std::istream>>& configuration )
{
    try
    {
        for( auto next : configuration ) {
            configure( next );
        }
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"configure").d("reason", ex.what()));
        return false;
    }
}

bool CBLEngineService::configure( std::shared_ptr<std::istream> configuration )
{
    try
    {
        bool handled = false;

        rapidjson::IStreamWrapper isw( *configuration );
        rapidjson::Document document;
        
        document.ParseStream( isw );
        
        ThrowIf( document.HasParseError(), GetParseError_En( document.GetParseError() ) );
        ThrowIfNot( document.IsObject(), "invalidConfigurationStream" );
        
        auto root = document.GetObject();
        
        if( root.HasMember( "aace.cbl" ) && root["aace.cbl"].IsObject() )
        {
            auto cblConfigRoot = root["aace.cbl"].GetObject();

            if( cblConfigRoot.HasMember( "requestTimeout" ) && cblConfigRoot["requestTimeout"].IsUint() ) {
                m_codePairRequestTimeout = std::chrono::seconds( cblConfigRoot["requestTimeout"].GetUint() );
            }

            if( cblConfigRoot.HasMember( "endpoint" ) && cblConfigRoot["endpoint"].IsString() ){
                m_endpoint = cblConfigRoot["endpoint"].GetString();
            }
            handled = true;
        }

        return handled;
    }
    catch( std::exception& ex ) {
        AACE_WARN(LX(TAG,"configure").d("reason", ex.what()));
        configuration->clear();
        configuration->seekg( 0 );
        return false;
    }
}

bool CBLEngineService::start()
{
    if( m_cblEngineImpl != nullptr ) {
        m_cblEngineImpl->engineStart();
    }
    
    return true;
}

bool CBLEngineService::stop()
{
    if( m_cblEngineImpl != nullptr ) {
        m_cblEngineImpl->onCancel();
    }
    
    return true;
}

bool CBLEngineService::shutdown()
{
    if( m_cblEngineImpl != nullptr ) {
        m_cblEngineImpl->shutdown();
        m_cblEngineImpl.reset();
    }
    
    return true;
}

bool CBLEngineService::registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
{
    try
    {
        return registerPlatformInterfaceType<aace::cbl::CBL>( platformInterface );
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool CBLEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::cbl::CBL> cbl )
{
    try
    {
        ThrowIfNotNull( m_cblEngineImpl, "platformInterfaceAlreadyRegistered" );
    
        auto alexaComponents = getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull( alexaComponents, "invalidAlexaComponentInterface" );

        auto customerDataManager = alexaComponents->getCustomerDataManager();
        ThrowIfNull( customerDataManager, "invalidCustomerDataManager" );

        auto deviceInfo = alexaComponents->getDeviceInfo();
        ThrowIfNull( deviceInfo, "invalidDeviceInfo" );

        m_cblEngineImpl = aace::engine::cbl::CBLEngineImpl::create( cbl, customerDataManager, deviceInfo, m_codePairRequestTimeout, m_endpoint );
        ThrowIfNull( m_cblEngineImpl, "createCBLEngineImplFailed" );
        
        auto alexaService = getContext()->getService<aace::engine::alexa::AlexaEngineService>();
        ThrowIfNull( alexaService, "alexaServiceNotFound" );

        alexaService->registerServiceFactory<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface>( [this] {
            return m_cblEngineImpl;
        });

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<CBL>").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::cbl
} // aace::engine
} // aace
