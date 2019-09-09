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
#ifndef AASB_AASBCONTROLLERIMPL_H
#define AASB_AASBCONTROLLERIMPL_H

#include <memory>

#include <AACE/Core/Engine.h>
#include <AACE/Core/EngineConfiguration.h>
#include <AACE/Alexa/AlexaProperties.h>
#include <AACE/Alexa/SpeechRecognizer.h>

#include <aasb/interfaces/IAlexaCapabilityDirectiveListener.h>
#include <aasb/interfaces/IConfigurationProvider.h>
#include <aasb/interfaces/IAASBController.h>

#include "AlexaClientHandler.h"
#include "AudioPlayerHandler.h"
#include "AuthProviderHandler.h"
#ifdef ENABLE_AAC_CAR_CONTROL
#include "CarControlHandler.h"
#endif // ENABLE_AAC_GLORIA
#include "CBLHandler.h"
#include "LocalMediaSourceHandlerManager.h"
#ifdef ENABLE_AAC_GLORIA
#include "CardRendererHandler.h"
#include "ListRendererHandler.h"
#endif // ENABLE_AAC_GLORIA
#include "LocationProviderHandler.h"
#include "LoggerHandler.h"
#include "NetworkInfoProviderHandler.h"
#include "SpeechSynthesizerHandler.h"
#include "TemplateRuntimeHandler.h"
#include "PhoneCallControllerHandler.h"
#include "PlaybackControllerHandler.h"
#include "NavigationHandler.h"

namespace aace {
namespace audio {
class AudioManager;
}
}  // namespace aace

namespace aasb {
namespace bridge {

/**
 * This class implements IAASBController forwarding those calls to @c RequestHandler.
 */
class AASBControllerImpl : public IAASBController {
public:
    /**
     * Destructor for @c AASBController.
     */
    ~AASBControllerImpl() = default;
    /// @name aasb::bridge::IAASBController Functions
    /// @{
    /**
     * Initializes the Alexa Auto Core Engine.
     *
     * @param configProvider An object to provide the neccessary configuration for AASB.
     */
    bool init(std::shared_ptr<IConfigurationProvider> configProvider) override;

    /**
     * Call @c start after @c init is successfully completed. This method will register the
     * capabilities and start the Alexa Auto Engine.
     *
     */
    bool start() override;

    /**
     * Optional method for client to set a different locale.
     *
     * @param locale New locale to set.
     */
    void setLocale(const std::string& locale) override;

    /**
     * Optional method for client to set the firmware version.
     *
     * @param firmwareVersion Firmware version to set.
     */
    void setFirmwareVersion(const std::string& firmwareVersion) override;

    /**
     * Start listening to user speech. Should be invoked in response to user's action such
     * as pressing "tap to talk" UI element (or physical button).
     *
     * @param mode Mode in which to start listening. Consult @c AASBControllerImpl
     *      documentation to find about different modes.
     */
    bool startListening(std::string mode) override;

    /**
     * Client should register an instance of @c IAlexaCapabilityDirectiveListener to receive callbacks
     * from AASB when an incoming directive arrives from Alexa.
     *
     * @param listener Listener which will receive callbacks on all the incoming
     *      directives that client of this library has to process.
     * @return true on success, false otherwise.
     */
    bool registerCapabilityDirectiveListener(std::shared_ptr<IAlexaCapabilityDirectiveListener> listener) override;

    /**
     * Sends the audio samples for Alexa processing. Client should continuously call this
     * method for the duration when they wish the user speech to be processed by Alexa.
     *
     * Client doesn't need to call this method if they have supplied the custom audio input
     * output provider by calling start with a valid @c IAudioInputOutputProvider
     *
     * @param data Buffer where audio samples are stored.
     * @param size Number of samples in the buffer.
     * @return Number of samples sent to Alexa for processing. It can return -ve number
     *      in case of error.
     */
    ssize_t writeAudioSamples(const int16_t* data, const size_t size) override;

    /**
     * Client should call this method when they want to send an event to Auto SDK.
     * Following are couple of examples:
     * 1) topic: TOPIC_AUTH_PROVIDER, action: ACTION_SET_AUTH_TOKEN, payload: "refresh-token"
     * 2) topic: TOPIC_AUDIO_PLAYER, action: ACTION_MEDIA_STATE_CHANGED, payload: "PLAYING"
     *
     * @param topic Which handler should handle the event.
     * @param action What is the event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    void onReceivedEvent(const std::string& topic, const std::string& action, const std::string& payload) override;

    /**
     * This is called by client to enable or disable WakeWord.
     *
     * @param wakeWordEnable -  true to enable or false to disable
     */
     bool setWakeWordEnable(bool wakeWordEnable) override;

