/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "RequestProcessor.h"
#include <memory>

#include <AACE/Alexa/AlexaConfiguration.h>
#include <AACE/Storage/StorageConfiguration.h>

#include <aasb/Consts.h>
#include "MediaPlayerHandler.h"
#include "PlatformSpecificLoggingMacros.h"
#include "PlatformAudioSpeechRecognizer.h"
#include "SpeechRecognizerHandler.h"
#include "SpeakerHandler.h"

// {@ Workaround: Load phone call control and navigation engine services
// Explicitly registering these services because other wise
// their engine libraries are not getting loaded.
// TODO: Find a better way to fix this.
#include <AACE/Engine/PhoneCallController/PhoneCallControllerEngineService.h>
#include <AACE/Engine/Navigation/NavigationEngineService.h>
#include <AACE/Engine/CBL/CBLEngineService.h>
REGISTER_SERVICE(aace::engine::alexa::AlexaEngineService)
REGISTER_SERVICE(aace::engine::phoneCallController::PhoneCallControllerEngineService)
REGISTER_SERVICE(aace::engine::navigation::NavigationEngineService)
REGISTER_SERVICE(aace::engine::cbl::CBLEngineService)
#ifdef ENABLE_AAC_WAKEWORD_ENGINE
#include <AACE/Engine/AmazonLiteEngineService.h>
REGISTER_SERVICE(aace::engine::amazonLite::AmazonLiteEngineService)
#endif
// @} End Workaround.

