/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/CarControl/CarControlAssets.h>
#include <AACE/CarControl/CarControlConfiguration.h>
#include <AACE/Engine/CarControl/CarControlEngineService.h>
REGISTER_SERVICE(aace::engine::carControl::CarControlEngineService)

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

#ifdef ENABLE_AAC_CAR_CONTROL_LOCAL_SERVICE
#include <AACE/Engine/CarControlLocalService/CarControlLocalService.h>
REGISTER_SERVICE(aace::engine::carControlLocalService::CarControlLocalService)
#endif

#ifdef ENABLE_AAC_SYSTEM_AUDIO
#include <AACE/Engine/SystemAudio/SystemAudioEngineService.h>
REGISTER_SERVICE(aace::engine::systemAudio::SystemAudioEngineService)
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
        m_engine(NULL), m_responseDispatcher{std::make_shared<ResponseDispatcher>()} {
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

    m_alertsHandler = aasb::alexa::AlertsHandler::create(m_logger, m_responseDispatcher);
    if (!m_engine->registerPlatformInterface(m_alertsHandler)) {
        AASB_ERROR("Failed to register alerts handler");
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

    if (m_config->shouldEnableCarControl()) {
        m_carControlHandler = aasb::carControl::CarControlHandler::create(m_logger, m_responseDispatcher);
        if (!m_engine->registerPlatformInterface(m_carControlHandler)) {
            AASB_ERROR("Failed to register car control handler");
            return false;
        }
    }

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
        m_config->getProductDSN(),
        m_config->getClientId(),
        m_config->getProductId(),
        m_config->getManufacturerName(),
        m_config->getDescription()));

    engineConfigurations.push_back(AlexaConfiguration::createCurlConfig(m_config->getCertificatesDirectoryPath()));

    // Get the directory to the where the Alexa voice agent is installed
    std::string appDataDir = m_config->getAppsDataDirectory();

    // Create AVS configuration under appDataDir
    std::string miscStorageDbPath = appDataDir + "/miscStorage.db";
    engineConfigurations.push_back(AlexaConfiguration::createMiscStorageConfig(miscStorageDbPath));

    std::string certifiedSenderDbPath = appDataDir + "/certifiedSender.db";
    engineConfigurations.push_back(AlexaConfiguration::createCertifiedSenderConfig(certifiedSenderDbPath));

    std::string alertsCapabilityAgentDbPath = appDataDir + "/alertsCapabilityAgent.db";
    engineConfigurations.push_back(AlexaConfiguration::createAlertsConfig(alertsCapabilityAgentDbPath));

    std::string notificationsDbPath = appDataDir + "/notifications.db";
    engineConfigurations.push_back(AlexaConfiguration::createNotificationsConfig(notificationsDbPath));

    std::string capabilitiesDelegate = appDataDir + "/capabilitiesDelegate.db";
    engineConfigurations.push_back(AlexaConfiguration::createCapabilitiesDelegateConfig(capabilitiesDelegate));

    std::string localStoragePath = appDataDir + "/localStorage.db";
    engineConfigurations.push_back(
        aace::storage::config::StorageConfiguration::createLocalStorageConfig(localStoragePath));

    auto deviceSettingsConfig = m_config->getDeviceSettingsConfig();
    auto defaultLocale = deviceSettingsConfig.defaultLocale.empty() ? "en-US" : deviceSettingsConfig.defaultLocale;
    auto defaultTimezone =
        deviceSettingsConfig.defaultTimezone.empty() ? "America/Vancouver" : deviceSettingsConfig.defaultTimezone;
    std::string deviceSettingsDbPath = appDataDir + "/deviceSettings.db";
    std::vector<std::string> locales = {"en-US",
                                        "en-GB",
                                        "de-DE",
                                        "en-IN",
                                        "en-CA",
                                        "ja-JP",
                                        "en-AU",
                                        "fr-FR",
                                        "it-IT",
                                        "es-ES",
                                        "es-MX",
                                        "fr-CA",
                                        "es-US",
                                        "hi-IN",
                                        "pt-BR"};
    std::vector<std::vector<std::string>> localeCombinations = {};

    if (!deviceSettingsConfig.locales.empty()) {
        locales = deviceSettingsConfig.locales;
    }

    engineConfigurations.push_back(AlexaConfiguration::createDeviceSettingsConfig(
        deviceSettingsDbPath, locales, defaultLocale, defaultTimezone, localeCombinations));

