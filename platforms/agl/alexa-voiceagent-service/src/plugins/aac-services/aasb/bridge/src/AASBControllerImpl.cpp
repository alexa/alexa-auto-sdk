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

#include <memory>
#include <AACE/Alexa/AlexaConfiguration.h>
#include <AACE/Storage/StorageConfiguration.h>
#include <AACE/Core/EngineConfiguration.h>
#include <aasb/Consts.h>
#include "PlatformSpecificLoggingMacros.h"
#include "AASBControllerImpl.h"
#include "SpeechRecognizerHandler.h"

// Temporarily including for GStream config creation.
#include <AACE/Engine/Utils/JSON/JSON.h>

// {@ Workaround: Load phone call control and navigation engine services
// Explicitly registering these services because other wise
// their engine libraries are not getting loaded.
// TODO: Find a better way to fix this.
#include <AACE/Engine/Core/EngineServiceManager.h>

#include <AACE/Engine/Alexa/AlexaEngineService.h>
REGISTER_SERVICE(aace::engine::alexa::AlexaEngineService)

#include <AACE/Engine/PhoneCallController/PhoneCallControllerEngineService.h>
REGISTER_SERVICE(aace::engine::phoneCallController::PhoneCallControllerEngineService)

#include <AACE/Engine/Navigation/NavigationEngineService.h>
REGISTER_SERVICE(aace::engine::navigation::NavigationEngineService)

#include <AACE/Engine/CBL/CBLEngineService.h>
REGISTER_SERVICE(aace::engine::cbl::CBLEngineService)

#include <AACE/Engine/Audio/AudioEngineService.h>
REGISTER_SERVICE(aace::engine::audio::AudioEngineService)

#ifdef ENABLE_AAC_WAKEWORD_ENGINE
#include <AACE/Engine/AmazonLite/AmazonLiteEngineService.h>
REGISTER_SERVICE(aace::engine::amazonLite::AmazonLiteEngineService)
#endif

#ifdef ENABLE_AAC_LOCAL_VOICE_CONTROL
#include <AACE/LocalVoiceControl/LocalVoiceControlConfiguration.h>
#include <AACE/Engine/LocalVoiceControl/LocalVoiceControlEngineService.h>
REGISTER_SERVICE(aace::engine::localVoiceControl::LocalVoiceControlEngineService)
#endif

#ifdef ENABLE_AAC_LOCAL_SKILLS
#include <AACE/LocalSkillService/LocalSkillServiceConfiguration.h>
#include <AACE/Engine/LocalSkillService/LocalSkillServiceEngineService.h>
REGISTER_SERVICE(aace::engine::localSkillService::LocalSkillServiceEngineService)
#endif

#ifdef ENABLE_AAC_CAR_CONTROL
#include <AACE/CarControl/CarControlConfiguration.h>
#include <AACE/Engine/CarControl/CarControlEngineService.h>
REGISTER_SERVICE(aace::engine::carControl::CarControlEngineService)
#endif

#ifdef ENABLE_AAC_GSTREAMER_AUDIO
#include <AACE/Engine/GStreamer/GStreamerEngineService.h>
REGISTER_SERVICE(aace::engine::gstreamer::GStreamerEngineService)
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
const std::string TAG = "aasb::alexa::AASBControllerImpl";

// Input channel name.
const std::string INPUT_CHANNEL_NAME = "Input";

AASBControllerImpl::AASBControllerImpl() :
        m_engine(NULL),
        m_responseDispatcher{std::make_shared<ResponseDispatcher>()} {
}

void AASBControllerImpl::setMockEngine(std::shared_ptr<aace::core::Engine> engine) {
    m_engine = engine;
}

