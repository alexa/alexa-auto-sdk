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

#include <climits>
#include <exception>

#include "AACE/Engine/Alexa/AlexaMetrics.h"
#include "AACE/Engine/Alexa/DeviceSettingsDelegate.h"
#include "AACE/Engine/Alexa/SpeechRecognizerEngineImpl.h"
#include "AACE/Engine/Alexa/UPLService.h"
#include "AACE/Engine/Alexa/WakewordObservableInterface.h"
#include "AACE/Engine/Alexa/WakewordObserverInterface.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

/// The maximum number of readers of the stream.
static const size_t MAX_READERS = 10;

/// The amount of audio data to keep in the ring buffer.
static const std::chrono::seconds AMOUNT_OF_AUDIO_DATA_IN_BUFFER = std::chrono::seconds(15);

/// The amount of time for wake-word verification
static const std::chrono::milliseconds VERIFICATION_TIMEOUT = std::chrono::milliseconds(500);

// String to identify log entries originating from this file.
static const std::string TAG("aace.alexa.SpeechRecognizerEngineImpl");

SpeechRecognizerEngineImpl::SpeechRecognizerEngineImpl(
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizerPlatformInterface,
    const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_speechRecognizerPlatformInterface(speechRecognizerPlatformInterface),
        m_audioFormat(audioFormat),
        m_wordSize(audioFormat.sampleSizeInBits / CHAR_BIT),
        m_state(alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::IDLE) {
}

bool SpeechRecognizerEngineImpl::initialize(
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::UserInactivityMonitorInterface> userInactivityMonitor,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager,
    DeviceSettingsDelegate& deviceSettingsDelegate,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface> systemSoundPlayer,
    std::shared_ptr<alexaClientSDK::speechencoder::SpeechEncoder> speechEncoder,
    std::shared_ptr<aace::engine::alexa::WakewordEngineAdapter> wakewordEngineAdapter,
    std::shared_ptr<aace::engine::alexa::WakewordVerifier> wakewordVerifier) {
    try {
        // create the audio channel
        m_audioInputChannel = audioManager->openAudioInputChannel(
            "SpeechRecognizer", aace::engine::audio::AudioManagerInterface::AudioInputType::VOICE);
        ThrowIfNull(m_audioInputChannel, "invalidAudioInputChannel");

        // create the wakeword confirmation setting
        ThrowIfNot(
            deviceSettingsDelegate.configureWakeWordConfirmationSetting(), "createWakeWordConfirmationSettingFailed");

        // create the speech confirmation setting
        ThrowIfNot(
            deviceSettingsDelegate.configureSpeechConfirmationSetting(), "createSpeechConfirmationSettingFailed");

        std::shared_ptr<alexaClientSDK::settings::WakeWordsSetting> localAndWakeWordsSetting = nullptr;

        if (wakewordEngineAdapter != nullptr && assetsManager->getDefaultSupportedWakeWords().empty() == false) {
            ThrowIfNot(
                deviceSettingsDelegate.configureLocaleAndWakeWordsSettings(assetsManager),
                "createLocaleAndWakeWordsSettingFailed");
        } else {
            ThrowIfNot(deviceSettingsDelegate.configureLocaleSetting(assetsManager), "createLocaleSettingFailed");
        }

        m_audioInputProcessor = alexaClientSDK::capabilityAgents::aip::AudioInputProcessor::create(
            directiveSequencer,
            messageSender,
            contextManager,
            focusManager,
            dialogUXStateAggregator,
            exceptionSender,
            userInactivityMonitor,
            systemSoundPlayer,
            assetsManager,
            deviceSettingsDelegate.getWakeWordConfirmationSetting(),
            deviceSettingsDelegate.getSpeechConfirmationSetting(),
            deviceSettingsDelegate.getWakeWordsSetting(),
            speechEncoder);

        ThrowIfNull(m_audioInputProcessor, "couldNotCreateAudioInputProcessor");
        ThrowIfNot(initializeAudioInputStream(), "initializeAudioInputStreamFailed");

        // add dialog state observer to aip
        m_audioInputProcessor->addObserver(shared_from_this());
        m_audioInputProcessor->addObserver(dialogUXStateAggregator);
        m_dialogUXStateAggregator = dialogUXStateAggregator;

        // register capability with the default endpoint
        defaultEndpointBuilder->withCapability(m_audioInputProcessor, m_audioInputProcessor);

        m_wakewordEngineAdapter = wakewordEngineAdapter;
        if (m_wakewordEngineAdapter != nullptr) {
            ThrowIfNot(
                m_wakewordEngineAdapter->initialize(m_audioInputStream, m_audioFormat), "wakewordInitializeFailed");
            m_wakewordEngineAdapter->addKeyWordObserver(shared_from_this());
        }

        m_directiveSequencer = directiveSequencer;
        m_wakewordVerifier = wakewordVerifier;

        // get the initialize wakeword enabled state from the platform interface
        m_wakewordEnabled = isWakewordSupported() && m_initialWakewordEnabledState;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<SpeechRecognizerEngineImpl> SpeechRecognizerEngineImpl::create(
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizerPlatformInterface,
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
    const alexaClientSDK::avsCommon::utils::AudioFormat& audioFormat,
    std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::UserInactivityMonitorInterface> userInactivityMonitor,
    const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::LocaleAssetsManagerInterface>& assetsManager,
    DeviceSettingsDelegate& deviceSettingsDelegate,
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> connectionManager,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SystemSoundPlayerInterface> systemSoundPlayer,
    std::shared_ptr<alexaClientSDK::speechencoder::SpeechEncoder> speechEncoder,
    std::shared_ptr<aace::engine::alexa::WakewordEngineAdapter> wakewordEngineAdapter,
    std::shared_ptr<aace::engine::alexa::WakewordVerifier> wakewordVerifier) {
    std::shared_ptr<SpeechRecognizerEngineImpl> speechRecognizerEngineImpl = nullptr;

    try {
        ThrowIfNull(speechRecognizerPlatformInterface, "invlaidSpeechRecognizerPlatformInterface");
        ThrowIfNull(audioManager, "invalidAudioManager");
        ThrowIfNull(defaultEndpointBuilder, "invalidEndpointBuilder");
        ThrowIfNull(directiveSequencer, "invalidDirectiveSequencer");
        ThrowIfNull(capabilitiesDelegate, "invalidCapabilitiesDelegate");
        ThrowIfNull(messageSender, "invalidMessageSender");
        ThrowIfNull(focusManager, "invalidFocusManager");
        ThrowIfNull(contextManager, "invalidContextManager");
        ThrowIfNull(dialogUXStateAggregator, "invalidDialogUXStateAggregator");
        ThrowIfNull(exceptionSender, "invalidExceptionSender");
        ThrowIfNull(userInactivityMonitor, "invalidUserInactivityMonitor");
        ThrowIfNull(connectionManager, "invalidConnectionManager");
        ThrowIfNull(systemSoundPlayer, "invalidSystemSoundPlayer");

        speechRecognizerEngineImpl = std::shared_ptr<SpeechRecognizerEngineImpl>(
            new SpeechRecognizerEngineImpl(speechRecognizerPlatformInterface, audioFormat));

        ThrowIfNot(
            speechRecognizerEngineImpl->initialize(
                audioManager,
                defaultEndpointBuilder,
                directiveSequencer,
                messageSender,
                contextManager,
                focusManager,
                dialogUXStateAggregator,
                capabilitiesDelegate,
                exceptionSender,
                userInactivityMonitor,
                assetsManager,
                deviceSettingsDelegate,
                connectionManager,
                systemSoundPlayer,
                speechEncoder,
                wakewordEngineAdapter,
                wakewordVerifier),
            "initializeSpeechRecognizerEngineImplFailed");

        // set the platform engine interface reference
        speechRecognizerPlatformInterface->setEngineInterface(speechRecognizerEngineImpl);

        return speechRecognizerEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        if (speechRecognizerEngineImpl != nullptr) {
            speechRecognizerEngineImpl->shutdown();
        }
        return nullptr;
    }
}

void SpeechRecognizerEngineImpl::doShutdown() {
    m_executor.shutdown();

    if (m_audioInputWriter != nullptr) {
        m_audioInputWriter->close();
    }

    if (m_audioInputProcessor != nullptr) {
        m_audioInputProcessor->removeObserver(shared_from_this());
        m_audioInputProcessor->removeObserver(m_dialogUXStateAggregator);
        m_audioInputProcessor->shutdown();
    }

    if (m_wakewordEngineAdapter != nullptr) {
        m_wakewordEngineAdapter->removeKeyWordObserver(shared_from_this());
    }

    if (m_speechRecognizerPlatformInterface != nullptr) {
        m_speechRecognizerPlatformInterface->setEngineInterface(nullptr);
    }

    if (m_audioInputChannel != nullptr) {
        m_audioInputChannel->doShutdown();
        m_audioInputChannel.reset();
    }
}

bool SpeechRecognizerEngineImpl::initializeAudioInputStream() {
    try {
        size_t size = alexaClientSDK::avsCommon::avs::AudioInputStream::calculateBufferSize(
            m_audioFormat.sampleRateHz * AMOUNT_OF_AUDIO_DATA_IN_BUFFER.count(), m_wordSize, MAX_READERS);
        auto buffer = std::make_shared<alexaClientSDK::avsCommon::avs::AudioInputStream::Buffer>(size);
        ThrowIfNull(buffer, "couldNotCreateAudioInputBuffer");

        // create the audio input stream
        m_audioInputStream = alexaClientSDK::avsCommon::avs::AudioInputStream::create(buffer, m_wordSize, MAX_READERS);
        ThrowIfNull(m_audioInputStream, "couldNotCreateAudioInputStream");

        // create the audio input writer
        m_audioInputWriter = m_audioInputStream->createWriter(
            alexaClientSDK::avsCommon::avs::AudioInputStream::Writer::Policy::NONBLOCKABLE);
        ThrowIfNull(m_audioInputWriter, "couldNotCreateAudioInputWriter");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initializeAudioInputStream").d("reason", ex.what()));
        m_audioInputStream.reset();
        m_audioInputWriter.reset();
        return false;
    }
}

bool SpeechRecognizerEngineImpl::waitForExpectingAudioState(bool state, const std::chrono::seconds duration) {
    std::unique_lock<std::mutex> lock(m_expectingAudioMutex);

    return m_expectingAudioState_cv.wait_for(lock, duration, [this, state]() { return state == isExpectingAudio(); });
}

bool SpeechRecognizerEngineImpl::startAudioInput() {
    try {
        // if we are already expecting audio then don't attempt to start the audio
        if (isExpectingAudio()) {
            AACE_WARN(LX(TAG, "startAudioInput").d("reason", "alreadyExpectingAudio"));
            return true;
        }

        // notify the platform that we are expecting audio... if the platform returns
        // and error then we reset the expecting audio state and throw an exception
        std::weak_ptr<SpeechRecognizerEngineImpl> wp = shared_from_this();

        m_currentChannelId = m_audioInputChannel->start([wp](const int16_t* data, const size_t size) {
            if (auto sp = wp.lock()) {
                sp->write(data, size);
            } else {
                AACE_ERROR(LX(TAG, "startAudioInput").d("reason", "invalidWeakPtrReference"));
            }
        });

        // throw an exception if we failed to start the audio input channel
        ThrowIf(
            m_currentChannelId == aace::engine::audio::AudioInputChannelInterface::INVALID_CHANNEL,
            "audioInputChannelStartFailed");

        // notify mutext that the expecting audio state has changed
        m_expectingAudioState_cv.notify_all();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "startAudioInput").d("reason", ex.what()).d("id", m_currentChannelId));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::stopAudioInput() {
    try {
        ThrowIf(
            m_currentChannelId == aace::engine::audio::AudioInputChannelInterface::INVALID_CHANNEL,
            "invalidAudioChannelId");
        ThrowIfNot(m_audioInputChannel->stop(m_currentChannelId), "audioInputChannelStopFailed");

        // notify mutext that the expecting audio state has changed
        m_expectingAudioState_cv.notify_all();

        // reset the channel id
        m_currentChannelId = aace::engine::audio::AudioInputChannelInterface::INVALID_CHANNEL;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "stopAudioInput").d("reason", ex.what()).d("id", m_currentChannelId));
        m_currentChannelId = aace::engine::audio::AudioInputChannelInterface::INVALID_CHANNEL;
        return false;
    }
}

bool SpeechRecognizerEngineImpl::isExpectingAudio() {
    return m_currentChannelId != aace::engine::audio::AudioInputChannelInterface::INVALID_CHANNEL;
}

// SpeechRecognizer
bool SpeechRecognizerEngineImpl::onStartCapture(
    Initiator initiator,
    uint64_t keywordBegin,
    uint64_t keywordEnd,
    const std::string& keyword) {
    if (m_connectionStatus != aace::alexa::AlexaClient::ConnectionStatus::CONNECTED) {
        AACE_WARN(LX(TAG, "onStartCapture").d("reason", "AlexaClient is not connected"));
        return false;
    }

    try {
        ThrowIf(
            m_state ==
                alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::RECOGNIZING,
            "alreadyRecognizing");

        // create a new audio provider for the specified initiator type
        std::shared_ptr<alexaClientSDK::capabilityAgents::aip::AudioProvider> audioProvider;

        switch (initiator) {
            case Initiator::HOLD_TO_TALK:
                audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>(
                    m_audioInputStream,
                    m_audioFormat,
                    alexaClientSDK::capabilityAgents::aip::ASRProfile::CLOSE_TALK,
                    false,
                    true,
                    false);
                break;

            case Initiator::TAP_TO_TALK:
                audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>(
                    m_audioInputStream,
                    m_audioFormat,
                    alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD,
                    true,
                    true,
                    true);
                break;

            case Initiator::WAKEWORD:
                ThrowIf(keywordBegin == SpeechRecognizerEngineInterface::UNSPECIFIED_INDEX, "invalidKeywordBeginIndex");
                ThrowIf(keywordEnd == SpeechRecognizerEngineInterface::UNSPECIFIED_INDEX, "invalidKeywordEndIndex");
                ThrowIf(keyword.empty(), "invalidKeyword");
                audioProvider = std::make_shared<alexaClientSDK::capabilityAgents::aip::AudioProvider>(
                    m_audioInputStream,
                    m_audioFormat,
                    alexaClientSDK::capabilityAgents::aip::ASRProfile::NEAR_FIELD,
                    true,
                    false,
                    true);
                break;
        }

        // start the recognize event
        ThrowIfNot(
            startCapture(
                audioProvider,
                static_cast<alexaClientSDK::capabilityAgents::aip::Initiator>(initiator),
                keywordBegin,
                keywordEnd,
                keyword),
            "startCaptureFailed");

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onStartCapture")
                       .d("reason", ex.what())
                       .d("initiator", initiator)
                       .d("state", m_state)
                       .d("id", m_currentChannelId));
        return false;
    }
}