    /**
     * set mock engine instance for testing (must be used only by test).
     * @param engine instance
     */
    void setMockEngine(std::shared_ptr<aace::core::Engine> engine) override;
    /// @}
private:
    /**
     * Constructs @c AASBControllerImpl.
     */
    AASBControllerImpl();

    /**
     * Friend for creating instances of AASBControllerImpl.
     */
    friend class AASBControllerFactory;

    // Config provider.
    std::shared_ptr<IConfigurationProvider> m_config;

    // Alexa Auto core engine
    std::shared_ptr<aace::core::Engine> m_engine;

    // Logger handler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // Alexa client handler
    std::shared_ptr<aasb::alexa::AlexaClientHandler> m_alexaClientHandler;

    // Audio player handler
    std::shared_ptr<aasb::alexa::AudioPlayerHandler> m_audioPlayerHandler;

    // Speech Recognizer
    std::shared_ptr<aace::alexa::SpeechRecognizer> m_speechRecognizer;

    // Speech Syntherizer handler
    std::shared_ptr<aasb::alexa::SpeechSynthesizerHandler> m_speechSynthesizerHandler;

    // Template Runtime handler
    std::shared_ptr<aasb::alexa::TemplateRuntimeHandler> m_templateRuntimeHandler;

    // Auth Provider handler
    std::shared_ptr<aasb::alexa::AuthProviderHandler> m_authProviderHandler;

    // LocalMediaSource handler manager
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandlerManager> m_LocalMediaSourceHandlerManager;

    // Handler for PCC (Phone call control)
    std::shared_ptr<aasb::phoneCallController::PhoneCallControllerHandler> m_phoneCallControllerHandler;

    // Handler for Playback Control
    std::shared_ptr<aasb::alexa::PlaybackControllerHandler> m_playbackControllerHandler;

    // Handler for Navigation
    std::shared_ptr<aasb::navigation::NavigationHandler> m_navigationHandler;

    // CBL handler
    std::shared_ptr<aasb::cbl::CBLHandler> m_CBLHandler;

#ifdef ENABLE_AAC_GLORIA
    // Gloria Card Renderer
    std::shared_ptr<aasb::gloria::CardRendererHandler> m_gloriaCardHandler;

    // Gloria List Renderer
    std::shared_ptr<aasb::gloria::ListRendererHandler> m_gloriaListHandler;
#endif //ENABLE_AAC_GLORIA

#ifdef ENABLE_AAC_CAR_CONTROL
    // Car Control Handler
    std::shared_ptr<aasb::carControl::CarControlHandler> m_carControlHandler;
#endif // ENABLE_AAC_CAR_CONTROL

    // Handler for Location provider
    std::shared_ptr<aasb::location::LocationProviderHandler> m_locationProviderHandler;

    // Network Info Provider handler
    std::shared_ptr<aasb::core::network::NetworkInfoProviderHandler> m_networkInfoProviderHandler;

    // Directive Dispatcher
    std::shared_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;

    /**
     * Configure the AACE Engine using @c m_config.
     *
     * @return true when succeeded, false otherwise.
     */
    bool configureEngine();

    /**
     * Register the AASB Audio player with AACE.
     *
     * @return true when succeeded, false otherwise.
     */
    bool registerAudioPlayerCapability();

    /**
     * Register the AASB Speech Synthesizer with AACE.
     *
     * @return true when succeeded, false otherwise.
     */
    bool registerSpeechSynthesizerCapability();

    /**
     * Register the AASB Speech Recognizer with AACE.
     *
     * @return true when succeeded, false otherwise.
     */
    bool registerSpeechRecognizerCapability();

    /**
     * Register the AASB LocalMediaSource with AACE.
     *
     * @param localMediaSourceConfig Configuration for notifying Alexa about the appropriate local media sources.
     * @return true when succeeded, false otherwise.
     */
    bool registerLocalMediaSourceCapability(
        const IConfigurationProvider::LocalMediaSourceConfiguration& localMediaSourceConfig);
};

}  // namespace bridge
}  // namespace aasb
#endif  // AASB_AASBCONTROLLERIMPL_H