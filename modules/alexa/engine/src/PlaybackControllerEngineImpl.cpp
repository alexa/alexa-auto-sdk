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

#include "AACE/Engine/Alexa/PlaybackControllerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.PlaybackControllerEngineImpl");

PlaybackControllerEngineImpl::PlaybackControllerEngineImpl( std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface ) : alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

bool PlaybackControllerEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate ) {
    
    try
    {
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );

        m_playbackControllerCapabilityAgent = alexaClientSDK::capabilityAgents::playbackController::PlaybackController::create( contextManager, messageSender );
        ThrowIfNull( m_playbackControllerCapabilityAgent, "couldNotCreateCapabilityAgent" );

        m_playbackRouter = alexaClientSDK::capabilityAgents::playbackController::PlaybackRouter::create( m_playbackControllerCapabilityAgent );
        ThrowIfNull( m_playbackRouter, "couldNotCreatePlaybackRouter" );

        // register capability with delegate
        ThrowIfNot( capabilitiesDelegate->registerCapability( m_playbackControllerCapabilityAgent ), "registerCapabilityFailed");

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<PlaybackControllerEngineImpl> PlaybackControllerEngineImpl::create(
    std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate ) {

    std::shared_ptr<PlaybackControllerEngineImpl> playbackControllerEngineImpl = nullptr;

    try
    {
        ThrowIfNull( playbackControllerPlatformInterface, "invalidPlaybackControllerPlatformInterface" );

        playbackControllerEngineImpl = std::shared_ptr<PlaybackControllerEngineImpl>( new PlaybackControllerEngineImpl( playbackControllerPlatformInterface ) );

        ThrowIfNot( playbackControllerEngineImpl->initialize( messageSender, contextManager, capabilitiesDelegate ), "initializePlaybackControllerEngineImplFailed" );

        return playbackControllerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( playbackControllerEngineImpl != nullptr ) {
            playbackControllerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void PlaybackControllerEngineImpl::doShutdown()
{
    if( m_playbackRouter != nullptr ) {
        m_playbackRouter->shutdown();
    }

    if( m_playbackControllerCapabilityAgent != nullptr ) {
        m_playbackControllerCapabilityAgent->shutdown();
    }
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> PlaybackControllerEngineImpl::getPlaybackRouter() {
    return m_playbackRouter;
}
    
void PlaybackControllerEngineImpl::onButtonPressed(PlaybackButton button) {
    m_playbackRouter->buttonPressed(static_cast<alexaClientSDK::avsCommon::avs::PlaybackButton>( button ));
}

void PlaybackControllerEngineImpl::onTogglePressed(PlaybackToggle toggle, bool action) {
    m_playbackRouter->togglePressed(static_cast<alexaClientSDK::avsCommon::avs::PlaybackToggle>( toggle ), action);
}
    
} // aace::engine::alexa
} // aace::engine
} // aace
