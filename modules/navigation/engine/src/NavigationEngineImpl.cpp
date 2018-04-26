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


std::shared_ptr<NavigationEngineImpl> NavigationEngineImpl::create(
    std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        std::shared_ptr<NavigationEngineImpl> navigationEngineImpl = std::shared_ptr<NavigationEngineImpl>( new NavigationEngineImpl( navigationPlatformInterface ) );
        auto navigationCapabilityAgent = NavigationCapabilityAgent::create( exceptionSender );
        ThrowIfNull( navigationCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // add navigation runtime observer
        navigationCapabilityAgent->addObserver( navigationEngineImpl );
        
        // set the capability agent reference in the engine implementation
        navigationEngineImpl->m_navigationCapabilityAgent = navigationCapabilityAgent;

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( navigationCapabilityAgent ), "addDirectiveHandlerFailed" );

        return navigationEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void NavigationEngineImpl::doShutdown() {
    if( m_navigationCapabilityAgent != nullptr ) {
        m_navigationCapabilityAgent->shutdown();
    }
}

void NavigationEngineImpl::setDestination( const std::string& payload ) {
    m_navigationPlatformInterface->setDestination( payload );
}

} // aace::engine::navigation
} // aace::engine
} // aace