bool SpeechRecognizerEngineImpl::onStopCapture() {
    try {
        ThrowIfNot(m_audioInputProcessor->stopCapture().get(), "stopCaptureFailed");
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "onStopCapture").d("reason", ex.what()).d("id", m_currentChannelId));
        return false;
    }
}

ssize_t SpeechRecognizerEngineImpl::write(const int16_t* data, const size_t size) {
    try {
        ThrowIfNot(waitForExpectingAudioState(true), "audioNotExpected");
        ThrowIfNull(m_audioInputWriter, "nullAudioInputWriter");

        ssize_t result = m_audioInputWriter->write(data, size);
        ThrowIf(result < 0, "errorWritingData");

        return result;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "write").d("reason", ex.what()).d("id", m_currentChannelId));
        return -1;
    }
}

void SpeechRecognizerEngineImpl::addObserver(std::shared_ptr<WakewordObserverInterface> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.insert(observer);
}

void SpeechRecognizerEngineImpl::removeObserver(std::shared_ptr<WakewordObserverInterface> observer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_observers.erase(observer);
}

void SpeechRecognizerEngineImpl::onKeyWordDetected(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AudioInputStream> stream,
    std::string keyword,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index beginIndex,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index endIndex,
    std::shared_ptr<const std::vector<char>> KWDMetadata) {
    if (m_state == AudioInputProcessorObserverInterface::State::IDLE &&
        m_speechRecognizerPlatformInterface->wakewordDetected(keyword)) {
        m_executor.submit([this, beginIndex, endIndex, keyword] {
            if (m_wakewordVerifier && m_wakewordVerifier->verify(keyword, VERIFICATION_TIMEOUT)) {
                AACE_INFO(LX(TAG, "onKeyWordDetected: Cancelled by Wakeword Verifier"));
                return;
            }

            // notify observers about the wakeword detected
            std::lock_guard<std::mutex> lock(m_mutex);

            for (const auto& next : m_observers) {
                next->wakewordDetected(keyword);
            }

            onStartCapture(Initiator::WAKEWORD, beginIndex, endIndex, keyword);
        });
    }
}