#if 0
    std::string externalStorageDir = m_config->getExternalStorageDirectory();
    std::string avsEndPointPath = externalStorageDir + "/aace.json";
    engineConfigurations.push_back(aace::core::config::ConfigurationFile::create(avsEndPointPath));
#endif

    // Pass Vehicle information
    engineConfigurations.push_back(aace::core::config::StreamConfiguration::create(m_config->getVehicleConfig()));

    // m_logger is not available until after AAC engine is started, so log to std::cerr if neeeded
    if (m_config->shouldEnableCarControl()) {
        // Climate control
        auto carControlConfig = m_config->getCarControlConfig();
        auto carControlEngineConfiguration = aace::carControl::config::CarControlConfiguration::create();
        for (auto const& endpoint : carControlConfig.endpoints) {
            std::string zone = endpoint.zone;
            // Climate Control
            if (endpoint.climate.enabled) {
                carControlEngineConfiguration->createControl(endpoint.climate.controlId, zone)
                    .addAssetId(aace::carControl::assets::alexa::device::CAR);
                if (!endpoint.climate.syncControllerId.empty()) {
                    carControlEngineConfiguration->addToggleController(endpoint.climate.syncControllerId, true)
                        .addAssetId(aace::carControl::assets::alexa::setting::CLIMATE_SYNC);
                }

                if (!endpoint.climate.recirculationControllerId.empty()) {
                    carControlEngineConfiguration->addToggleController(endpoint.climate.recirculationControllerId, true)
                        .addAssetId(aace::carControl::assets::alexa::setting::AIR_RECIRCULATION);
                }
            }
            // Air Conditioner
            if (endpoint.airConditioner.enabled) {
                carControlEngineConfiguration->createControl(endpoint.airConditioner.controlId, zone)
                    .addAssetId(aace::carControl::assets::alexa::device::AIR_CONDITIONER)
                    .addPowerController(true);
                // Mode ModeController
                if (!endpoint.airConditioner.modeValues.empty()) {
                    carControlEngineConfiguration
                        ->addModeController(endpoint.airConditioner.modeControllerId, true, false)
                        .addAssetId(aace::carControl::assets::alexa::setting::MODE);
                    for (auto const& value : endpoint.airConditioner.modeValues) {
                        if (VALUE_CARCONTROL_AC_MODE_MANUAL == value) {
                            carControlEngineConfiguration->addValue(VALUE_CARCONTROL_AC_MODE_MANUAL)
                                .addAssetId(aace::carControl::assets::alexa::setting::MANUAL);
                        } else if (VALUE_CARCONTROL_AC_MODE_AUTO == value) {
                            carControlEngineConfiguration->addValue(VALUE_CARCONTROL_AC_MODE_AUTO)
                                .addAssetId(aace::carControl::assets::alexa::setting::AUTO);
                        } else if (VALUE_CARCONTROL_AC_MODE_ECONOMY == value) {
                            carControlEngineConfiguration->addValue(VALUE_CARCONTROL_AC_MODE_ECONOMY)
                                .addAssetId(aace::carControl::assets::alexa::setting::ECONOMY);
                        } else {
                            std::cerr << TAG << ":Received Unknown AC mode " << value << std::endl;
                        }
                    }
                }
                // Intensity ModeController
                if (!endpoint.airConditioner.intensityValues.empty()) {
                    carControlEngineConfiguration
                        ->addModeController(endpoint.airConditioner.intensityControllerId, true, true)
                        .addAssetId(aace::carControl::assets::alexa::setting::INTENSITY);
                    for (auto const& value : endpoint.airConditioner.intensityValues) {
                        if (VALUE_CARCONTROL_AC_INTENSITY_LOW == value) {
                            carControlEngineConfiguration->addValue(VALUE_CARCONTROL_AC_INTENSITY_LOW)
                                .addAssetId(aace::carControl::assets::alexa::value::LOW)
                                .addAssetId(aace::carControl::assets::alexa::value::MINIMUM);
                        } else if (VALUE_CARCONTROL_AC_INTENSITY_MEDIUM == value) {
                            carControlEngineConfiguration->addValue(VALUE_CARCONTROL_AC_INTENSITY_MEDIUM)
                                .addAssetId(aace::carControl::assets::alexa::value::MEDIUM);
                        } else if (VALUE_CARCONTROL_AC_INTENSITY_HIGH == value) {
                            carControlEngineConfiguration->addValue(VALUE_CARCONTROL_AC_INTENSITY_HIGH)
                                .addAssetId(aace::carControl::assets::alexa::value::HIGH)
                                .addAssetId(aace::carControl::assets::alexa::value::MAXIMUM);
                        } else {
                            std::cerr << TAG << ":Received Unknown AC intensity " << value << std::endl;
                        }
                    }
                }
            }
            // Heater
            if (endpoint.heater.enabled) {
                std::string unit = "";
                if (VALUE_CARCONTROL_TEMPERATURE_UNIT_CELSIUS == endpoint.heater.unit) {
                    unit = aace::carControl::assets::alexa::unit::CELSIUS;
                } else if (VALUE_CARCONTROL_TEMPERATURE_UNIT_FAHRENHEIT == endpoint.heater.unit) {
                    unit = aace::carControl::assets::alexa::unit::FAHRENHEIT;
                }

                // The preset values must be exact increments of the precision otherwise
                // range controller will fail
                double mid = (endpoint.heater.maximum - endpoint.heater.minimum) / endpoint.heater.precision;
                int midInt = mid / 2;
                double medium = endpoint.heater.minimum + midInt * endpoint.heater.precision;

                carControlEngineConfiguration->createControl(endpoint.heater.controlId, zone)
                    .addAssetId(aace::carControl::assets::alexa::device::HEATER)
                    .addAssetId(aace::carControl::assets::alexa::device::COOLER)
                    .addPowerController(true)
                    .addRangeController(
                        endpoint.heater.controllerId,
                        true,
                        endpoint.heater.minimum,
                        endpoint.heater.maximum,
                        endpoint.heater.precision,
                        unit)
                    .addAssetId(aace::carControl::assets::alexa::setting::TEMPERATURE)
                    .addAssetId(aace::carControl::assets::alexa::setting::HEAT)
                    .addPreset(endpoint.heater.minimum)
                    .addAssetId(aace::carControl::assets::alexa::value::LOW)
                    .addAssetId(aace::carControl::assets::alexa::value::MINIMUM)
                    .addPreset(medium)
                    .addAssetId(aace::carControl::assets::alexa::value::MEDIUM)
                    .addPreset(endpoint.heater.maximum)
                    .addAssetId(aace::carControl::assets::alexa::value::HIGH)
                    .addAssetId(aace::carControl::assets::alexa::value::MAXIMUM);
            }
            // Fan
            if (endpoint.fan.enabled) {
                // The preset values must be exact increments of the precision otherwise
                // range controller will fail
                double mid = (endpoint.fan.maximum - endpoint.fan.minimum) / endpoint.fan.precision;
                int midInt = mid / 2;
                double medium = endpoint.fan.minimum + midInt * endpoint.fan.precision;

                carControlEngineConfiguration->createControl(endpoint.fan.controlId, zone)
                    .addAssetId(aace::carControl::assets::alexa::device::FAN)
                    .addPowerController(true)
                    .addRangeController(
                        endpoint.fan.controllerId,
                        true,
                        endpoint.fan.minimum,
                        endpoint.fan.maximum,
                        endpoint.fan.precision)
                    .addAssetId(aace::carControl::assets::alexa::setting::FAN_SPEED)
                    .addPreset(endpoint.fan.minimum)
                    .addAssetId(aace::carControl::assets::alexa::value::LOW)
                    .addAssetId(aace::carControl::assets::alexa::value::MINIMUM)
                    .addPreset(medium)
                    .addAssetId(aace::carControl::assets::alexa::value::MEDIUM)
                    .addPreset(endpoint.fan.maximum)
                    .addAssetId(aace::carControl::assets::alexa::value::HIGH)
                    .addAssetId(aace::carControl::assets::alexa::value::MAXIMUM);
            }

            // Vent
            if (endpoint.vent.enabled) {
                carControlEngineConfiguration->createControl(endpoint.vent.controlId, zone)
                    .addAssetId(aace::carControl::assets::alexa::device::VENT)
                    .addPowerController(true)
                    .addModeController(endpoint.vent.positionsControllerId, true, true)
                    .addAssetId(aace::carControl::assets::alexa::setting::POSITION);

                for (auto position : endpoint.vent.positions) {
                    if (VALUE_CARCONTROL_VENT_POSITION_BODY == position) {
                        carControlEngineConfiguration->addValue(VALUE_CARCONTROL_VENT_POSITION_BODY)
                            .addAssetId(aace::carControl::assets::alexa::setting::BODY_VENTS);
                    } else if (VALUE_CARCONTROL_VENT_POSITION_MIX == position) {
                        carControlEngineConfiguration->addValue(VALUE_CARCONTROL_VENT_POSITION_MIX)
                            .addAssetId(aace::carControl::assets::alexa::setting::MIX_VENTS);
                    } else if (VALUE_CARCONTROL_VENT_POSITION_FLOOR == position) {
                        carControlEngineConfiguration->addValue(VALUE_CARCONTROL_VENT_POSITION_FLOOR)
                            .addAssetId(aace::carControl::assets::alexa::setting::FLOOR_VENTS);
                    } else if (VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD == position) {
                        carControlEngineConfiguration->addValue(VALUE_CARCONTROL_VENT_POSITION_WINDSHIELD)
                            .addAssetId(aace::carControl::assets::alexa::setting::WINDSHIELD_VENTS);
                    } else {
                        std::cerr << TAG << ":Received Unknown Vent Position " << position << std::endl;
                    }
                }
            }

            // Window
            if (endpoint.window.defrost) {
                carControlEngineConfiguration->createControl(endpoint.window.controlId, zone)
                    .addAssetId(aace::carControl::assets::alexa::device::WINDSHIELD)
                    .addAssetId(aace::carControl::assets::alexa::device::WINDOW)
                    .addToggleController(endpoint.window.controllerId, true)
                    .addAssetId(aace::carControl::assets::alexa::setting::DEFROST);
            }

            // Light
            if (!endpoint.lights.empty()) {
                for (auto const& light : endpoint.lights) {
                    carControlEngineConfiguration->createControl(light.controlId, zone);
                    if (light.type == VALUE_CARCONTROL_LIGHT_TYPE_AMBIENT_LIGHT) {
                        carControlEngineConfiguration->addAssetId(
                            aace::carControl::assets::alexa::device::AMBIENT_LIGHT);
                    } else if (light.type == VALUE_CARCONTROL_LIGHT_TYPE_DOME_LIGHT) {
                        carControlEngineConfiguration->addAssetId(aace::carControl::assets::alexa::device::DOME_LIGHT);
                        carControlEngineConfiguration->addAssetId(aace::carControl::assets::alexa::device::CABIN_LIGHT);
                    } else if (light.type == VALUE_CARCONTROL_LIGHT_TYPE_LIGHT) {
                        carControlEngineConfiguration->addAssetId(aace::carControl::assets::alexa::device::LIGHT);
                    } else if (light.type == VALUE_CARCONTROL_LIGHT_TYPE_READING_LIGHT) {
                        carControlEngineConfiguration->addAssetId(
                            aace::carControl::assets::alexa::device::READING_LIGHT);
                    } else if (light.type == VALUE_CARCONTROL_LIGHT_TYPE_TRUNK_LIGHT) {
                        carControlEngineConfiguration->addAssetId(aace::carControl::assets::alexa::device::TRUNK_LIGHT);
                    }

                    carControlEngineConfiguration->addPowerController(true);

                    if (!light.colors.empty()) {
                        carControlEngineConfiguration->addModeController(light.colorControllerId, true, true)
                            .addAssetId(aace::carControl::assets::alexa::setting::COLOR);
                        for (auto const& color : light.colors) {
                            if (VALUE_CARCONTROL_LIGHT_COLOR_WHITE == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_WHITE)
                                    .addAssetId(aace::carControl::assets::alexa::color::WHITE);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_RED == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_RED)
                                    .addAssetId(aace::carControl::assets::alexa::color::RED);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_ORANGE == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_ORANGE)
                                    .addAssetId(aace::carControl::assets::alexa::color::ORANGE);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_YELLOW == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_YELLOW)
                                    .addAssetId(aace::carControl::assets::alexa::color::YELLOW);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_GREEN == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_GREEN)
                                    .addAssetId(aace::carControl::assets::alexa::color::GREEN);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_BLUE == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_BLUE)
                                    .addAssetId(aace::carControl::assets::alexa::color::BLUE);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_INDIGO == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_INDIGO)
                                    .addAssetId(aace::carControl::assets::alexa::color::INDIGO);
                            } else if (VALUE_CARCONTROL_LIGHT_COLOR_VIOLET == color) {
                                carControlEngineConfiguration->addValue(VALUE_CARCONTROL_LIGHT_COLOR_VIOLET)
                                    .addAssetId(aace::carControl::assets::alexa::color::VIOLET);
                            } else {
                                std::cerr << TAG << ":Received Unknown Light Color " << color << std::endl;
                            }
                        }
                    }
                }
            }
        }

        engineConfigurations.push_back(carControlEngineConfiguration);
    }