bool AASBControllerImpl::init(std::shared_ptr<IConfigurationProvider> configProvider) {
    if (m_engine == NULL) {
        m_engine = aace::core::Engine::create();
        AASB_INFO("Alexa Auto Core Engine created");
    } else {
        AASB_ERROR("Failed to create Alexa Auto Core Engine.");
    }

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

bool AASBControllerImpl::start() {
    m_logger = aasb::core::logger::LoggerHandler::create();
    if (!m_engine->registerPlatformInterface(m_logger)) {
        AASB_ERROR("Failed to register logger handler");
        return false;
    }

    m_networkInfoProviderHandler = aasb::core::network::NetworkInfoProviderHandler::create(m_logger);
    if (!m_engine->registerPlatformInterface(m_networkInfoProviderHandler)) {
        AASB_ERROR("Failed to register network info provider handler");
        return false;
    }

    m_alexaClientHandler = aasb::alexa::AlexaClientHandler::create(m_logger, m_responseDispatcher);
    if (!m_engine->registerPlatformInterface(m_alexaClientHandler)) {
        AASB_ERROR("Failed to register alexa client handler");
        return false;
    }

    if (!registerAudioPlayerCapability()) {
        return false;
    }

    if (!registerSpeechRecognizerCapability()) {
        return false;
    }

    if (!registerSpeechSynthesizerCapability()) {
        return false;
    }

    if (m_config->shouldEnableLocalMediaSource()) {
        auto localMediaSourceConfig = m_config->getLocalMediaSourceConfig();
        if (!registerLocalMediaSourceCapability(localMediaSourceConfig)) {
            return false;
        }
    }

    m_templateRuntimeHandler = aasb::alexa::TemplateRuntimeHandler::create(m_logger, m_responseDispatcher);
    if (!m_engine->registerPlatformInterface(m_templateRuntimeHandler)) {
        AASB_ERROR("Failed to register template runtime handler");
        return false;
    }

    m_playbackControllerHandler = aasb::alexa::PlaybackControllerHandler::create(m_logger);
    if (!m_engine->registerPlatformInterface(m_playbackControllerHandler)) {
        AASB_ERROR("Failed to register playback control handler");
        return false;
    }

    if (m_config->shouldEnableCBL()) {
        std::string cblRefreshTokenFile = m_config->getAppsDataDirectory() + "/" + TOPIC_CBL + "_refresh_token";
        m_CBLHandler = aasb::cbl::CBLHandler::create(m_logger, m_responseDispatcher, cblRefreshTokenFile);
        if (!m_engine->registerPlatformInterface(m_CBLHandler)) {
            AASB_ERROR("Failed to register CBL handler");
            return false;
        }
    } else {
        m_authProviderHandler = aasb::alexa::AuthProviderHandler::create(m_logger);
        if (!m_engine->registerPlatformInterface(m_authProviderHandler)) {
            AASB_ERROR("Failed to register auth provider handler");
            return false;
        }
    }

#ifdef ENABLE_AAC_GLORIA
    if (m_config->shouldEnableGloriaCard()) {
        m_gloriaCardHandler = aasb::gloria::CardRendererHandler::create(m_logger, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_gloriaCardHandler)) {
            AASB_ERROR("Failed to register Gloria card renderer handler");
            return false;
        }
    }

    if (m_config->shouldEnableGloriaList()) {
        m_gloriaListHandler = aasb::gloria::ListRendererHandler::create(m_logger, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_gloriaListHandler)) {
            AASB_ERROR("Failed to register Gloria list renderer handler");
            return false;
        }
    }
#endif  // ENABLE_AAC_GLORIA

#ifdef ENABLE_AAC_LOCAL_SKILLS
#ifdef ENABLE_AAC_CAR_CONTROL
    if (m_config->shouldEnableCarControl()) {
        m_carControlHandler = aasb::carControl::CarControlHandler::create(m_logger, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_carControlHandler)) {
            AASB_ERROR("Failed to register car control handler");
            return false;
        }
    }
