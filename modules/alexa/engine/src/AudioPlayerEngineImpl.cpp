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

#include "AACE/Engine/Alexa/AudioPlayerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AVSCommon/Utils/MediaPlayer/PooledMediaPlayerFactory.h>

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioPlayerEngineImpl");

AudioPlayerEngineImpl::AudioPlayerEngineImpl( std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface ) :
    AudioChannelEngineImpl( alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::Type::AVS_SPEAKER_VOLUME ),
    m_audioPlayerPlatformInterface( audioPlayerPlatformInterface ) {
}

bool AudioPlayerEngineImpl::initialize(
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender ) {
    
    try
    {
        ThrowIfNot( initializeAudioChannel( audioOutputChannel, speakerManager ), "initializeAudioChannelFailed" );
    
        auto factory = alexaClientSDK::mediaPlayer::PooledMediaPlayerFactory::create({shared_from_this()});
    
        // create the capability agent
        m_audioPlayerCapabilityAgent = alexaClientSDK::capabilityAgents::audioPlayer::AudioPlayer::create( std::move(factory), messageSender, focusManager, contextManager, exceptionSender, playbackRouter, certifiedSender );
        ThrowIfNull( m_audioPlayerCapabilityAgent, "couldNotCreateCapabilityAgent" );
        
        // add audio observer
        m_audioPlayerCapabilityAgent->addObserver( std::dynamic_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface>( shared_from_this() ) );

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability( m_audioPlayerCapabilityAgent, m_audioPlayerCapabilityAgent );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AudioPlayerEngineImpl> AudioPlayerEngineImpl::create(
    std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface,
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender ) {
    
    std::shared_ptr<AudioPlayerEngineImpl> audioPlayerEngineImpl = nullptr;

    try
    {
        ThrowIfNull( audioPlayerPlatformInterface, "invalidAudioPlayerPlatformInterface" );
        ThrowIfNull( audioManager, "invalidAudioManager" );
        ThrowIfNull( defaultEndpointBuilder, "invalidDefaultEndpointBuilder" );
        ThrowIfNull( messageSender, "invalidMessageSender" );
        ThrowIfNull( focusManager, "invalidFocusManager" );
        ThrowIfNull( contextManager, "invalidContextManager" );
        ThrowIfNull( attachmentManager, "invalidAttachmentManager" );
        ThrowIfNull( capabilitiesDelegate, "invalidCapabilitiesDelegate" );
        ThrowIfNull( speakerManager, "invalidSpeakerManager" );
        ThrowIfNull( exceptionSender, "invalidExceptionSender" );
        ThrowIfNull( playbackRouter, "invalidPlaybackRouter" );
        ThrowIfNull( certifiedSender, "invalidCertifiedSender" );
        
        // open the Music audio channel
        auto audioOutputChannel = audioManager->openAudioOutputChannel( "AudioPlayer", aace::audio::AudioOutputProvider::AudioOutputType::MUSIC );
        ThrowIfNull( audioOutputChannel, "openAudioOutputChannelFailed" );

        audioPlayerEngineImpl = std::shared_ptr<AudioPlayerEngineImpl>( new AudioPlayerEngineImpl( audioPlayerPlatformInterface ) );
        
        ThrowIfNot( audioPlayerEngineImpl->initialize( audioOutputChannel, defaultEndpointBuilder, messageSender, focusManager, contextManager, attachmentManager, capabilitiesDelegate, speakerManager, exceptionSender, playbackRouter, certifiedSender ), "initializeAudioPlayerEngineImplFailed" );

        return audioPlayerEngineImpl;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"create").d("reason", ex.what()));
        if( audioPlayerEngineImpl != nullptr ) {
            audioPlayerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void AudioPlayerEngineImpl::doShutdown()
{
    if( m_audioPlayerCapabilityAgent != nullptr ) {
        m_audioPlayerCapabilityAgent->removeObserver( std::dynamic_pointer_cast<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface>( shared_from_this() ) );
        m_audioPlayerCapabilityAgent->shutdown();
    }

    AudioChannelEngineImpl::doShutdown();
    m_audioPlayerPlatformInterface.reset();
}

//
// AudioPlayerObserverInterface
//
void AudioPlayerEngineImpl::onPlayerActivityChanged( alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context ) {
    m_audioPlayerPlatformInterface->playerActivityChanged( static_cast<aace::alexa::AudioPlayer::PlayerActivity>( state ) );
}
    
void AudioPlayerEngineImpl::setObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface> observer) {
    std::lock_guard<std::mutex> lock{m_observersMutex};
//    m_renderPlayerObserver = observer;
    m_audioPlayerCapabilityAgent->setObserver(observer);
}

} // aace::engine::alexa
} // aace::engine
} // aace
