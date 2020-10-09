/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Alexa/AlexaMetrics.h"
#include "AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h"
#include "AACE/Engine/Alexa/UPLService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.SpeechSynthesizerEngineImpl");

SpeechSynthesizerEngineImpl::SpeechSynthesizerEngineImpl(
    std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizerPlatformInterface) :
        AudioChannelEngineImpl(
            alexaClientSDK::avsCommon::sdkInterfaces::ChannelVolumeInterface::Type::AVS_SPEAKER_VOLUME,
            "SpeechSynthesizer"),
        m_speechSynthesizerPlatformInterface(speechSynthesizerPlatformInterface) {
}

bool SpeechSynthesizerEngineImpl::initialize(
    std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) {
    try {
        ThrowIfNot(initializeAudioChannel(audioOutputChannel, speakerManager), "initializeAudioChannelFailed");

        m_speechSynthesizerCapabilityAgent =
            alexaClientSDK::capabilityAgents::speechSynthesizer::SpeechSynthesizer::create(
                shared_from_this(),
                messageSender,
                focusManager,
                contextManager,
                exceptionSender,
                nullptr,
                dialogUXStateAggregator);
        ThrowIfNull(m_speechSynthesizerCapabilityAgent, "couldNotCreateCapabilityAgent");

        // add dialog state observer
        m_speechSynthesizerCapabilityAgent->addObserver(dialogUXStateAggregator);

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability(m_speechSynthesizerCapabilityAgent, m_speechSynthesizerCapabilityAgent);

        m_directiveSequencer = directiveSequencer;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<SpeechSynthesizerEngineImpl> SpeechSynthesizerEngineImpl::create(
    std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizerPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender) {
    std::shared_ptr<SpeechSynthesizerEngineImpl> speechSynthesizerEngineImpl = nullptr;

    try {
        ThrowIfNull(speechSynthesizerPlatformInterface, "invalidSpeechSynthesizerPlatformInterface");
        ThrowIfNull(defaultEndpointBuilder, "invalidEndpointBuilder");
        ThrowIfNull(audioManager, "invalidAudioManager");
        ThrowIfNull(directiveSequencer, "invalidDirectiveSequencer");
        ThrowIfNull(capabilitiesDelegate, "invalidCapabilitiesDelegate");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(focusManager, "invalidFocusManager");
        ThrowIfNull(speakerManager, "invalidSpeakerManager");
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(attachmentManager, "invalidAttachmentManager");
        ThrowIfNull(dialogUXStateAggregator, "invalidDialogUXStateAggregator");
        ThrowIfNull(exceptionSender, "invalidExceptionSender");

        // open the TTS audio channel
        auto audioOutputChannel = audioManager->openAudioOutputChannel(
            "SpeechSynthesizer", aace::audio::AudioOutputProvider::AudioOutputType::TTS);
        ThrowIfNull(audioOutputChannel, "openAudioOutputChannelFailed");

        // create the speech synthesizer impl
        speechSynthesizerEngineImpl = std::shared_ptr<SpeechSynthesizerEngineImpl>(
            new SpeechSynthesizerEngineImpl(speechSynthesizerPlatformInterface));

        // initialize the speech synthesizer
        ThrowIfNot(
            speechSynthesizerEngineImpl->initialize(
                audioOutputChannel,
                defaultEndpointBuilder,
                directiveSequencer,
                messageSender,
                focusManager,
                contextManager,
                attachmentManager,
                dialogUXStateAggregator,
                capabilitiesDelegate,
                speakerManager,
                exceptionSender),
            "initializeSpeechSynthesizerEngineImplFailed");

        return speechSynthesizerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (speechSynthesizerEngineImpl != nullptr) {
            speechSynthesizerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void SpeechSynthesizerEngineImpl::doShutdown() {
    AudioChannelEngineImpl::doShutdown();

    if (m_speechSynthesizerCapabilityAgent != nullptr) {
        m_speechSynthesizerCapabilityAgent->shutdown();
        m_speechSynthesizerCapabilityAgent.reset();
    }
}

void SpeechSynthesizerEngineImpl::handlePrePlaybackStarted(SourceId id) {
    try {
        auto m_directiveSequencer_lock = m_directiveSequencer.lock();
        ThrowIfNull(m_directiveSequencer_lock, "invalidDirectiveSequencer");

        ALEXA_METRIC(
            LX(TAG, "executePlaybackStarted").d("dialogrequestid", m_directiveSequencer_lock->getDialogRequestId()),
            aace::engine::alexa::AlexaMetrics::Location::PLAYBACK_STARTED);

        aace::engine::alexa::UPLService::getInstance()->updateDialogStateForId(
            aace::engine::alexa::UPLService::DialogState::PLAYBACK_STARTED,
            m_directiveSequencer_lock->getDialogRequestId(),
            m_directiveSequencer_lock->isDialogRequestOnline());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("id", id));
    }
}

void SpeechSynthesizerEngineImpl::handlePrePlaybackFinished(SourceId id) {
    try {
        auto m_directiveSequencer_lock = m_directiveSequencer.lock();
        ThrowIfNull(m_directiveSequencer_lock, "invalidDirectiveSequencer");

        ALEXA_METRIC(
            LX(TAG, "executePlaybackFinished").d("dialogrequestid", m_directiveSequencer_lock->getDialogRequestId()),
            aace::engine::alexa::AlexaMetrics::Location::PLAYBACK_FINISHED);

        aace::engine::alexa::UPLService::getInstance()->updateDialogStateForId(
            aace::engine::alexa::UPLService::DialogState::PLAYBACK_FINISHED,
            m_directiveSequencer_lock->getDialogRequestId(),
            m_directiveSequencer_lock->isDialogRequestOnline());
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()).d("id", id));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
