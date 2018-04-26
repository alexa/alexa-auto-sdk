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

#include "AACE/Engine/Alexa/PlaybackControllerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.PlaybackControllerEngineImpl");

PlaybackControllerEngineImpl::PlaybackControllerEngineImpl( std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface ) {
}

std::shared_ptr<PlaybackControllerEngineImpl> PlaybackControllerEngineImpl::create(
    std::shared_ptr<aace::alexa::PlaybackController> playbackControllerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager ) {
    
    try
    {
        std::shared_ptr<PlaybackControllerEngineImpl> playbackControllerEngineImpl = std::shared_ptr<PlaybackControllerEngineImpl>( new PlaybackControllerEngineImpl( playbackControllerPlatformInterface ) );
        
        auto playbackControllerCapabilityAgent = alexaClientSDK::capabilityAgents::playbackController::PlaybackController::create( contextManager, messageSender );
        ThrowIfNull( playbackControllerCapabilityAgent, "couldNotCreateCapabilityAgent" );

        auto playbackRouter = alexaClientSDK::capabilityAgents::playbackController::PlaybackRouter::create( playbackControllerCapabilityAgent );
        ThrowIfNull( playbackRouter, "couldNotCreatePlaybackRouter" );
        
        // set the playback router reference
        playbackControllerEngineImpl->m_playbackRouter = playbackRouter;
        
        // set the capability agent reference in the playback controller engine implementation
        playbackControllerEngineImpl->m_playbackControllerCapabilityAgent = playbackControllerCapabilityAgent;

        return playbackControllerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> PlaybackControllerEngineImpl::getPlaybackRouter() {
    return m_playbackRouter;
}
    
void PlaybackControllerEngineImpl::onPlayButtonPressed() {
    m_playbackRouter->playButtonPressed();
}

void PlaybackControllerEngineImpl::onPauseButtonPressed() {
    m_playbackRouter->pauseButtonPressed();
}

void PlaybackControllerEngineImpl::onNextButtonPressed() {
    m_playbackRouter->nextButtonPressed();
}

void PlaybackControllerEngineImpl::onPreviousButtonPressed() {
    m_playbackRouter->previousButtonPressed();
}
    
} // aace::engine::alexa
} // aace::engine
} // aace
