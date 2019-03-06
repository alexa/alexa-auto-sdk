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

#include "AACE/Engine/Navigation/NavigationEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace aace {
namespace engine {
namespace navigation {
    
// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.NavigationEngineImpl");

NavigationEngineImpl::NavigationEngineImpl( std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface ) :
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_navigationPlatformInterface( navigationPlatformInterface ) {
}

bool NavigationEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        m_navigationCapabilityAgent = NavigationCapabilityAgent::create( exceptionSender );
        ThrowIfNull( m_navigationCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // add navigation runtime observer
        m_navigationCapabilityAgent->addObserver( shared_from_this() );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( m_navigationCapabilityAgent ), "addDirectiveHandlerFailed" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_navigationCapabilityAgent ), "registerCapabilityFailed");
        AACE_DEBUG(LX(TAG,"RegisterCapability").d("Navigation","successful")); 

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<NavigationEngineImpl> NavigationEngineImpl::create(
    std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        ThrowIfNull( navigationPlatformInterface, "couldNotCreateNavigationPlatformInterface" );
        ThrowIfNull( capabilitiesDelegate, "couldNotCreateNavigationPlatformInterface" );
        ThrowIfNull( directiveSequencer, "couldNotCreateNavigationPlatformInterface" );
        ThrowIfNull( exceptionSender, "couldNotCreateNavigationPlatformInterface" );

        std::shared_ptr<NavigationEngineImpl> navigationEngineImpl = std::shared_ptr<NavigationEngineImpl>( new NavigationEngineImpl( navigationPlatformInterface ) );

        ThrowIfNot( navigationEngineImpl->initialize( directiveSequencer, capabilitiesDelegate, exceptionSender ), "initializeNavigationEngineImplFailed" );

        return navigationEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void NavigationEngineImpl::doShutdown()
{
    if( m_navigationCapabilityAgent != nullptr ) {
        m_navigationCapabilityAgent->shutdown();
    }
}

void NavigationEngineImpl::setDestination( const std::string& payload ) {
    if( m_navigationPlatformInterface != nullptr ) {
        m_navigationPlatformInterface->setDestination( payload );
    }
}

void NavigationEngineImpl::cancelNavigation() {
    if( m_navigationPlatformInterface != nullptr ) {
        m_navigationPlatformInterface->cancelNavigation();
    }
}


} // aace::engine::navigation
} // aace::engine
} // aace