#endif // ENABLE_AAC_CAR_CONTROL
#endif // ENABLE_AAC_LOCAL_SKILLS

    if (m_config->shouldEnablePhoneCallControl()) {
        m_phoneCallControllerHandler =
                aasb::phoneCallController::PhoneCallControllerHandler::create(m_logger, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_phoneCallControllerHandler)) {
            AASB_ERROR("Failed to register phonecall control handler");
            return false;
        }
    }

    if (m_config->shouldEnableNavigation()) {
        m_navigationHandler = aasb::navigation::NavigationHandler::create(m_logger, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_navigationHandler)) {
            AASB_ERROR("Failed to register navigation handler");
            return false;
        }

        m_locationProviderHandler =
                aasb::location::LocationProviderHandler::create(m_logger, m_config, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_locationProviderHandler)) {
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

void AASBControllerImpl::setLocale(const std::string& locale) {
    if (!m_engine) {
        AASB_ERROR("Failed to set locale. Engine not created yet.");
        return;
    }

    m_engine->setProperty(aace::alexa::property::LOCALE, locale);
}

void AASBControllerImpl::setFirmwareVersion(const std::string& firmwareVersion) {
    if (!m_engine) {
        AASB_ERROR("Failed to set device firmware version. Engine not created yet.");
        return;
    }

    m_engine->setProperty(aace::alexa::property::FIRMWARE_VERSION, firmwareVersion);
}

bool AASBControllerImpl::startListening(std::string mode) {
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

ssize_t AASBControllerImpl::writeAudioSamples(const int16_t* data, const size_t size) {
    return 0;
}

bool AASBControllerImpl::configureEngine() {
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

    #if 0
    std::string externalStorageDir = m_config->getExternalStorageDirectory();
    std::string avsEndPointPath = externalStorageDir + "/aace.json";
    engineConfigurations.push_back(aace::core::config::ConfigurationFile::create(avsEndPointPath));
    #endif

#ifdef ENABLE_AAC_LOCAL_VOICE_CONTROL
    if (m_config->shouldEnableLocalVoiceControl()) {
        auto lvcConfig = m_config->getLocalVoiceControlConfig();
        engineConfigurations.push_back(
                aace::localVoiceControl::config::LocalVoiceControlConfiguration::createIPCConfig(
                        lvcConfig.socketRootDirectory + "/LVC/data",
                        aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission::ALL,
                        lvcConfig.socketRootDirectory + "/LVC/data",
                        aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission::ALL,
                        "127.0.0.1",
                        lvcConfig.socketRootDirectory + "/LVC/data"));
#ifdef ENABLE_AAC_LOCAL_SKILLS
        const std::string lssPath = lvcConfig.socketRootDirectory + "/LVC/data/lss.socket";
        const std::string erPath = lvcConfig.socketRootDirectory + "/LVC/data/er.socket";

        engineConfigurations.push_back(
                aace::localSkillService::config::LocalSkillServiceConfiguration::createLocalSkillServiceConfig(
                        lssPath, erPath));
#endif
#ifdef ENABLE_AAC_CAR_CONTROL
        if (m_config->shouldEnableCarControl()) {
            // Climate control
            auto carControlConfig = m_config->getCarControlConfig();            
            auto carControlConfigEngineConfiguration = aace::carControl::config::CarControlConfiguration::create();
            for (auto zone : carControlConfig.zones) {
                std::string name = zone.name;
                // Climate Control
                if (zone.climate.enabled) {
                    carControlConfigEngineConfiguration->createClimateControl(name);
                    carControlConfigEngineConfiguration->addPowerController(true);
                    if (zone.climate.addSyncController) {
                        carControlConfigEngineConfiguration->addClimateSyncController();
                    }

                    if (zone.climate.addRecirculationController) {
                        carControlConfigEngineConfiguration->addAirRecirculationController();
                    }
                }

                // Air Conditioner
                if (zone.airConditioner.enabled) {
                    carControlConfigEngineConfiguration->createAirConditionerControl(name).addPowerController(true);

                    std::vector<aace::carControl::CarControl::AirConditionerMode> modes;                    
                    for (auto mode : zone.airConditioner.modes) {
                        if (VALUE_CARCONTROL_AC_MODE_MANUAL == mode) {
                            modes.push_back(aace::carControl::CarControl::AirConditionerMode::MANUAL);
                        } else if (VALUE_CARCONTROL_AC_MODE_AUTO == mode) {
                            modes.push_back(aace::carControl::CarControl::AirConditionerMode::AUTO);
                        } else if (VALUE_CARCONTROL_AC_MODE_ECONOMY == mode) {
                            modes.push_back(aace::carControl::CarControl::AirConditionerMode::ECONOMY);
                        } else if (VALUE_CARCONTROL_AC_MODE_MAXIMUM == mode) {
                            modes.push_back(aace::carControl::CarControl::AirConditionerMode::MAXIMUM);
                        } else {
                            m_logger->log(Level::WARN, TAG, "Received Unknown AC mode " + mode);
                        }
                    }
                    carControlConfigEngineConfiguration->addAirConditionerModeController(modes);         
                }

                // Heater
                if (zone.heater.enabled) {
                    carControlConfigEngineConfiguration->createHeaterControl(name).addPowerController(true);
                    
                    if (VALUE_CARCONTROL_TEMPERATURE_UNIT_CELSIUS == zone.heater.unit) {
                        carControlConfigEngineConfiguration->addTemperatureController(zone.heater.minimum,
                                                                                      zone.heater.maximum,
                                                                                      zone.heater.precision,
                                                                                      aace::carControl::CarControl::TemperatureUnit::CELSIUS);
                    } else if (VALUE_CARCONTROL_TEMPERATURE_UNIT_FAHRENHEIT == zone.heater.unit) {
                        carControlConfigEngineConfiguration->addTemperatureController(zone.heater.minimum,
                                                                                      zone.heater.maximum,
                                                                                      zone.heater.precision,
                                                                                      aace::carControl::CarControl::TemperatureUnit::FAHRENHEIT);
                    } else {
                        m_logger->log(Level::WARN, TAG, "Received Unknown Heater Unit " + zone.heater.unit);
                    }

                }

                // Fan
                if (zone.fan.enabled) {
                    carControlConfigEngineConfiguration->createFanControl(name)
                        .addPowerController(true)
                        .addFanSpeedController(zone.fan.minimum, zone.fan.maximum, zone.fan.precision);
                }

                // Vent
                if (zone.vent.enabled) {
                    std::vector<aace::carControl::CarControl::VentPosition> ventPositions;                    
                    for (auto position : zone.vent.positions) {
                        if (VALUE_CARCONTROL_VENT_POSITION_BODY == position) {
                            ventPositions.push_back(aace::carControl::CarControl::VentPosition::BODY);
                        } else if (VALUE_CARCONTROL_VENT_POSITION_MIX == position) {
                            ventPositions.push_back(aace::carControl::CarControl::VentPosition::MIX);
                        } else if (VALUE_CARCONTROL_VENT_POSITION_FLOOR == position) {
                            ventPositions.push_back(aace::carControl::CarControl::VentPosition::FLOOR);
                        } else if (VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD == position) {
                            ventPositions.push_back(aace::carControl::CarControl::VentPosition::WINDSHIELD);
                        } else {
                            m_logger->log(Level::WARN, TAG, "Received Unknown Vent Position " + position);
                        }
                    }

                    carControlConfigEngineConfiguration->createVentControl(name)
                        .addPowerController(true)
                        .addVentPositionController(ventPositions);
                }

                // Window
                if (zone.window.enabled) {
                    carControlConfigEngineConfiguration->createWindowControl(name);
                    if (zone.window.defrost) {
                        carControlConfigEngineConfiguration->addDefrostController();
                    }
                }

                // Light
                if (zone.light.enabled) {
                    aace::carControl::CarControl::LightType lightType = aace::carControl::CarControl::LightType::LIGHT;
                    if (zone.light.type == VALUE_CARCONTROL_LIGHT_TYPE_AMBIENT_LIGHT) {
                        lightType = aace::carControl::CarControl::LightType::AMBIENT_LIGHT;
                    } else if (zone.light.type == VALUE_CARCONTROL_LIGHT_TYPE_DOME_LIGHT) {
                        lightType = aace::carControl::CarControl::LightType::DOME_LIGHT;
                    } else if (zone.light.type == VALUE_CARCONTROL_LIGHT_TYPE_LIGHT) {
                        lightType = aace::carControl::CarControl::LightType::LIGHT;
                    } else if (zone.light.type == VALUE_CARCONTROL_LIGHT_TYPE_READING_LIGHT) {
                        lightType = aace::carControl::CarControl::LightType::READING_LIGHT;
                    } else if (zone.light.type == VALUE_CARCONTROL_LIGHT_TYPE_TRUNK_LIGHT) {
                        lightType = aace::carControl::CarControl::LightType::TRUNK_LIGHT;
                    }

                    carControlConfigEngineConfiguration->createLightControl(lightType, name).addPowerController(true);

                    std::vector<aace::carControl::CarControl::LightColor> colors;
                    for (auto color : zone.light.colors) {
                        if (VALUE_CARCONTROL_LIGHT_COLOR_WHITE == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::WHITE);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_RED == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::RED);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_ORANGE == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::ORANGE);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_YELLOW == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::YELLOW);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_GREEN == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::GREEN);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_BLUE == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::BLUE);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_INDIGO == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::INDIGO);
                        } else if (VALUE_CARCONTROL_LIGHT_COLOR_VIOLET == color) {
                            colors.push_back(aace::carControl::CarControl::LightColor::VIOLET);
                        } else {
                            m_logger->log(Level::WARN, TAG, "Received Unknown Light Color " + color);
                        }
                    }
                    carControlConfigEngineConfiguration->addLightColorController(colors);
                }
            }

            engineConfigurations.push_back(carControlConfigEngineConfiguration);
        }
