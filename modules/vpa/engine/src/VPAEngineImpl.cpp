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

#include <sstream>

#include "AACE/Engine/VPA/VPAEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace aace {
namespace engine {
namespace vpa {
    
// String to identify log entries originating from this file.
static const std::string TAG("aace.vpa.VPAEngineImpl");

VPAEngineImpl::VPAEngineImpl( std::shared_ptr<aace::vpa::VpaDirective> vpaDirectivePlatformInterface ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_vpaDirectivePlatformInterface( vpaDirectivePlatformInterface ) {
}

bool VPAEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager ) {
    
    try
    {
        m_vpaCapabilityAgent = VPACapabilityAgent::create( exceptionSender );
        ThrowIfNull( m_vpaCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // add navigation runtime observer
        m_vpaCapabilityAgent->addObserver( shared_from_this() );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_vpaCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_vpaCapabilityAgent ), "registerCapabilityFailed");
        AACE_DEBUG(LX(TAG,"RegisterCapability").d("VPA","successful")); 

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<VPAEngineImpl> VPAEngineImpl::create(
    std::shared_ptr<aace::vpa::VpaDirective> vpaDirectivePlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager ) {
    
    try
    {
        ThrowIfNull( vpaDirectivePlatformInterface, "couldNotCreatevpaDirectivePlatformInterface" );
        ThrowIfNull( capabilitiesDelegate, "couldNotCreatevpaDirectivePlatformInterface" );
        ThrowIfNull( directiveSequencer, "couldNotCreatevpaDirectivePlatformInterface" );
        ThrowIfNull( exceptionSender, "couldNotCreatevpaDirectivePlatformInterface" );
        ThrowIfNull( contextManager, "couldNotCreateVPAContextManager" );

        std::shared_ptr<VPAEngineImpl> vpaEngineImpl = std::shared_ptr<VPAEngineImpl>( new VPAEngineImpl( vpaDirectivePlatformInterface ) );

        ThrowIfNot( vpaEngineImpl->initialize( directiveSequencer, capabilitiesDelegate, exceptionSender, contextManager ), "initializeNavigationEngineImplFailed" );

        return vpaEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void VPAEngineImpl::doShutdown()
{
    if( m_vpaCapabilityAgent != nullptr ) {
        m_vpaCapabilityAgent->shutdown();
    }
}

void VPAEngineImpl::sendDirective( const std::string& payload ) {
    if( m_vpaDirectivePlatformInterface != nullptr ) {
        m_vpaDirectivePlatformInterface->sendDirective( payload );
    }
}

} // aace::engine::vpa
} // aace::engine
} // aace
