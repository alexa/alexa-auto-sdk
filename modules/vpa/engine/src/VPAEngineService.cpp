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

#include "AACE/Engine/VPA/VPAEngineService.h"
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace aace {
namespace engine {
namespace vpa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.vpa.VPAEngineService");

// register the service
REGISTER_SERVICE(VPAEngineService);

VPAEngineService::VPAEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

bool VPAEngineService::configure( std::shared_ptr<std::istream> configuration )
{
    return true;
}

bool VPAEngineService::shutdown()
{
    if( m_vpaEngineImpl != nullptr ) {
        m_vpaEngineImpl->shutdown();
    }
    
    return true;
}

bool VPAEngineService::registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface )
{
    try
    {
        ReturnIf( registerPlatformInterfaceType<aace::vpa::VpaDirective>( platformInterface ), true );
        return false;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what()));
        return false;
    }
}

bool VPAEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::vpa::VpaDirective> vpa )
{
    AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<VPA>"));
    try
    {
        ThrowIfNotNull( m_vpaEngineImpl, "platformInterfaceAlreadyRegistered" );

        // create the vpa engine implementation
        auto alexaComponents = getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull( alexaComponents, "invalidAlexaComponentInterface" );
        
        auto directiveSequencer = alexaComponents->getDirectiveSequencer();
        ThrowIfNull( directiveSequencer, "directiveSequencerInvalid" );

        auto capabilitiesDelegate = alexaComponents->getCapabilitiesDelegate();
        ThrowIfNull( capabilitiesDelegate, "capabilitiesDelegateInvalid" );

        auto exceptionSender = alexaComponents->getExceptionEncounteredSender();
        ThrowIfNull( exceptionSender, "exceptionSenderInvalid" );

        auto contextManager = alexaComponents->getContextManager();
        ThrowIfNull( contextManager, "contextManagerInvalid" );

        m_vpaEngineImpl = aace::engine::vpa::VPAEngineImpl::create( vpa, directiveSequencer, capabilitiesDelegate, exceptionSender, contextManager);
        ThrowIfNull( m_vpaEngineImpl, "createVPAEngineImplFailed" );

        vpa->setLocalStorage(getContext()->getServiceInterface<aace::engine::storage::LocalStorageInterface>( "aace.storage" ));

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<Navigation>").d("reason", ex.what()));
        return false;
    }
}

} // aace::engine::vpa
} // aace::engine
} // aace