void SpeechRecognizerEngineImpl::onConnectionStatusChanged(
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::Status status,
    const alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface::ChangedReason reason) {
    m_connectionStatus = static_cast<aace::alexa::AlexaClient::ConnectionStatus>(status);
}

bool SpeechRecognizerEngineImpl::startCapture(
    std::shared_ptr<alexaClientSDK::capabilityAgents::aip::AudioProvider> audioProvider,
    alexaClientSDK::capabilityAgents::aip::Initiator initiator,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index begin,
    alexaClientSDK::avsCommon::avs::AudioInputStream::Index keywordEnd,
    const std::string& keyword) {
    try {
        // ask the aip to start recognizing input
        ALEXA_METRIC(LX(TAG, "startCapture"), aace::engine::alexa::AlexaMetrics::Location::SPEECH_START_CAPTURE);
        aace::engine::alexa::UPLService::getInstance()->updateDialogStateForId(
            aace::engine::alexa::UPLService::DialogState::START_CAPTURE,
            "",
            m_directiveSequencer->isDialogRequestOnline());

        ThrowIfNot(
            m_audioInputProcessor
                ->recognize(*audioProvider, initiator, std::chrono::steady_clock::now(), begin, keywordEnd, keyword)
                .get(),
            "recognizeFailed");

        // notify the platform that we are expecting audio... if the platform returns
        // and error then we reset the expecting audio state and throw an exception
        if (isExpectingAudio() == false) {
            ThrowIfNot(startAudioInput(), "startAudioInputFailed");
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "startCapture").d("reason", ex.what()).d("id", m_currentChannelId));
        m_audioInputProcessor->resetState();
        return false;
    }
}

