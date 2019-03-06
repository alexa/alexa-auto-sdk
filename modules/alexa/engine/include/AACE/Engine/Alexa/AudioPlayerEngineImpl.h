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

#ifndef AACE_ENGINE_ALEXA_AUDIO_PLAYER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_AUDIO_PLAYER_ENGINE_IMPL_H

#include <ACL/AVSConnectionManager.h>
#include <AVSCommon/AVS/Attachment/AttachmentManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/AudioPlayerObserverInterface.h>

#include <AudioPlayer/AudioPlayer.h>
#include <ContextManager/ContextManager.h>

#include "AACE/Alexa/AudioPlayer.h"
#include "AudioChannelEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

class PlaybackRouterDelegate;

class AudioPlayerEngineImpl :
    public AudioChannelEngineImpl,
    public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerInterface,
    public alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface {
    
private:
    AudioPlayerEngineImpl( std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface );

    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter );

public:
    static std::shared_ptr<AudioPlayerEngineImpl> create(
        std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter );
    
    //
    // AudioPlayerInterface
    //
    void addObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer ) override {
        m_audioPlayerCapabilityAgent->addObserver( observer );
    }
    
    void removeObserver( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AudioPlayerObserverInterface> observer ) override {
        m_audioPlayerCapabilityAgent->removeObserver( observer );
    }
    
    std::chrono::milliseconds getAudioItemOffset() override {
        return m_audioPlayerCapabilityAgent->getAudioItemOffset();
    }
    
    //
    // AudioPlayerObserverInterface
    //
    void onPlayerActivityChanged( alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context ) override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::AudioPlayer> m_audioPlayerPlatformInterface = nullptr;
    std::shared_ptr<alexaClientSDK::capabilityAgents::audioPlayer::AudioPlayer> m_audioPlayerCapabilityAgent;
};

} // aace::engine::alexa
} // aace::engine
} // aace

#endif // AACE_ENGINE_ALEXA_AUDIO_PLAYER_ENGINE_IMPL_H