#endif
    }
#endif

#if ENABLE_AAC_GSTREAMER_AUDIO
    IConfigurationProvider::AudioIOConfiguration audioConfig = m_config->getAudioIOConfig();
    if (audioConfig.useGStreamerAudioIO) {
        // Audio Input configuration
        rapidjson::Document document( rapidjson::kObjectType );
        rapidjson::Value aaceGStreamerElement( rapidjson::kObjectType );
        rapidjson::Value audioInputProviderElement( rapidjson::kObjectType );
        rapidjson::Value audioInputDevicesElement( rapidjson::kObjectType );

        audioInputProviderElement.AddMember("enabled", rapidjson::Value().SetBool(true), document.GetAllocator());
        audioInputDevicesElement.AddMember("voice", rapidjson::Value().SetString( audioConfig.voiceInputDevice.c_str(), audioConfig.voiceInputDevice.length() ), document.GetAllocator());
        audioInputDevicesElement.AddMember("communication", rapidjson::Value().SetString( audioConfig.communicationInputDevice.c_str(), audioConfig.communicationInputDevice.length() ), document.GetAllocator());
        audioInputDevicesElement.AddMember("loopback", rapidjson::Value().SetString( audioConfig.loopbackInputDevice.c_str(), audioConfig.loopbackInputDevice.length() ), document.GetAllocator());

        audioInputProviderElement.AddMember("deviceNames", audioInputDevicesElement, document.GetAllocator());            
        aaceGStreamerElement.AddMember("AudioInputProvider", audioInputProviderElement, document.GetAllocator());


        // Audio Output configuration
        rapidjson::Value audioOutputProviderElement( rapidjson::kObjectType );
        rapidjson::Value audioOutputDevicesElement( rapidjson::kObjectType );

        audioOutputProviderElement.AddMember("enabled", rapidjson::Value().SetBool(true), document.GetAllocator());
        audioOutputDevicesElement.AddMember("tts", rapidjson::Value().SetString( audioConfig.ttsOutputDevice.c_str(), audioConfig.ttsOutputDevice.length() ), document.GetAllocator());
        audioOutputDevicesElement.AddMember("music", rapidjson::Value().SetString( audioConfig.musicOutputDevice.c_str(), audioConfig.musicOutputDevice.length() ), document.GetAllocator());
        audioOutputDevicesElement.AddMember("notification", rapidjson::Value().SetString( audioConfig.notificationOutputDevice.c_str(), audioConfig.notificationOutputDevice.length() ), document.GetAllocator());
        audioOutputDevicesElement.AddMember("alarm", rapidjson::Value().SetString( audioConfig.alarmOutputDevice.c_str(), audioConfig.alarmOutputDevice.length() ), document.GetAllocator());
        audioOutputDevicesElement.AddMember("earcon", rapidjson::Value().SetString( audioConfig.earconOutputDevice.c_str(), audioConfig.earconOutputDevice.length() ), document.GetAllocator());
        audioOutputDevicesElement.AddMember("communication", rapidjson::Value().SetString( audioConfig.communicationOutputDevice.c_str(), audioConfig.communicationOutputDevice.length() ), document.GetAllocator());
        audioOutputDevicesElement.AddMember("ringtone", rapidjson::Value().SetString( audioConfig.ringtoneOutputDevice.c_str(), audioConfig.ringtoneOutputDevice.length() ), document.GetAllocator());

        audioOutputProviderElement.AddMember("deviceNames", audioOutputDevicesElement, document.GetAllocator());            
        aaceGStreamerElement.AddMember("AudioOutputProvider", audioOutputProviderElement, document.GetAllocator());

        document.AddMember( "aace.gstreamer", aaceGStreamerElement, document.GetAllocator() );

        engineConfigurations.push_back(
            aace::core::config::StreamConfiguration::create(aace::engine::utils::json::toStream(document)));
    }    