bool SpeechRecognizerEngineImpl::isWakewordSupported() {
    return m_wakewordEngineAdapter != nullptr;
}

bool SpeechRecognizerEngineImpl::isWakewordEnabled() {
    return m_wakewordEnabled;
}

bool SpeechRecognizerEngineImpl::enableWakewordDetection() {
    try {
        // check to make sure wakeword is supported
        ThrowIfNot(isWakewordSupported(), "wakewordNotSupported");

        // check if wakeword detection is already enabled, and we are not in the initial
        // wakeword enable state (this is the first time we are enabling!)
        ReturnIf(m_wakewordEnabled && m_initialWakewordEnabledState == false, true);

        // enable the wakeword engine adapter
        ThrowIfNot(m_wakewordEngineAdapter->enable(), "enableFailed");

        // set the wakeword enabled and expecting audio flags to true
        m_wakewordEnabled = true;
        m_initialWakewordEnabledState = false;

        // tell the platform interface to start providing audio input
        ThrowIfNot(startAudioInput(), "startAudioInputFailed");

        return true;
    } catch (std::exception& ex) {
        //setExpectingAudioState( false );
        AACE_ERROR(LX(TAG, "enableWakewordDetection").d("reason", ex.what()));
        m_wakewordEnabled = false;
        return false;
    }
}