#ifdef ENABLE_AAC_LOCAL_VOICE_CONTROL
    if (m_config->shouldEnableLocalVoiceControl()) {
        auto lvcConfig = m_config->getLocalVoiceControlConfig();
        engineConfigurations.push_back(aace::localVoiceControl::config::LocalVoiceControlConfiguration::createIPCConfig(
            lvcConfig.socketRootDirectory + "/LVC/data",
            aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission::OWNER,
            lvcConfig.socketRootDirectory + "/LVC/data",
            aace::localVoiceControl::config::LocalVoiceControlConfiguration::SocketPermission::OWNER,
            "127.0.0.1",
            lvcConfig.socketRootDirectory + "/LVC/data"));
#ifdef ENABLE_AAC_LOCAL_SKILLS
        const std::string lssPath = lvcConfig.socketRootDirectory + "/LVC/data/lss.socket";

        engineConfigurations.push_back(
            aace::localSkillService::config::LocalSkillServiceConfiguration::createLocalSkillServiceConfig(lssPath));
#endif
    }
#endif

#ifdef ENABLE_AAC_SYSTEM_AUDIO
    IConfigurationProvider::AudioIOConfiguration audioConfig = m_config->getAudioIOConfig();
    if (audioConfig.useGStreamerAudioIO) {
        // Audio Input configuration
        rapidjson::Document document(rapidjson::kObjectType);
        rapidjson::Value aaceGStreamerElement(rapidjson::kObjectType);
        rapidjson::Value audioInputProviderElement(rapidjson::kObjectType);
        rapidjson::Value audioInputDevicesElement(rapidjson::kObjectType);
        rapidjson::Value audioInputTypesElement(rapidjson::kObjectType);

        audioInputProviderElement.AddMember("enabled", rapidjson::Value().SetBool(true), document.GetAllocator());

        if (!audioConfig.voiceInputDevice.empty()) {
            rapidjson::Value audioInputVoiceDeviceElement(rapidjson::kObjectType);
            audioInputVoiceDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.voiceInputDevice.c_str(), audioConfig.voiceInputDevice.length()),
                document.GetAllocator());
            audioInputDevicesElement.AddMember("voice", audioInputVoiceDeviceElement, document.GetAllocator());
            audioInputTypesElement.AddMember("VOICE", "voice", document.GetAllocator());
        }

        if (!audioConfig.communicationInputDevice.empty()) {
            rapidjson::Value audioInputCommunicationDeviceElement(rapidjson::kObjectType);
            audioInputCommunicationDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.communicationInputDevice.c_str(), audioConfig.communicationInputDevice.length()),
                document.GetAllocator());
            audioInputDevicesElement.AddMember(
                "communication", audioInputCommunicationDeviceElement, document.GetAllocator());
            audioInputTypesElement.AddMember("COMMUNICATION", "communication", document.GetAllocator());
        }

        if (!audioConfig.loopbackInputDevice.empty()) {
            rapidjson::Value audioInputLoopbackDeviceElement(rapidjson::kObjectType);
            audioInputLoopbackDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.loopbackInputDevice.c_str(), audioConfig.loopbackInputDevice.length()),
                document.GetAllocator());
            audioInputDevicesElement.AddMember("loopback", audioInputLoopbackDeviceElement, document.GetAllocator());
            audioInputTypesElement.AddMember("LOOPBACK", "loopback", document.GetAllocator());
        }

        audioInputProviderElement.AddMember("devices", audioInputDevicesElement, document.GetAllocator());
        audioInputProviderElement.AddMember("types", audioInputTypesElement, document.GetAllocator());
        aaceGStreamerElement.AddMember("AudioInputProvider", audioInputProviderElement, document.GetAllocator());

        // Audio Output configuration
        rapidjson::Value audioOutputProviderElement(rapidjson::kObjectType);
        rapidjson::Value audioOutputDevicesElement(rapidjson::kObjectType);
        rapidjson::Value audioOutputTypesElement(rapidjson::kObjectType);

        audioOutputProviderElement.AddMember("enabled", rapidjson::Value().SetBool(true), document.GetAllocator());

        if (!audioConfig.ttsOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(audioConfig.ttsOutputDevice.c_str(), audioConfig.ttsOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("tts", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("TTS", "tts", document.GetAllocator());
        }

        if (!audioConfig.musicOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.musicOutputDevice.c_str(), audioConfig.musicOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("music", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("MUSIC", "music", document.GetAllocator());
        }

        if (!audioConfig.notificationOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.notificationOutputDevice.c_str(), audioConfig.notificationOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("notification", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("NOTIFICATION", "notification", document.GetAllocator());
        }

        if (!audioConfig.alarmOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.alarmOutputDevice.c_str(), audioConfig.alarmOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("alarm", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("ALARM", "alarm", document.GetAllocator());
        }

        if (!audioConfig.earconOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.earconOutputDevice.c_str(), audioConfig.earconOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("earcon", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("EARCON", "earcon", document.GetAllocator());
        }

        if (!audioConfig.communicationOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.communicationOutputDevice.c_str(), audioConfig.communicationOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("communication", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("COMMUNICATION", "communication", document.GetAllocator());
        }

        if (!audioConfig.ringtoneOutputDevice.empty()) {
            rapidjson::Value audioOutputDeviceElement(rapidjson::kObjectType);
            audioOutputDeviceElement.AddMember(
                "card",
                rapidjson::Value().SetString(
                    audioConfig.ringtoneOutputDevice.c_str(), audioConfig.ringtoneOutputDevice.length()),
                document.GetAllocator());
            audioOutputDevicesElement.AddMember("ringtone", audioOutputDeviceElement, document.GetAllocator());
            audioOutputTypesElement.AddMember("RINGTONE", "ringtone", document.GetAllocator());
        }

        audioOutputProviderElement.AddMember("devices", audioOutputDevicesElement, document.GetAllocator());
        audioOutputProviderElement.AddMember("types", audioOutputTypesElement, document.GetAllocator());
        aaceGStreamerElement.AddMember("AudioOutputProvider", audioOutputProviderElement, document.GetAllocator());

        document.AddMember("aace.systemAudio", aaceGStreamerElement, document.GetAllocator());

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

    if (topic == TOPIC_CARCONTROL) {
        m_logger->log(Level::INFO, TAG, "Received Car Control Event: " + action + " payload: " + payload);
        if (m_carControlHandler) {
            m_carControlHandler->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::WARN, TAG, "CarControl not enabled");
        }
        return;
    }

    AASB_ERROR("UKNOWN TOPIC %s", topic.c_str());
}

bool AASBControllerImpl::registerAudioPlayerCapability() {
    if (!m_engine->registerPlatformInterface(
            m_audioPlayerHandler = aasb::alexa::AudioPlayerHandler::create(m_logger))) {
        AASB_ERROR("Failed to register audio player handler");
        return false;
    }

    return true;
}

bool AASBControllerImpl::registerSpeechSynthesizerCapability() {
    if (!m_engine->registerPlatformInterface(
            m_speechSynthesizerHandler = aasb::alexa::SpeechSynthesizerHandler::create())) {
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

    if (!m_engine->registerPlatformInterface(
            m_speechRecognizer =
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

    if (localMediaSourceConfig.hasSiriusXM) {
        AASB_INFO("Registering SiriusXM source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getSiriusXMSource())) {
            AASB_ERROR("Failed to register SiriusXM source");
            return false;
        }
    }

    if (localMediaSourceConfig.hasDAB) {
        AASB_INFO("Registering DAB source");
        if (!m_engine->registerPlatformInterface(m_LocalMediaSourceHandlerManager->getDABSource())) {
            AASB_ERROR("Failed to register DAB source");
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