#endif    
    return m_engine->configure(engineConfigurations);
}

bool AASBControllerImpl::registerCapabilityDirectiveListener(
    std::shared_ptr<aasb::bridge::IAlexaCapabilityDirectiveListener> listener) {
    m_responseDispatcher->registerCapabilityDirectiveListener(listener);
    return true;
}

void AASBControllerImpl::onReceivedEvent(
    const std::string& topic,
    const std::string& action,
    const std::string& payload) {
    if (topic == TOPIC_AUTH_PROVIDER) {
        if (m_authProviderHandler) {
            m_logger->log(Level::INFO, TAG, "Received AuthProvider Event: " + action + " payload: " + payload);
            m_authProviderHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "Auth Provider is not enabled.");
        }
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
        if (m_playbackControllerHandler) {
            m_logger->log(Level::INFO, TAG, "Received Playback Controller Event: " + action + " payload: " + payload);
            m_playbackControllerHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "Playback Controller is not enabled.");
        }
        return;
    }

    if (topic == TOPIC_CBL) {
        if (m_CBLHandler) {
            m_logger->log(Level::INFO, TAG, "Received CBL Event: " + action + " payload: " + payload);
            m_CBLHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "CBL is not enabled.");
        }
        return;
    }

    if (topic == TOPIC_LOCAL_MEDIA_SOURCE) {
        m_logger->log(Level::INFO, TAG, "Received Local Media Source Event: " + action + " payload: " + payload);
        m_LocalMediaSourceHandlerManager->onReceivedEvent(action, payload);
        return;
    }

    if (topic == TOPIC_LOCATIONPROVIDER) {
        m_logger->log(Level::INFO, TAG, "Received Location Provider Event: " + action + " payload: " + payload);
        if (m_locationProviderHandler) {
            m_locationProviderHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "Location provider not enabled.");
        }
    }