using AlexaConfiguration = aace::alexa::config::AlexaConfiguration;
using AlexaConnectionStatus = aace::alexa::AlexaClient::ConnectionStatus;

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace bridge {

// TAG to use for Logging.
const std::string TAG = "aasb::alexa::RequestProcessor";

// Input channel name.
const std::string INPUT_CHANNEL_NAME = "Input";

const std::string RequestProcessor::NOT_INITALIZED_ERROR = "NOT_INITIALIZED";

RequestProcessor::RequestProcessor() : RequestProcessor(aace::core::Engine::create()) {
}

RequestProcessor::RequestProcessor(std::shared_ptr<aace::core::Engine> engine) :
        m_engine{engine},
        m_directiveDispatcher{std::make_shared<DirectiveDispatcher>()},
        m_aacePlatformSpeechRecognizer{false} {
    if (m_engine != NULL) {
        AASB_INFO("Alexa Auto Core Engine created");
    } else {
        AASB_ERROR("Failed to create Alexa Auto Core Engine.");
    }
}

std::shared_ptr<aasb::bridge::RequestProcessor> RequestProcessor::create() {
    return std::shared_ptr<RequestProcessor>(new RequestProcessor());
}

bool RequestProcessor::init(std::shared_ptr<IConfigurationProvider> configProvider) {
    if (!configProvider) {
        AASB_ERROR("Invalid configuration provider.");
        return false;
    }

    m_config = configProvider;

    // Configure the Engine
    if (!configureEngine()) {
        AASB_ERROR("Failed to configure Alexa Auto Core Engine");
        return false;
    }

    return true;
}

bool RequestProcessor::start() {
    if (!m_engine->registerPlatformInterface(m_logger = aasb::core::logger::LoggerHandler::create())) {
        AASB_ERROR("Failed to register logger handler");
        return false;
    }

    if (!m_engine->registerPlatformInterface(
            m_networkInfoProviderHandler = aasb::core::network::NetworkInfoProviderHandler::create(m_logger))) {
        AASB_ERROR("Failed to register network info provider handler");
        return false;
    }

    if (!m_engine->registerPlatformInterface(
            m_authProviderHandler = aasb::alexa::AuthProviderHandler::create(m_logger))) {
        AASB_ERROR("Failed to register auth provider handler");
        return false;
    }

    if (!m_engine->registerPlatformInterface(
            m_alexaClientHandler = aasb::alexa::AlexaClientHandler::create(m_logger, m_directiveDispatcher))) {
        AASB_ERROR("Failed to register alexa client handler");
        return false;
    }

    auto audioConfig = m_config->getAudioIOConfig();
#ifdef ENABLE_AAC_PLATFORM_AUDIO
    m_AudioManager = aace::audio::AudioManager::create(audioConfig.platformData);
#endif

    if (!registerAudioPlayerCapability(audioConfig)) {
        return false;
    }

    if (!registerSpeechRecognizerCapability(audioConfig)) {
        return false;
    }

    if (!registerSpeechSynthesizerCapability(audioConfig)) {
        return false;
    }

    if (!m_engine->registerPlatformInterface(
            m_templateRuntimeHandler = aasb::alexa::TemplateRuntimeHandler::create(m_logger, m_directiveDispatcher))) {
        AASB_ERROR("Failed to register template runtime handler");
        return false;
    }

    if (!m_engine->registerPlatformInterface(
            m_playbackControllerHandler = aasb::alexa::PlaybackControllerHandler::create(m_logger))) {
        AASB_ERROR("Failed to register playback control handler");
        return false;
    }

    if (m_config->shouldEnableCBL()) {
        if (!m_engine->registerPlatformInterface(
                m_CBLHandler = aasb::cbl::CBLHandler::create(m_logger, m_directiveDispatcher))) {
            AASB_ERROR("Failed to register CBL handler");
            return false;
        }
    }

    if (m_config->shouldEnablePhoneCallControl()) {
        if (!m_engine->registerPlatformInterface(
                m_phoneCallControllerHandler = aasb::phoneCallController::PhoneCallControllerHandler::create
                    (m_logger, m_directiveDispatcher))) {
            AASB_ERROR("Failed to register phonecall control handler");
            return false;
        }
    }

    if (m_config->shouldEnableNavigation()) {
        if (!m_engine->registerPlatformInterface(
                m_navigationHandler = aasb::navigation::NavigationHandler::create
                    (m_logger, m_directiveDispatcher))) {
            AASB_ERROR("Failed to register navigation handler");
            return false;
        }

        if (!m_engine->registerPlatformInterface(
                m_locationProviderHandler = aasb::location::LocationProviderHandler::create
                    (m_logger, m_config, m_directiveDispatcher))) {
            AASB_ERROR("Failed to register location provider handler");
            return false;
        }
    }

    if (!m_engine->start()) {
        AASB_ERROR("Failed to start Alexa Auto Core Engine");
        return false;
    }

    AASB_ERROR("Alexa Auto Core Engine started.");
    return true;
}

void RequestProcessor::setAVSEndPoint(const std::string& avsEndPoint) {
    if (!m_engine) {
        AASB_ERROR("Failed to set AVS Endpoint. Engine not created yet.");
        return;
    }

    m_engine->setProperty(aace::alexa::property::AVS_ENDPOINT, avsEndPoint);
}

void RequestProcessor::setLocale(const std::string& locale) {
    if (!m_engine) {
        AASB_ERROR("Failed to set locale. Engine not created yet.");
        return;
    }

    m_engine->setProperty(aace::alexa::property::LOCALE, locale);
}

void RequestProcessor::setFirmwareVersion(const std::string& firmwareVersion) {
    if (!m_engine) {
        AASB_ERROR("Failed to set device firmware version. Engine not created yet.");
        return;
    }

    m_engine->setProperty(aace::alexa::property::FIRMWARE_VERSION, firmwareVersion);
}

bool RequestProcessor::startListening(std::string mode) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__);

    if (!m_engine || !m_speechRecognizer) {
        m_logger->log(
            Level::ERROR, TAG, "Failed to start listening to audio input: Invalid engine or speech recognizer");
        return false;
    }

    bool result = false;

    if (mode == "END_OF_SPEECH_NEEDED")
        result = m_speechRecognizer->tapToTalk();
    else if (mode == "END_OF_SPEECH_NOT_NEEDED")
        result = m_speechRecognizer->holdToTalk();
    else
        m_logger->log(Level::ERROR, TAG, "Invalid listening mode");

    return result;
}

ssize_t RequestProcessor::writeAudioSamples(const int16_t* data, const size_t size) {
    if (!m_engine || !m_speechRecognizer) {
        AASB_ERROR("Failed to write audio input.");
        return false;
    }

    if (m_aacePlatformSpeechRecognizer) {
        AASB_ERROR("writeAudioSamples cant be executed on AACE Platform speech recognizer.");
        return false;
    }

    auto speechRecognizer = std::static_pointer_cast<aasb::alexa::SpeechRecognizerHandler>(m_speechRecognizer);

    return speechRecognizer->write(data, size);
}

std::string RequestProcessor::getAuthState() {
    if (m_alexaClientHandler) return m_alexaClientHandler->getAuthState();
    return NOT_INITALIZED_ERROR;
}

