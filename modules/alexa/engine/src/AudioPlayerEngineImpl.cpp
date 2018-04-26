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

#include "AACE/Engine/Alexa/AudioPlayerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioPlayerEngineImpl");

AudioPlayerEngineImpl::AudioPlayerEngineImpl( std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface ) :
    AudioChannelEngineImpl( audioPlayerPlatformInterface ),
    alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
    m_audioPlayerPlatformInterface( audioPlayerPlatformInterface ) {
}

std::shared_ptr<AudioPlayerEngineImpl> AudioPlayerEngineImpl::create(
    std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender ) {
    
    try
    {
        auto audioPlayerEngineImpl = std::shared_ptr<AudioPlayerEngineImpl>( new AudioPlayerEngineImpl( audioPlayerPlatformInterface ) );
        
        // create the playback router instance
        audioPlayerEngineImpl->m_playbackRouter = std::shared_ptr<PlaybackRouterDelegate>( new PlaybackRouterDelegate() );
        
        // create the capability agent
        auto audioPlayerCapabilityAgent = alexaClientSDK::capabilityAgents::audioPlayer::AudioPlayer::create( std::static_pointer_cast<MediaPlayerInterface>( audioPlayerEngineImpl ), connectionManager, focusManager, contextManager, attachmentManager, exceptionSender, audioPlayerEngineImpl->m_playbackRouter );
        
        ThrowIfNull( audioPlayerCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // set the capability agent reference in the audio player engine implementation
        audioPlayerEngineImpl->m_audioPlayerCapabilityAgent = audioPlayerCapabilityAgent;
        
        // add audio observer
        audioPlayerCapabilityAgent->addObserver( audioPlayerEngineImpl );

        // add capability agent to the directive sequencer
        ThrowIfNot( directiveSequencer->addDirectiveHandler( audioPlayerCapabilityAgent ), "addDirectiveHandlerFailed" );

        return audioPlayerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        return nullptr;
    }
}

void AudioPlayerEngineImpl::doShutdown()
{
    if( m_audioPlayerCapabilityAgent != nullptr ) {
        m_audioPlayerCapabilityAgent->shutdown();
    }
}

void AudioPlayerEngineImpl::setPlaybackRouter( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> router ) {
    m_playbackRouter->setDelegate( router );
}

//
// AudioPlayerObserverInterface
//
void AudioPlayerEngineImpl::onPlayerActivityChanged( alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context ) {
    m_audioPlayerPlatformInterface->playerActivityChanged( static_cast<aace::alexa::AudioPlayer::PlayerActivity>( state ) );
}

} // aace::engine::alexa
} // aace::engine
} // aace