#ifdef ENABLE_AAC_GLORIA
    if (topic == TOPIC_GLORIA_LISTRENDERER) {
        m_logger->log(Level::INFO, TAG, "Received Gloria list renderer Event: " + action + " payload: " + payload);
        if (m_gloriaListHandler) {
            m_gloriaListHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "Gloria list rendering not enabled");
        }
        return;
    }
#endif  // ENABLE_AAC_GLORIA

#ifdef ENABLE_AAC_CAR_CONTROL
    if (topic == TOPIC_CARCONTROL) {
        m_logger->log(Level::INFO, TAG, "Received Car Control Event: " + action + " payload: " + payload);
        if (m_carControlHandler) {
            m_carControlHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "CarControl not enabled");
        }
        return;
    }
#endif // ENABLE_AAC_CAR_CONTROL

    AASB_ERROR("UKNOWN TOPIC %s", topic.c_str());
}

bool AASBControllerImpl::registerAudioPlayerCapability() {
    if (!m_engine->registerPlatformInterface(
            m_audioPlayerHandler =
                aasb::alexa::AudioPlayerHandler::create(m_logger))) {
        AASB_ERROR("Failed to register audio player handler");
        return false;
    }

    return true;
}

bool AASBControllerImpl::registerSpeechSynthesizerCapability() {
    if (!m_engine->registerPlatformInterface(
            m_speechSynthesizerHandler =
                aasb::alexa::SpeechSynthesizerHandler::create())) {
        AASB_ERROR("Failed to register speech synthesizer handler");
        return false;
    }

    return true;
}

