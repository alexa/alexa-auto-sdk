/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <acsdkAudioPlayer/AudioPlayer.h>
#include <acsdkAudioPlayerInterfaces/AudioPlayerObserverInterface.h>
#include <AVSCommon/AVS/Attachment/AttachmentManagerInterface.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesRegistrarInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackHandlerInterface.h>
#include <AVSCommon/SDKInterfaces/PlaybackRouterInterface.h>
#include <AVSCommon/Utils/Metrics/MetricRecorderInterface.h>
#include <ContextManager/ContextManager.h>

#include <AACE/Alexa/AlexaEngineInterfaces.h>
#include <AACE/Alexa/AudioPlayer.h>
#include <AACE/Engine/Audio/AudioManagerInterface.h>

#include "AudioChannelEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

class PlaybackRouterDelegate;

class AudioPlayerEngineImpl
        : public AudioChannelEngineImpl
        , public aace::alexa::AudioPlayerEngineInterface
        , public alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsProviderInterface {
private:
    AudioPlayerEngineImpl(std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface);

    bool initialize(
        std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
        std::shared_ptr<alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface>
            audioPlayerObserverDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder);

public:
    static std::shared_ptr<AudioPlayerEngineImpl> create(
        std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface,
        std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::PlaybackRouterInterface> playbackRouter,
        std::shared_ptr<alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface>
            audioPlayerObserverDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> authDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder);

    //
    // AudioPlayerEngineInterface
    //
    int64_t onGetPlayerPosition() override;
    int64_t onGetPlayerDuration() override;

    //
    // AudioPlayerObserverInterface
    //
    void onPlayerActivityChanged(alexaClientSDK::avsCommon::avs::PlayerActivity state, const Context& context) override;

    /// @name RenderPlayerInfoCardsProviderInterface Functions
    /// @{
    void setObserver(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>
                         observer) override;
    /// @}

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::AudioPlayer> m_audioPlayerPlatformInterface = nullptr;
    std::shared_ptr<alexaClientSDK::acsdkAudioPlayer::AudioPlayer> m_audioPlayerCapabilityAgent;

    /// Observer for changes related to RenderPlayerInfoCards.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface>
        m_renderPlayerObserver;

    /// Mutex to serialize access to the observers.
    std::mutex m_observersMutex;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_AUDIO_PLAYER_ENGINE_IMPL_H