bool SpeechRecognizerEngineImpl::disableWakewordDetection() {
    bool success = true;

    try {
        // check if wakeword detection is already disabled
        ReturnIfNot(m_wakewordEnabled, true);

        // check to make sure wakeword is supported
        ThrowIfNot(isWakewordSupported(), "wakewordNotSupported");

        // tell the platform to stop providing audio input
        ThrowIfNot(stopAudioInput(), "stopAudioInputFailed");
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "disableWakewordDetection").d("reason", ex.what()));
        success = false;
    }

    m_wakewordEngineAdapter->disable();
    m_wakewordEnabled = false;

    //setExpectingAudioState( false );

    return success;
}

void SpeechRecognizerEngineImpl::onStateChanged(
    alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State state) {
    m_state = state;

    // state changed to BUSY means that either the StopCapture directive has been received
    // or the speech recognizer was stopped manually
    if (state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::BUSY) {
        ALEXA_METRIC(LX(TAG, "onStateChanged"), aace::engine::alexa::AlexaMetrics::Location::SPEECH_STOP_CAPTURE);
        ALEXA_METRIC(
            LX(TAG, "stopCapture").d("dialogRequestId", m_directiveSequencer->getDialogRequestId()),
            aace::engine::alexa::AlexaMetrics::Location::SPEECH_STOP_CAPTURE);

        aace::engine::alexa::UPLService::getInstance()->updateDialogStateForId(
            aace::engine::alexa::UPLService::DialogState::STOP_CAPTURE,
            m_directiveSequencer->getDialogRequestId(),
            m_directiveSequencer->isDialogRequestOnline());

        if (isExpectingAudio()) {
            ALEXA_METRIC(LX(TAG, "onStateChanged"), aace::engine::alexa::AlexaMetrics::Location::END_OF_SPEECH);
            m_speechRecognizerPlatformInterface->endOfSpeechDetected();

            if (m_wakewordEnabled == false) {
                if (stopAudioInput() == false) {
                    AACE_ERROR(LX(TAG, "handleAudioStateChanged").d("reason", "stopAudioInputFailed"));
                }

                //setExpectingAudioState( false );
            }
        }
    } else if (state == alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::IDLE) {
        if (isExpectingAudio() && m_wakewordEnabled == false) {
            if (stopAudioInput() == false) {
                AACE_ERROR(LX(TAG, "handleAudioStateChanged").d("reason", "stopAudioInputFailed"));
            }

            //setExpectingAudioState( false );
        }
    } else if (
        state ==
        alexaClientSDK::avsCommon::sdkInterfaces::AudioInputProcessorObserverInterface::State::EXPECTING_SPEECH) {
        // let the recognizer know we are expecting audio from the platform interface
        //setExpectingAudioState( true );

        // call platform interface if we are starting the recognizer and the
        // wakeword engine was not already enabled...
        if (m_wakewordEnabled == false) {
            ThrowIfNot(startAudioInput(), "startAudioInputFailed");
        }
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
