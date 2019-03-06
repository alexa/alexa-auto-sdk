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
#ifndef AASB_REQUESTPROCESSOR_H
#define AASB_REQUESTPROCESSOR_H

#include <memory>

#include <AACE/Core/Engine.h>
#include <AACE/Core/EngineConfiguration.h>
#include <AACE/Alexa/AlexaProperties.h>
#include <AACE/Alexa/SpeechRecognizer.h>
#include <AACE/Engine/Core/EngineServiceManager.h>
#include <AACE/Engine/Alexa/AlexaEngineService.h>
#ifdef ENABLE_AAC_PLATFORM_AUDIO
#include <AACE/Audio/AudioManager.h>
#endif

#include <aasb/interfaces/IAlexaCapabilityDirectiveListener.h>
#include <aasb/interfaces/IConfigurationProvider.h>

#include "AlexaClientHandler.h"
#include "AudioPlayerHandler.h"
#include "AuthProviderHandler.h"
#include "CBLHandler.h"
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
 * This class is a delegate class for @c RequestHandler. @c RequestHandler implements much
 * of its API by forwarding those calls to @c RequestProcessor.
 */
class RequestProcessor {
public:
    /**
     * Destructor for @c RequestProcessor.
     */
    ~RequestProcessor() = default;

    /// Error to return when functions in this class (which returns string) are called
    /// without calling init.
    static const std::string NOT_INITALIZED_ERROR;

    /**
     * Factory method to create a new instance of @c RequestProcessor
     */
    static std::shared_ptr<RequestProcessor> create();

    /**
     * Initializes the Alexa Auto Core Engine.
     *
     * @param configProvider An object to provide the neccessary configuration for AASB.
     */
    bool init(std::shared_ptr<IConfigurationProvider> configProvider);

    /**
     * Call @c start after @c init is successfully completed. This method will register the
     * capabilities and start the Alexa Auto Engine.
     *
     */
    bool start();

    /**
     * Optional method for client to set a different AVS endpoint. By default Auto SDK
     * will connect to prod AVS endpoint.
     *
     * @param avsEndPoint IP/Name and/or port string for custom AVS endpoint.
     */
    void setAVSEndPoint(const std::string& avsEndPoint);

    /**
     * Optional method for client to set a different locale.
     *
     * @param locale New locale to set.
     */
    void setLocale(const std::string& locale);

    /**
     * Optional method for client to set the firmware version.
     *
     * @param firmwareVersion Firmware version to set.
     */
    void setFirmwareVersion(const std::string& firmwareVersion);

    /**
     * Start listening to user speech. Should be invoked in response to user's action such
     * as pressing "tap to talk" UI element (or physical button).
     *
     * @param mode Mode in which to start listening. Consult @c RequestHandler
     *      documentation to find about different modes.
     */
    bool startListening(std::string mode);

    /**
     * Client should register an instance of @c IAlexaCapabilityDirectiveListener to receive callbacks
     * from AASB when an incoming directive arrives from Alexa.
     *
     * @param listener Listener which will receive callbacks on all the incoming
     *      directives that client of this library has to process.
     * @return true on success, false otherwise.
     */
    bool registerCapabilityDirectiveListener(std::shared_ptr<IAlexaCapabilityDirectiveListener> listener);

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
    ssize_t writeAudioSamples(const int16_t* data, const size_t size);

    /**
     * Obtain current authorization state of Auto SDK.
     *
     * @return current authorization state.
     */
    std::string getAuthState();

    /**
     * Obtain current dialog state of Auto SDK.
     *
     * @return current dialog state.
     * NOTE: Refer to @c RequestHandler documentation for possible dialog state
     *      values.
     */
    std::string getDialogState();

    /**
     * Obtain current network connection state of Auto SDK.
     *
     * @return current network connection state.
     * NOTE: Refer to @c RequestHandler documentation for possible connection state
     *      values.
     */
    std::string getConnectionStatus();

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
    void onReceivedEvent(const std::string& topic, const std::string& action, const std::string& payload);

private:
    /**
     * Constructs @c RequestProcessor.
     */
    RequestProcessor();

    /**
     * Constructs @c RequestProcessor (must be used only by test).
     *
     * @c engine The engine instance to use.
     */
    RequestProcessor(std::shared_ptr<aace::core::Engine> engine);

    /**
     * Friend for testing.
     */
    friend class RequestProcessorTest;

private:
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

    // Set to true if speech recognizer is provided by AACE Platform Audio library.
    // False if @c SpeechRecognizer is an instance of @c SpeechRecognizerHandler.
    bool m_aacePlatformSpeechRecognizer;

    // Speech Syntherizer handler
    std::shared_ptr<aasb::alexa::SpeechSynthesizerHandler> m_speechSynthesizerHandler;

    // Template Runtime handler
    std::shared_ptr<aasb::alexa::TemplateRuntimeHandler> m_templateRuntimeHandler;

    // Auth Provider handler
    std::shared_ptr<aasb::alexa::AuthProviderHandler> m_authProviderHandler;

    // Handler for PCC (Phone call control)
    std::shared_ptr<aasb::phoneCallController::PhoneCallControllerHandler> m_phoneCallControllerHandler;

    // Handler for Playback Control
    std::shared_ptr<aasb::alexa::PlaybackControllerHandler> m_playbackControllerHandler;

    // Handler for Navigation
    std::shared_ptr<aasb::navigation::NavigationHandler> m_navigationHandler;

    // Handler for Location provider
    std::shared_ptr<aasb::location::LocationProviderHandler> m_locationProviderHandler;

    // Network Info Provider handler
    std::shared_ptr<aasb::core::network::NetworkInfoProviderHandler> m_networkInfoProviderHandler;

    // CBL handler
    std::shared_ptr<aasb::cbl::CBLHandler> m_CBLHandler;

    // Directive Dispatcher
    std::shared_ptr<aasb::bridge::DirectiveDispatcher> m_directiveDispatcher;

#ifdef ENABLE_AAC_PLATFORM_AUDIO
    /// Platform Audio Manager for creating audio input/output channels.
    std::unique_ptr<aace::audio::AudioManager> m_AudioManager;
#endif

    /**
     * Configure the AACE Engine using @c m_config.
     *
     * @return true when succeeded, false otherwise.
     */
    bool configureEngine();

    /**
     * Register the AASB Audio player with AACE.
     *
     * @param audioConfig Configuration for creating the appropriate audio devices.
     * @return true when succeeded, false otherwise.
     */
    bool registerAudioPlayerCapability(const IConfigurationProvider::AudioIOConfiguration& audioConfig);

    /**
     * Register the AASB Speech Synthesizer with AACE.
     *
     * @param audioConfig Configuration for creating the appropriate audio devices.
     * @return true when succeeded, false otherwise.
     */
    bool registerSpeechSynthesizerCapability(const IConfigurationProvider::AudioIOConfiguration& audioConfig);

    /**
     * Register the AASB Speech Recognizer with AACE.
     *
     * @param audioConfig Configuration for creating the appropriate audio devices.
     * @return true when succeeded, false otherwise.
     */
    bool registerSpeechRecognizerCapability(const IConfigurationProvider::AudioIOConfiguration& audioConfig);
};
}  // namespace bridge
}  // namespace aasb
#endif  // AASB_REQUESTPROCESSOR_H