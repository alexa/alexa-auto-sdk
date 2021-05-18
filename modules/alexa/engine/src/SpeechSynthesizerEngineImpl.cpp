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

#include "AACE/Engine/Alexa/SpeechSynthesizerEngineImpl.h"
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
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    try {
        ThrowIfNot(initializeAudioChannel(audioOutputChannel, speakerManager), "initializeAudioChannelFailed");

        m_speechSynthesizerCapabilityAgent =
            alexaClientSDK::capabilityAgents::speechSynthesizer::SpeechSynthesizer::create(
                shared_from_this(),
                messageSender,
                focusManager,
                contextManager,
                exceptionSender,
                metricRecorder,
                dialogUXStateAggregator,
                nullptr,
                nullptr);
        ThrowIfNull(m_speechSynthesizerCapabilityAgent, "couldNotCreateCapabilityAgent");

        // add dialog state observer
        m_speechSynthesizerCapabilityAgent->addObserver(dialogUXStateAggregator);

        // register capability with the default endpoint
        capabilitiesRegistrar->withCapability(m_speechSynthesizerCapabilityAgent, m_speechSynthesizerCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<SpeechSynthesizerEngineImpl> SpeechSynthesizerEngineImpl::create(
    std::shared_ptr<aace::alexa::SpeechSynthesizer> speechSynthesizerPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface> attachmentManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::utils::metrics::MetricRecorderInterface> metricRecorder) {
    std::shared_ptr<SpeechSynthesizerEngineImpl> speechSynthesizerEngineImpl = nullptr;

    try {
        ThrowIfNull(speechSynthesizerPlatformInterface, "invalidSpeechSynthesizerPlatformInterface");
        ThrowIfNull(capabilitiesRegistrar, "invalidCapabilitiesRegistrar");
        ThrowIfNull(audioManager, "invalidAudioManager");
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
                capabilitiesRegistrar,
                messageSender,
                focusManager,
                contextManager,
                attachmentManager,
                dialogUXStateAggregator,
                speakerManager,
                exceptionSender,
                metricRecorder),
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

}  // namespace alexa
}  // namespace engine
}  // namespace aace
