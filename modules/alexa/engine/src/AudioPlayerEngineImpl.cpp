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

#include "AACE/Engine/Alexa/AudioPlayerEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include "AACE/Engine/Utils/Metrics/Metrics.h"
#include <AVSCommon/Utils/MediaPlayer/PooledMediaPlayerFactory.h>
#include <AVSCommon/Utils/MediaPlayer/PooledMediaResourceProvider.h>

namespace aace {
namespace engine {
namespace alexa {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.AudioPlayerEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "AudioPlayerEngineImpl";

/// Counter metrics for AudioPlayer Platform APIs
static const std::string METRIC_AUDIO_PLAYER_PLAYER_ACTIVITY_CHANGED = "PlayerActivityChanged";
static const std::string METRIC_AUDIO_PLAYER_GET_PLAYER_POSITION = "GetPlayerPosition";
static const std::string METRIC_AUDIO_PLAYER_GET_PLAYER_DURATION = "GetPlayerDuration";

AudioPlayerEngineImpl::AudioPlayerEngineImpl(std::shared_ptr<aace::alexa::AudioPlayer> audioPlayerPlatformInterface) :
        AudioChannelEngineImpl(
            alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME,
            "AudioPlayer"),
        m_audioPlayerPlatformInterface(audioPlayerPlatformInterface) {
}

bool AudioPlayerEngineImpl::initialize(
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
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    try {
        ThrowIfNot(
            initializeAudioChannel(audioOutputChannel, speakerManager, authDelegate), "initializeAudioChannelFailed");
        // temporary
        std::vector<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface>>
            audioChannelVolumeInterfaces{getChannelVolumeInterface()};
        auto provider = alexaClientSDK::avsCommon::utils::mediaPlayer::PooledMediaResourceProvider::
            createPooledMediaResourceProviderInterface({shared_from_this()}, audioChannelVolumeInterfaces);

        // create the capability agent
        m_audioPlayerCapabilityAgent = alexaClientSDK::acsdkAudioPlayer::AudioPlayer::create(
            std::move(provider),
            messageSender,
            focusManager,
            contextManager,
            exceptionSender,
            playbackRouter,
            nullptr,
            metricRecorder);
        ThrowIfNull(m_audioPlayerCapabilityAgent, "couldNotCreateCapabilityAgent");

        // add audio player observers
        m_audioPlayerCapabilityAgent->addObserver(
            std::dynamic_pointer_cast<alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface>(
                shared_from_this()));
        m_audioPlayerCapabilityAgent->addObserver(audioPlayerObserverDelegate);

        // register capability with the default endpoint
        capabilitiesRegistrar->withCapability(m_audioPlayerCapabilityAgent, m_audioPlayerCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<AudioPlayerEngineImpl> AudioPlayerEngineImpl::create(
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
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    std::shared_ptr<AudioPlayerEngineImpl> audioPlayerEngineImpl = nullptr;

    try {
        ThrowIfNull(audioPlayerPlatformInterface, "invalidAudioPlayerPlatformInterface");
        ThrowIfNull(audioManager, "invalidAudioManager");
        ThrowIfNull(capabilitiesRegistrar, "invalidCapabilitiesRegistrar");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(focusManager, "invalidFocusManager");
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(attachmentManager, "invalidAttachmentManager");
        ThrowIfNull(speakerManager, "invalidSpeakerManager");
        ThrowIfNull(exceptionSender, "invalidExceptionSender");
        ThrowIfNull(playbackRouter, "invalidPlaybackRouter");
        ThrowIfNull(audioPlayerObserverDelegate, "invalidAudioPlayerObserverInterfaceDelegate");
        ThrowIfNull(authDelegate, "invalidAuthDelegate");
        ThrowIfNull(metricRecorder, "invalidMetricRecorder");

        // open the Music audio channel
        auto audioOutputChannel = audioManager->openAudioOutputChannel(
            "AudioPlayer", aace::audio::AudioOutputProvider::AudioOutputType::MUSIC);
        ThrowIfNull(audioOutputChannel, "openAudioOutputChannelFailed");

        audioPlayerEngineImpl =
            std::shared_ptr<AudioPlayerEngineImpl>(new AudioPlayerEngineImpl(audioPlayerPlatformInterface));

        ThrowIfNot(
            audioPlayerEngineImpl->initialize(
                audioOutputChannel,
                capabilitiesRegistrar,
                messageSender,
                focusManager,
                contextManager,
                attachmentManager,
                speakerManager,
                exceptionSender,
                playbackRouter,
                audioPlayerObserverDelegate,
                authDelegate,
                metricRecorder),
            "initializeAudioPlayerEngineImplFailed");

        // set the platform's engine interface reference
        audioPlayerPlatformInterface->setEngineInterface(audioPlayerEngineImpl);

        return audioPlayerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (audioPlayerEngineImpl != nullptr) {
            audioPlayerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void AudioPlayerEngineImpl::doShutdown() {
    if (m_audioPlayerCapabilityAgent != nullptr) {
        m_audioPlayerCapabilityAgent->removeObserver(
            std::dynamic_pointer_cast<alexaClientSDK::acsdkAudioPlayerInterfaces::AudioPlayerObserverInterface>(
                shared_from_this()));
        m_audioPlayerCapabilityAgent->shutdown();
        m_audioPlayerCapabilityAgent.reset();
    }

    AudioChannelEngineImpl::doShutdown();

    if (m_audioPlayerPlatformInterface != nullptr) {
        m_audioPlayerPlatformInterface->setEngineInterface(nullptr);
        m_audioPlayerPlatformInterface.reset();
    }
}

//
// AudioPlayerEngineInterface
//
int64_t AudioPlayerEngineImpl::onGetPlayerPosition() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onGetPlayerPosition", {METRIC_AUDIO_PLAYER_GET_PLAYER_POSITION});
    return getMediaPosition();
}

int64_t AudioPlayerEngineImpl::onGetPlayerDuration() {
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onGetPlayerDuration", {METRIC_AUDIO_PLAYER_GET_PLAYER_DURATION});
    return getMediaDuration();
}

//
// AudioPlayerObserverInterface
//
void AudioPlayerEngineImpl::onPlayerActivityChanged(
    alexaClientSDK::avsCommon::avs::PlayerActivity state,
    const Context& context) {
    std::stringstream activityState;
    activityState << static_cast<aace::alexa::AudioPlayer::PlayerActivity>(state);
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onPlayerActivityChanged",
        {METRIC_AUDIO_PLAYER_PLAYER_ACTIVITY_CHANGED, activityState.str()});
    m_audioPlayerPlatformInterface->playerActivityChanged(static_cast<aace::alexa::AudioPlayer::PlayerActivity>(state));
}

void AudioPlayerEngineImpl::setObserver(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::RenderPlayerInfoCardsObserverInterface> observer) {
    std::lock_guard<std::mutex> lock{m_observersMutex};
    m_audioPlayerCapabilityAgent->setObserver(observer);
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