std::string RequestProcessor::getDialogState() {
    if (m_alexaClientHandler) return m_alexaClientHandler->getDialogState();
    return NOT_INITALIZED_ERROR;
}

std::string RequestProcessor::getConnectionStatus() {
    if (m_alexaClientHandler) return m_alexaClientHandler->getConnectionStatus();
    return NOT_INITALIZED_ERROR;
}

bool RequestProcessor::configureEngine() {
    if (!m_engine) {
        AASB_ERROR("Failed to configure engine. Engine not created yet.");
        return false;
    }

    std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> engineConfigurations;

    engineConfigurations.push_back(AlexaConfiguration::createDeviceInfoConfig(
        m_config->getProductDSN(), m_config->getClientId(), m_config->getProductId()));

    engineConfigurations.push_back(AlexaConfiguration::createCurlConfig(m_config->getCertificatesDirectoryPath()));

    std::string appDataDir = m_config->getAppsDataDirectory();
    std::string localStoragePath = appDataDir + "/localStorage.sqlite";
    engineConfigurations.push_back(
        aace::storage::config::StorageConfiguration::createLocalStorageConfig(localStoragePath));

    std::string miscStorageDbPath = appDataDir + "/miscStorage.db";
    engineConfigurations.push_back(AlexaConfiguration::createMiscStorageConfig(miscStorageDbPath));

    std::string alexaCertSenderPath = appDataDir + "/alexa-certifiedSender.db";
    engineConfigurations.push_back(AlexaConfiguration::createCertifiedSenderConfig(alexaCertSenderPath));

    std::string alertsDbPath = appDataDir + "/alexa-alerts.db";
    engineConfigurations.push_back(AlexaConfiguration::createAlertsConfig(alertsDbPath));

    std::string settingsDbPath = appDataDir + "/settings.sqlite";
    engineConfigurations.push_back(AlexaConfiguration::createSettingsConfig(settingsDbPath));

    std::string notificationsDbPath = appDataDir + "/alexa-notifications.db";
    engineConfigurations.push_back(AlexaConfiguration::createNotificationsConfig(notificationsDbPath));

    return m_engine->configure(engineConfigurations);
}

bool RequestProcessor::registerCapabilityDirectiveListener(
    std::shared_ptr<aasb::bridge::IAlexaCapabilityDirectiveListener> listener) {
    m_directiveDispatcher->registerCapabilityDirectiveListener(listener);
    return true;
}

void RequestProcessor::onReceivedEvent(
    const std::string& topic,
    const std::string& action,
    const std::string& payload) {
    if (topic == TOPIC_AUTH_PROVIDER) {
        m_authProviderHandler->onReceivedEvent(action, payload);
        return;
    }

    if (topic == TOPIC_SPEECH_RECOGNIZER) {
        // m_speechRecognizerHandler->onReceivedEvent( action, payload );
        return;
    }

    if (topic == TOPIC_SPEECH_SYNTHESIZER) {
        m_speechSynthesizerHandler->onReceivedEvent(action, payload);
        return;
    }

    if (topic == TOPIC_AUDIO_PLAYER) {
        m_audioPlayerHandler->onReceivedEvent(action, payload);
        return;
    }

    if (topic == TOPIC_PHONECALL_CONTROLLER) {
        if (m_phoneCallControllerHandler) {
            m_phoneCallControllerHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "Phone call controller is not enabled.");
        }
        return;
    }

    if (topic == TOPIC_PLAYBACK_CONTROLLER) {
        m_playbackControllerHandler->onReceivedEvent(action, payload);
        return;
    }

    if (topic == TOPIC_CBL) {
        m_logger->log(Level::INFO, TAG, "Received CBL Event: " + action + " payload: " + payload);
        m_CBLHandler->onReceivedEvent(action, payload);
        return;
    }

    AASB_ERROR("UKNOWN TOPIC %s", topic.c_str());
}