bool AASBControllerImpl::registerSpeechRecognizerCapability() {
    bool wakeWordEnabled = m_config->shouldEnableWakeword();
#ifndef ENABLE_AAC_WAKEWORD_ENGINE
    if (wakeWordEnabled) {
        wakeWordEnabled = false;
        AASB_WARNING("Cannot enable wakeword engine because AAC isnt compiled with Wakeword engine.");
    }
#endif

    if (!m_engine->registerPlatformInterface(m_speechRecognizer =
        aasb::alexa::SpeechRecognizerHandler::create(wakeWordEnabled, m_logger, m_responseDispatcher))) {
        AASB_ERROR("Failed to register speech recognizer handler");
        return false;
    }

    if (wakeWordEnabled) {
        if (!m_speechRecognizer->enableWakewordDetection()) {
            AASB_WARNING("Enabling wakeword failed");
        }
    }

    return true;
}

bool AASBControllerImpl::registerLocalMediaSourceCapability(
    const IConfigurationProvider::LocalMediaSourceConfiguration& localMediaSourceConfig) {
    m_LocalMediaSourceHandlerManager =
        aasb::alexa::LocalMediaSourceHandlerManager::create(m_logger, m_responseDispatcher);

    if (localMediaSourceConfig.hasBluetooth) {
        AASB_INFO("Registering BT source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getBluetoothSource())) {
            AASB_ERROR("Failed to register BT source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasUSB) {
        AASB_INFO("Registering USB source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getUSBSource())) {
            AASB_ERROR("Failed to register USB source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasFMRadio) {
        AASB_INFO("Registering FM Radio source");

        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getFMRadioSource())) {
            AASB_ERROR("Failed to register FM Radio source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasAMRadio) {
        AASB_INFO("Registering AM Radio source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getAMRadioSource())) {
            AASB_ERROR("Failed to register AM Radio source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasSatelliteRadio) {
        AASB_INFO("Registering Satellite Radio source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getSatelliteRadioSource())) {
            AASB_ERROR("Failed to register Satellite Radio source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasLineIn) {
        AASB_INFO("Registering Line In source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getLineInSource())) {
            AASB_ERROR("Failed to register Line In source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasCompactDisc) {
        AASB_INFO("Registering Compact Disc source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getCompactDiscSource())) {
            AASB_ERROR("Failed to register Compact Disc source");
            return false;
        }
    }

    return true;
}

bool AASBControllerImpl::setWakeWordEnable(bool wakeWordEnable) {
#ifndef ENABLE_AAC_WAKEWORD_ENGINE
    if (wakeWordEnable) {
        AASB_WARNING("Cannot enable wakeword engine because AAC isnt compiled with Wakeword engine.");
        return false;
    }
#endif

    if (wakeWordEnable && !m_speechRecognizer->enableWakewordDetection()) {
        AASB_WARNING("Enabling wakeword failed");
        return false;
    }

    if (!wakeWordEnable && !m_speechRecognizer->disableWakewordDetection()) {
        AASB_WARNING("Disabling wakeword failed");
        return false;
    }

    return true;
}

}  // namespace bridge
}  // namespace aasb