bool RequestProcessor::registerAudioPlayerCapability(const IConfigurationProvider::AudioIOConfiguration& audioConfig) {
    std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> speaker;

    if (audioConfig.usePlatformAudioIO) {
#ifdef ENABLE_AAC_PLATFORM_AUDIO
        auto outputChannel = m_AudioManager->openOutputChannel(TOPIC_AUDIO_PLAYER, audioConfig.audioPlayerDevice);
        mediaPlayer = outputChannel.mediaPlayer;
        speaker = outputChannel.speaker;
#else
        AASB_ERROR("AASB must be compiled with ENABLE_AAC_PLATFORM_AUDIO flag to use platform audio io");
        return false;
#endif
    } else {
        std::string mediaPlayerFile = m_config->getAppsDataDirectory() + "/alexa_audioplayer_media";
        mediaPlayer = aasb::alexa::MediaPlayerHandler::create(
            TOPIC_AUDIO_PLAYER, m_logger, m_directiveDispatcher, mediaPlayerFile);
        speaker = aasb::alexa::SpeakerHandler::create(m_logger);
    }
    if (!m_engine->registerPlatformInterface(
            m_audioPlayerHandler =
                aasb::alexa::AudioPlayerHandler::create(audioConfig.usePlatformAudioIO, mediaPlayer, speaker))) {
        AASB_ERROR("Failed to register audio player handler");
        return false;
    }

    return true;
}

bool RequestProcessor::registerSpeechSynthesizerCapability(
    const IConfigurationProvider::AudioIOConfiguration& audioConfig) {
    std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer;
    std::shared_ptr<aace::alexa::Speaker> speaker;

    if (audioConfig.usePlatformAudioIO) {
#ifdef ENABLE_AAC_PLATFORM_AUDIO
        auto outputChannel =
            m_AudioManager->openOutputChannel(TOPIC_SPEECH_SYNTHESIZER, audioConfig.speechSynthesizerDevice);
        mediaPlayer = outputChannel.mediaPlayer;
        speaker = outputChannel.speaker;
#else
        AASB_ERROR("AASB must be compiled with ENABLE_AAC_PLATFORM_AUDIO flag to use platform audio io");
        return false;
#endif
    } else {
        std::string mediaPlayerFile = m_config->getAppsDataDirectory() + "/alexa_speechsynth_media";
        mediaPlayer = aasb::alexa::MediaPlayerHandler::create(
            TOPIC_SPEECH_SYNTHESIZER, m_logger, m_directiveDispatcher, mediaPlayerFile);
        speaker = aasb::alexa::SpeakerHandler::create(m_logger);
    }

    if (!m_engine->registerPlatformInterface(
            m_speechSynthesizerHandler =
                aasb::alexa::SpeechSynthesizerHandler::create(audioConfig.usePlatformAudioIO, mediaPlayer, speaker))) {
        AASB_ERROR("Failed to register speech synthesizer handler");
        return false;
    }

    return true;
}

bool RequestProcessor::registerSpeechRecognizerCapability(
    const IConfigurationProvider::AudioIOConfiguration& audioConfig) {

    bool wakeWordEnabled = m_config->shouldEnableWakeword();
#ifndef ENABLE_AAC_WAKEWORD_ENGINE
    if (wakeWordEnabled) {
        wakeWordEnabled = false;
        AASB_WARNING("Cannot enable wakeword engine because AAC isnt compiled with Wakeword engine.");
    }
#endif
    std::shared_ptr<aace::alexa::SpeechRecognizer> speechRecognizer;
    if (audioConfig.usePlatformAudioIO) {
#ifdef ENABLE_AAC_PLATFORM_AUDIO
        auto inputChannel = m_AudioManager->openInputChannel(INPUT_CHANNEL_NAME, audioConfig.audioInputDevice);
        speechRecognizer =
            aasb::alexa::PlatformAudioSpeechRecognizer::create(wakeWordEnabled, m_logger, inputChannel.audioCapture);
#else
        AASB_ERROR("AASB must be compiled with ENABLE_AAC_PLATFORM_AUDIO flag to use platform audio io");
        return false;
#endif
        m_aacePlatformSpeechRecognizer = true;
    } else {
        speechRecognizer =
            aasb::alexa::SpeechRecognizerHandler::create(wakeWordEnabled, m_logger, m_directiveDispatcher);
        m_aacePlatformSpeechRecognizer = false;
    }

    if (!m_engine->registerPlatformInterface(m_speechRecognizer = speechRecognizer)) {
        AASB_ERROR("Failed to register speech recognizer handler");
        return false;
    }

    if (wakeWordEnabled) {
        if (!speechRecognizer->enableWakewordDetection()) {
            AASB_WARNING("Enabling wakeword failed");
        }
    }

    return true;
}

}  // namespace bridge
}  // namespace aasb
