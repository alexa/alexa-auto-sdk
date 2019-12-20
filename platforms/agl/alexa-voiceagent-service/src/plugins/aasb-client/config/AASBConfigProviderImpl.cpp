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

#include "AASBConfigProviderImpl.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <filescan-utils.h>

#include <aasb/Consts.h>

namespace agl {
namespace alexa {

/// Shortcut to reach logging level.
using Level = agl::common::interfaces::ILogger::Level;

/// Logging tag for this file.
static std::string TAG = "agl::alexa::AASBConfigProviderImpl";

/// File name where alexa json configuration is stored.
static std::string ALEXA_CONFIG_FILE_NAME = "AlexaAutoCoreEngineConfig.json";

std::shared_ptr<AASBConfigProviderImpl> AASBConfigProviderImpl::create(
    std::shared_ptr<agl::common::interfaces::ILogger> logger,
    afb_api_t api,
    std::shared_ptr<agl::audio::Audio> audio) {
    return std::shared_ptr<AASBConfigProviderImpl>(new AASBConfigProviderImpl(logger, api, audio));
}

AASBConfigProviderImpl::AASBConfigProviderImpl(std::shared_ptr<agl::common::interfaces::ILogger> logger, afb_api_t api, std::shared_ptr<agl::audio::Audio> audio) :
        m_logger(logger),
        m_api(api),
        m_audio(audio),
        m_enableWakewordByDefault(false),
        m_enableCBL(false),
        m_bluetooth(false),
        m_usb(false),
        m_fmRadio(false),
        m_amRadio(false),
        m_satelliteRadio(false),
        m_LineIn(false),
        m_compactDisc(false),
        m_siriusXM(false),
        m_dab(false),
        m_enableLocalMediaSource(false),
        m_enableCarControl(false),
        m_enableLocalVoiceControl(false) {
    m_LocalVoiceControlConfiguration = std::unique_ptr<LVCConfiguration>(new LVCConfiguration());
    m_carControlConfiguration = std::unique_ptr<CarControlConfiguration>(new CarControlConfiguration());
    std::string alexaConfigFile = getDataRootPath() + ALEXA_CONFIG_FILE_NAME;
    initConfigFromFile(alexaConfigFile);
    logCurrentConfiguration();
}

AASBConfigProviderImpl::LocalMediaSourceConfiguration AASBConfigProviderImpl::getLocalMediaSourceConfig() {
    LocalMediaSourceConfiguration localMediaSourceConfig;
    localMediaSourceConfig.hasBluetooth = m_bluetooth;
    localMediaSourceConfig.hasUSB = m_usb;
    localMediaSourceConfig.hasFMRadio = m_fmRadio;
    localMediaSourceConfig.hasAMRadio = m_amRadio;
    localMediaSourceConfig.hasSatelliteRadio = m_satelliteRadio;
    localMediaSourceConfig.hasLineIn = m_LineIn;
    localMediaSourceConfig.hasCompactDisc = m_compactDisc;
    localMediaSourceConfig.hasSiriusXM = m_siriusXM;
    localMediaSourceConfig.hasDAB = m_dab;
    return localMediaSourceConfig;
}

AASBConfigProviderImpl::AudioIOConfiguration AASBConfigProviderImpl::getAudioIOConfig() {
    AudioIOConfiguration audioConfig;
    audioConfig.useGStreamerAudioIO = true;

    // Input devices
    if (!m_voiceInputDevice.empty())
      audioConfig.voiceInputDevice = m_voiceInputDevice;

    if (!m_communicationInputDevice.empty())
      audioConfig.communicationInputDevice =  m_communicationInputDevice;

    if(!m_loopbackInputDevice.empty())
      audioConfig.loopbackInputDevice =  m_loopbackInputDevice;

    // Output devices
    if(!m_ttsOutputDevice.empty())
      audioConfig.ttsOutputDevice = m_audio->openAHLChannel(m_ttsOutputDevice);

    if(!m_musicOutputDevice.empty())
      audioConfig.musicOutputDevice = m_audio->openAHLChannel(m_musicOutputDevice);

    if(!m_notificationOutputDevice.empty())
      audioConfig.notificationOutputDevice = m_audio->openAHLChannel(m_notificationOutputDevice);

    if(!m_alarmOutputDevice.empty())
      audioConfig.alarmOutputDevice = m_audio->openAHLChannel(m_alarmOutputDevice);

    if(!m_earconOutputDevice.empty())
      audioConfig.earconOutputDevice = m_audio->openAHLChannel(m_earconOutputDevice);

    if(!m_communicationOutputDevice.empty())
      audioConfig.communicationOutputDevice = m_audio->openAHLChannel(m_communicationOutputDevice);

    if(!m_ringtoneOutputDevice.empty())
      audioConfig.ringtoneOutputDevice = m_audio->openAHLChannel(m_ringtoneOutputDevice);

    return audioConfig;
}

AASBConfigProviderImpl::DeviceSettingsConfiguration AASBConfigProviderImpl::getDeviceSettingsConfig() {
    return m_deviceSettingsConfiguration;
}

AASBConfigProviderImpl::LVCConfiguration AASBConfigProviderImpl::getLocalVoiceControlConfig() {
    LVCConfiguration lvcConfig;
    lvcConfig.socketRootDirectory = m_LocalVoiceControlConfiguration->socketRootDirectory;
    return lvcConfig;
}

AASBConfigProviderImpl::CarControlConfiguration AASBConfigProviderImpl::getCarControlConfig() {
    CarControlConfiguration  carControlConfig;
    carControlConfig.enabled = m_carControlConfiguration->enabled;
    carControlConfig.endpoints = m_carControlConfiguration->endpoints;
    return carControlConfig;
}

std::string AASBConfigProviderImpl::getCertificatesDirectoryPath() {
    return m_certificatePath;
}

std::string AASBConfigProviderImpl::getAppsDataDirectory() {
    return getDataRootPath();
}

std::string AASBConfigProviderImpl::getProductDSN() {
    return m_deviceSerialNumber;
}

std::string AASBConfigProviderImpl::getClientId() {
    return m_clientId;
}

std::string AASBConfigProviderImpl::getProductId() {
    return m_productId;
}

std::string AASBConfigProviderImpl::getManufacturerName() {
    return m_manufacturerName;
}

std::string AASBConfigProviderImpl::getDescription() {
    return m_description;
}

std::string AASBConfigProviderImpl::getExternalStorageDirectory() {
    return "";
}

bool AASBConfigProviderImpl::shouldEnableWakeword() {
    return m_enableWakewordByDefault;
}

bool AASBConfigProviderImpl::shouldEnablePhoneCallControl() {
    return true;
}

bool AASBConfigProviderImpl::shouldEnableNavigation() {
    return true;
}

bool AASBConfigProviderImpl::shouldEnableCBL() {
    return m_enableCBL;
}

bool AASBConfigProviderImpl::shouldEnableLocalMediaSource() {
    return m_enableLocalMediaSource;
}

bool AASBConfigProviderImpl::shouldEnableGloriaCard() {
    return false;
}

bool AASBConfigProviderImpl::shouldEnableGloriaList() {
    return false;
}

bool AASBConfigProviderImpl::shouldEnableCarControl() {
    return m_enableCarControl;
}

bool AASBConfigProviderImpl::shouldEnableLocalVoiceControl() {
    return m_enableLocalVoiceControl;
}

std::pair<float, float> AASBConfigProviderImpl::getCurrentLocation() {
    return m_currentLocation;
}

std::string AASBConfigProviderImpl::getCountry() {
    return m_country;
}

std::shared_ptr<std::istream> AASBConfigProviderImpl::getVehicleConfig() {
    return m_vehicleConfig;
}

void AASBConfigProviderImpl::initConfigFromFile(const std::string& fileName) {
    m_logger->log(Level::INFO, TAG, "Initializing configuration from: " + fileName);

    std::ifstream ifs(fileName);
    if (!ifs.fail()) {
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document document;
        document.ParseStream(isw);
        auto root = document.GetObject();
        if (root.HasMember("deviceInfo") && root["deviceInfo"].IsObject()) {
            auto deviceInfo = root["deviceInfo"].GetObject();
            if (deviceInfo.HasMember("clientId") && deviceInfo["clientId"].IsString()) {
                m_clientId = deviceInfo["clientId"].GetString();
            }
            if (deviceInfo.HasMember("deviceSerialNumber") && deviceInfo["deviceSerialNumber"].IsString()) {
                m_deviceSerialNumber = deviceInfo["deviceSerialNumber"].GetString();
            }
            if (deviceInfo.HasMember("productId") && deviceInfo["productId"].IsString()) {
                m_productId = deviceInfo["productId"].GetString();
            }
            if (deviceInfo.HasMember("manufacturerName") && deviceInfo["manufacturerName"].IsString()) {
                m_manufacturerName = deviceInfo["manufacturerName"].GetString();
            }
            if (deviceInfo.HasMember("description") && deviceInfo["description"].IsString()) {
                m_description = deviceInfo["description"].GetString();
            }
        }
        if (root.HasMember("libcurlUtils") && root["libcurlUtils"].IsObject()) {
            auto curlConfig = root["libcurlUtils"].GetObject();
            if (curlConfig.HasMember("CURLOPT_CAPATH") && curlConfig["CURLOPT_CAPATH"].IsString()) {
                m_certificatePath = curlConfig["CURLOPT_CAPATH"].GetString();
            }
        }
        if (root.HasMember("deviceSettings") && root["deviceSettings"].IsObject()) {
            auto deviceConfig = root["deviceSettings"].GetObject();
            if (deviceConfig.HasMember("defaultLocale") && deviceConfig["defaultLocale"].IsString()) {
                m_deviceSettingsConfiguration.defaultLocale = deviceConfig["defaultLocale"].GetString();
            }
            if (deviceConfig.HasMember("defaultTimezone") && deviceConfig["defaultTimezone"].IsString()) {
                m_deviceSettingsConfiguration.defaultTimezone = deviceConfig["defaultTimezone"].GetString();
            }
            if (deviceConfig.HasMember("locales") && deviceConfig["locales"].IsArray()) {
                auto locales = deviceConfig["locales"].GetArray();
                for (auto& locale : locales) {
                    if (locale.IsString()) {
                        m_deviceSettingsConfiguration.locales.push_back(locale.GetString());
                    }
                }
            }
        }
        if (root.HasMember("auth") && root["auth"].IsObject()) {
            auto authConfig = root["auth"].GetObject();
            if (authConfig.HasMember("token") && authConfig["token"].IsString()) {
                m_token = authConfig["token"].GetString();
            }
        }
        if (root.HasMember("aace.audio.input") && root["aace.audio.input"].IsObject()) {
            auto audioConfig = root["aace.audio.input"].GetObject();
            if (audioConfig.HasMember("voice") && audioConfig["voice"].IsString()) {
                m_voiceInputDevice = audioConfig["voice"].GetString();
            }
            if (audioConfig.HasMember("communication") && audioConfig["communication"].IsString()) {
                m_communicationInputDevice = audioConfig["communication"].GetString();
            }
            if (audioConfig.HasMember("loopback") && audioConfig["loopback"].IsString()) {
                m_loopbackInputDevice = audioConfig["loopback"].GetString();
            }
        }
        if (root.HasMember("aace.audio.output") && root["aace.audio.output"].IsObject()) {
            auto audioConfig = root["aace.audio.output"].GetObject();
            if (audioConfig.HasMember("tts") && audioConfig["tts"].IsString()) {
                m_ttsOutputDevice = audioConfig["tts"].GetString();
            }
            if (audioConfig.HasMember("music") && audioConfig["music"].IsString()) {
                m_musicOutputDevice = audioConfig["music"].GetString();
            }
            if (audioConfig.HasMember("notification") && audioConfig["notification"].IsString()) {
                m_notificationOutputDevice = audioConfig["notification"].GetString();
            }
            if (audioConfig.HasMember("alarm") && audioConfig["alarm"].IsString()) {
                m_alarmOutputDevice = audioConfig["alarm"].GetString();
            }
            if (audioConfig.HasMember("earcon") && audioConfig["earcon"].IsString()) {
                m_earconOutputDevice = audioConfig["earcon"].GetString();
            }
            if (audioConfig.HasMember("communication") && audioConfig["communication"].IsString()) {
                m_communicationOutputDevice = audioConfig["communication"].GetString();
            }
            if (audioConfig.HasMember("ringtone") && audioConfig["ringtone"].IsString()) {
                m_ringtoneOutputDevice = audioConfig["ringtone"].GetString();
            }
        }
        if (root.HasMember("aace.cbl") && root["aace.cbl"].IsObject()) {
            auto cblConfig = root["aace.cbl"].GetObject();
            if (cblConfig.HasMember("enabled") && cblConfig["enabled"].IsBool()) {
                m_enableCBL = cblConfig["enabled"].GetBool();
            }
        }
        if (root.HasMember("aace.localskills") && root["aace.localskills"].IsObject()) {
            auto localSkillsConfig = root["aace.localskills"].GetObject();
            if (localSkillsConfig.HasMember("aace.carcontrol") && localSkillsConfig["aace.carcontrol"].IsObject()) {
                auto carControlConfig = localSkillsConfig["aace.carcontrol"].GetObject();
                if (carControlConfig.HasMember("enabled") && carControlConfig["enabled"].IsBool()) {
                    m_enableCarControl = carControlConfig["enabled"].GetBool();
                }
                if (carControlConfig.HasMember("endpoints") && carControlConfig["endpoints"].IsArray()) {
                    auto endpointsConfig = carControlConfig["endpoints"].GetArray();
                    for (auto& endpoint : endpointsConfig) {
                        if (endpoint.IsObject()) {
                            auto endpointValue = endpoint.GetObject();
                            CarControlConfiguration::Endpoint endpointMember;
                            if (endpointValue.HasMember("enabled") && endpointValue["enabled"].IsBool()) {
                                if (!endpointValue["enabled"].GetBool())
                                    continue;
                            }
                            if (endpointValue.HasMember("zone") && endpointValue["zone"].IsString()) {
                                endpointMember.zone = endpointValue["zone"].GetString();
                            }
                            if (endpointValue.HasMember("climate") && endpointValue["climate"].IsObject()) {
                                auto climate = endpointValue["climate"].GetObject();
                                endpointMember.climate.enabled = true;
                                if (climate.HasMember("controlId") && climate["controlId"].IsString()) {
                                   endpointMember.climate.controlId = climate["controlId"].GetString();
                                }
                                if (climate.HasMember("sync") && climate["sync"].IsObject()) {
                                    auto sync = climate["sync"].GetObject();
                                    if (sync.HasMember("controllerId") && sync["controllerId"].IsString()) {
                                        endpointMember.climate.syncControllerId = sync["controllerId"].GetString();
                                    }
                                }
                                if (climate.HasMember("recirculate") && climate["recirculate"].IsObject()) {
                                    auto recirculate = climate["recirculate"].GetObject();
                                    if (recirculate.HasMember("controllerId") && recirculate["controllerId"].IsString()) {
                                        endpointMember.climate.recirculationControllerId = recirculate["controllerId"].GetString();
                                    }
                                }
                            }
                            if (endpointValue.HasMember("airconditioner") && endpointValue["airconditioner"].IsObject()) {
                                auto airConditioner = endpointValue["airconditioner"].GetObject();
                                endpointMember.airConditioner.enabled = true;
                                if (airConditioner.HasMember("controlId") && airConditioner["controlId"].IsString()) {
                                    endpointMember.airConditioner.controlId = airConditioner["controlId"].GetString();
                                }
                                if (airConditioner.HasMember("mode") && airConditioner["mode"].IsObject()) {
                                    auto modeConfig = airConditioner["mode"].GetObject();
                                    if (modeConfig.HasMember("controllerId") && modeConfig["controllerId"].IsString()) {
                                        endpointMember.airConditioner.modeControllerId = modeConfig["controllerId"].GetString();
                                    }
                                    if (modeConfig.HasMember("values") && modeConfig["values"].IsObject()) {
                                        auto valuesConfig = modeConfig["values"].GetObject();
                                        if (valuesConfig.HasMember("AUTO") && valuesConfig["AUTO"].IsBool()) {
                                            if (valuesConfig["AUTO"].GetBool()) {
                                                endpointMember.airConditioner.modeValues.push_back("AUTO");
                                            }
                                        }
                                        if (valuesConfig.HasMember("ECONOMY") && valuesConfig["ECONOMY"].IsBool()) {
                                            if (valuesConfig["ECONOMY"].GetBool()) {
                                                endpointMember.airConditioner.modeValues.push_back("ECONOMY");
                                            }
                                        }
                                        if (valuesConfig.HasMember("MANUAL") && valuesConfig["MANUAL"].IsBool()) {
                                            if (valuesConfig["MANUAL"].GetBool()) {
                                                endpointMember.airConditioner.modeValues.push_back("MANUAL");
                                            }
                                        }
                                    }
                                }
                                if (airConditioner.HasMember("intensity") && airConditioner["intensity"].IsObject()) {
                                    auto intensityConfig = airConditioner["intensity"].GetObject();
                                    if (intensityConfig.HasMember("controllerId") && intensityConfig["controllerId"].IsString()) {
                                        endpointMember.airConditioner.intensityControllerId = intensityConfig["controllerId"].GetString();
                                    }
                                    if (intensityConfig.HasMember("values") && intensityConfig["values"].IsObject()) {
                                        auto valuesConfig = intensityConfig["values"].GetObject();
                                        if (valuesConfig.HasMember("LOW") && valuesConfig["LOW"].IsBool()) {
                                            if (valuesConfig["LOW"].GetBool()) {
                                                endpointMember.airConditioner.intensityValues.push_back("LOW");
                                            }
                                        }
                                        if (valuesConfig.HasMember("MEDIUM") && valuesConfig["MEDIUM"].IsBool()) {
                                            if (valuesConfig["MEDIUM"].GetBool()) {
                                                endpointMember.airConditioner.intensityValues.push_back("MEDIUM");
                                            }
                                        }
                                        if (valuesConfig.HasMember("HIGH") && valuesConfig["HIGH"].IsBool()) {
                                            if (valuesConfig["HIGH"].GetBool()) {
                                                endpointMember.airConditioner.intensityValues.push_back("HIGH");
                                            }
                                        }
                                    }
                                }
                            }
                            if (endpointValue.HasMember("heater") && endpointValue["heater"].IsObject()) {
                                auto heater = endpointValue["heater"].GetObject();
                                endpointMember.heater.enabled = true;
                                if (heater.HasMember("controlId") && heater["controlId"].IsString()) {
                                    endpointMember.heater.controlId = heater["controlId"].GetString();
                                }
                                if (heater.HasMember("controllerId") && heater["controllerId"].IsString()) {
                                    endpointMember.heater.controllerId = heater["controllerId"].GetString();
                                }
                                if (heater.HasMember("minimum") && heater["minimum"].IsNumber()) {
                                    endpointMember.heater.minimum = heater["minimum"].GetDouble();
                                }
                                if (heater.HasMember("maximum") && heater["maximum"].IsNumber()) {
                                    endpointMember.heater.maximum = heater["maximum"].GetDouble();
                                }
                                if (heater.HasMember("precision") && heater["precision"].IsNumber()) {
                                    endpointMember.heater.precision = heater["precision"].GetDouble();
                                }
                                if (heater.HasMember("unit") && heater["unit"].IsString()) {
                                    endpointMember.heater.unit = heater["unit"].GetString();
                                }
                            }
                            if (endpointValue.HasMember("fan") && endpointValue["fan"].IsObject()) {
                                auto fan = endpointValue["fan"].GetObject();
                                endpointMember.fan.enabled = true;
                                if (fan.HasMember("controlId") && fan["controlId"].IsString()) {
                                    endpointMember.fan.controlId = fan["controlId"].GetString();
                                }
                                if (fan.HasMember("controllerId") && fan["controllerId"].IsString()) {
                                    endpointMember.fan.controllerId = fan["controllerId"].GetString();
                                }
                                if (fan.HasMember("minimum") && fan["minimum"].IsNumber()) {
                                    endpointMember.fan.minimum = fan["minimum"].GetDouble();
                                }
                                if (fan.HasMember("maximum") && fan["maximum"].IsNumber()) {
                                    endpointMember.fan.maximum = fan["maximum"].GetDouble();
                                }
                                if (fan.HasMember("precision") && fan["precision"].IsNumber()) {
                                    endpointMember.fan.precision = fan["precision"].GetDouble();
                                }
                            }
                            if (endpointValue.HasMember("vent") && endpointValue["vent"].IsObject()) {
                                auto ventConfig = endpointValue["vent"].GetObject();
                                endpointMember.vent.enabled = true;
                                if (ventConfig.HasMember("controlId") && ventConfig["controlId"].IsString()) {
                                    endpointMember.vent.controlId = ventConfig["controlId"].GetString();
                                }
                                if (ventConfig.HasMember("positions") && ventConfig["positions"].IsObject()) {
                                    auto positionsConfig = ventConfig["positions"].GetObject();
                                    if (positionsConfig.HasMember("controllerId") && positionsConfig["controllerId"].IsString()) {
                                        endpointMember.vent.positionsControllerId = positionsConfig["controllerId"].GetString();
                                    }
                                    if (positionsConfig.HasMember("values") && positionsConfig["values"].IsObject()) {
                                        auto valuesConfig = positionsConfig["values"].GetObject();
                                        if (valuesConfig.HasMember("BODY") && valuesConfig["BODY"].IsBool()) {
                                            if (valuesConfig["BODY"].GetBool()) {
                                                endpointMember.vent.positions.push_back("BODY");
                                            }
                                        }
                                        if (valuesConfig.HasMember("MIX") && valuesConfig["MIX"].IsBool()) {
                                            if (valuesConfig["MIX"].GetBool()) {
                                                endpointMember.vent.positions.push_back("MIX");
                                            }
                                        }
                                        if (valuesConfig.HasMember("FLOOR") && valuesConfig["FLOOR"].IsBool()) {
                                            if (valuesConfig["FLOOR"].GetBool()) {
                                                endpointMember.vent.positions.push_back("FLOOR");
                                            }
                                        }
                                        if (valuesConfig.HasMember("WINDSHIELD") && valuesConfig["WINDSHIELD"].IsBool()) {
                                            if (valuesConfig["WINDSHIELD"].GetBool()) {
                                                endpointMember.vent.positions.push_back("WINDSHIELD");
                                            }
                                        }
                                    }
                                }
                            }
                            if (endpointValue.HasMember("window") && endpointValue["window"].IsObject()) {
                                auto window = endpointValue["window"].GetObject();
                                endpointMember.window.enabled = true;
                                if (window.HasMember("controlId") && window["controlId"].IsString()) {
                                    endpointMember.window.controlId = window["controlId"].GetString();
                                }
                                if (window.HasMember("controllerId") && window["controllerId"].IsString()) {
                                    endpointMember.window.controllerId = window["controllerId"].GetString();
                                }
                                if (window.HasMember("defrost") && window["defrost"].IsBool()) {
                                    endpointMember.window.defrost = window["defrost"].GetBool();
                                }
                            }
                            if (endpointValue.HasMember("lights") && endpointValue["lights"].IsArray()) {
                                auto lightConfig = endpointValue["lights"].GetArray();
                                for (auto& light : lightConfig) {
                                    CarControlConfiguration::Endpoint::Light lightMember;
                                    if (light.HasMember("controlId") && light["controlId"].IsString()) {
                                        lightMember.controlId = light["controlId"].GetString();
                                    }
                                    if (light.HasMember("type") && light["type"].IsString()) {
                                        lightMember.type = light["type"].GetString();
                                    }
                                    if (light.HasMember("color") && light["color"].IsObject()) {
                                        auto colorConfig = light["color"].GetObject();
                                        if (colorConfig.HasMember("controllerId") && colorConfig["controllerId"].IsString()) {
                                            lightMember.colorControllerId = colorConfig["controllerId"].GetString();
                                        }
                                        if (colorConfig.HasMember("values") && colorConfig["values"].IsObject()) {
                                            auto valuesConfig = colorConfig["values"].GetObject();
                                            if (valuesConfig.HasMember("WHITE") && valuesConfig["WHITE"].IsBool()) {
                                                if (valuesConfig["WHITE"].GetBool()) {
                                                    lightMember.colors.push_back("WHITE");
                                                }
                                            }
                                            if (valuesConfig.HasMember("RED") && valuesConfig["RED"].IsBool()) {
                                                if (valuesConfig["RED"].GetBool()) {
                                                    lightMember.colors.push_back("RED");
                                                }
                                            }
                                            if (valuesConfig.HasMember("ORANGE") && valuesConfig["ORANGE"].IsBool()) {
                                                if (valuesConfig["ORANGE"].GetBool()) {
                                                    lightMember.colors.push_back("ORANGE");
                                                }
                                            }
                                            if (valuesConfig.HasMember("YELLOW") && valuesConfig["YELLOW"].IsBool()) {
                                                if (valuesConfig["YELLOW"].GetBool()) {
                                                    lightMember.colors.push_back("YELLOW");
                                                }
                                            }
                                            if (valuesConfig.HasMember("GREEN") && valuesConfig["GREEN"].IsBool()) {
                                                if (valuesConfig["GREEN"].GetBool()) {
                                                    lightMember.colors.push_back("GREEN");
                                                }
                                            }
                                            if (valuesConfig.HasMember("BLUE") && valuesConfig["BLUE"].IsBool()) {
                                                if (valuesConfig["BLUE"].GetBool()) {
                                                    lightMember.colors.push_back("BLUE");
                                                }
                                            }
                                            if (valuesConfig.HasMember("INDIGO") && valuesConfig["INDIGO"].IsBool()) {
                                                if (valuesConfig["INDIGO"].GetBool()) {
                                                    lightMember.colors.push_back("INDIGO");
                                                }
                                            }
                                            if (valuesConfig.HasMember("VIOLET") && valuesConfig["VIOLET"].IsBool()) {
                                                if (valuesConfig["VIOLET"].GetBool()) {
                                                    lightMember.colors.push_back("VIOLET");
                                                }
                                            }
                                        }
                                    }
                                    // Add light
                                    endpointMember.lights.push_back(lightMember);
                                }
                            }
                            // Add this endpoint to the configuration
                            m_carControlConfiguration->endpoints.push_back(endpointMember);
                        }
                    }
                }
            }
        }
        if (root.HasMember("aace.localmediasource") && root["aace.localmediasource"].IsObject()) {
            auto localMediaSourceConfig = root["aace.localmediasource"].GetObject();
            if (localMediaSourceConfig.HasMember("enabled") && localMediaSourceConfig["enabled"].IsBool()) {
                m_enableLocalMediaSource = localMediaSourceConfig["enabled"].GetBool();
            }

            if (localMediaSourceConfig.HasMember("sources") && localMediaSourceConfig["sources"].IsObject()) {
                auto sources = localMediaSourceConfig["sources"].GetObject();
                if (sources.HasMember("bluetooth") && sources["bluetooth"].IsBool()) {
                    m_bluetooth = sources["bluetooth"].GetBool();
                }
                if (sources.HasMember("usb") && sources["usb"].IsBool()) {
                    m_usb = sources["usb"].GetBool();
                }
                if (sources.HasMember("fmRadio") && sources["fmRadio"].IsBool()) {
                    m_fmRadio = sources["fmRadio"].GetBool();
                }
                if (sources.HasMember("amRadio") && sources["amRadio"].IsBool()) {
                    m_amRadio = sources["amRadio"].GetBool();
                }
                if (sources.HasMember("satelliteRadio") && sources["satelliteRadio"].IsBool()) {
                    m_satelliteRadio = sources["satelliteRadio"].GetBool();
                }
                if (sources.HasMember("lineIn") && sources["lineIn"].IsBool()) {
                    m_LineIn = sources["lineIn"].GetBool();
                }
                if (sources.HasMember("compactDisc") && sources["compactDisc"].IsBool()) {
                    m_compactDisc = sources["compactDisc"].GetBool();
                }
                if (sources.HasMember("siriusXM") && sources["siriusXM"].IsBool()) {
                    m_siriusXM = sources["siriusXM"].GetBool();
                }
                if (sources.HasMember("dab") && sources["dab"].IsBool()) {
                    m_dab = sources["dab"].GetBool();
                }
            }
        }
        if (root.HasMember("aace.wakeword") && root["aace.wakeword"].IsObject()) {
            auto wakewordConfig = root["aace.wakeword"].GetObject();
            if (wakewordConfig.HasMember("enabled") && wakewordConfig["enabled"].IsBool()) {
                m_enableWakewordByDefault = wakewordConfig["enabled"].GetBool();
            }
        }
        if (root.HasMember("aace.vehicle") && root["aace.vehicle"].IsObject()) {
            auto vehicleConfig = root["aace.vehicle"].GetObject();
            if (vehicleConfig.HasMember("operatingCountry") && vehicleConfig["operatingCountry"].IsString()) {
                m_country = vehicleConfig["operatingCountry"].GetString();
            }
            if (vehicleConfig.HasMember("currentLocation") && vehicleConfig["currentLocation"].IsString()) {
                auto locationStr = std::string(vehicleConfig["currentLocation"].GetString());
                auto commaPos = locationStr.find(",");
                if (commaPos != std::string::npos & commaPos < locationStr.length()) {
                    auto lonStr = locationStr.substr(0, commaPos);
                    auto latStr = locationStr.substr(commaPos + 1);
                    m_currentLocation.first = std::stof(lonStr);
                    m_currentLocation.second = std::stof(latStr);
                }
            }

            // Create stream with only vehicle configuration
            rapidjson::Document vehicleDoc;
            vehicleDoc.SetObject();
            rapidjson::Value aaceVehicleNode( rapidjson::kObjectType );
            vehicleDoc.AddMember( "aace.vehicle", vehicleConfig, vehicleDoc.GetAllocator() );

            // Create string to pass to string stream
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            vehicleDoc.Accept(writer);

            m_vehicleConfig = std::make_shared<std::istringstream>(buffer.GetString());
        }
        if (root.HasMember("aace.localvoicecontrol") && root["aace.localvoicecontrol"].IsObject()) {
            auto lvcConfig = root["aace.localvoicecontrol"].GetObject();
            if (lvcConfig.HasMember("enabled") && lvcConfig["enabled"].IsBool()) {
                m_enableLocalVoiceControl = lvcConfig["enabled"].GetBool();
            }
            if (lvcConfig.HasMember("socketRootDirectory") && lvcConfig["socketRootDirectory"].IsString()) {
                m_LocalVoiceControlConfiguration->socketRootDirectory = lvcConfig["socketRootDirectory"].GetString();
            }
        }
    }
}

// GetBindingDirPath() method provided by AGL SDK crashes every single time.
// It turns out that on latest AGL platforms, GetBindingDirPath(afb_api_t) version
// is supposed to be the correct version. However when we include filescan-utils.h
// it compiles a version without "afb_api_t" parameter. For now, I have made a
// copy of this method here which accepts "afb_api_t" parameter.
// TODO: Fix it
std::string GetBindingDirectoryPath(afb_api_t api) {
    // A file description should not be greater than 999.999.999
    char fd_link[CONTROL_MAXPATH_LEN];
    char retdir[CONTROL_MAXPATH_LEN];
    ssize_t len;
    sprintf(fd_link, "/proc/self/fd/%d", afb_dynapi_rootdir_get_fd(api));

    if ((len = readlink(fd_link, retdir, sizeof(retdir) - 1)) == -1) {
        perror("lstat");
        strncpy(retdir, "/tmp", CONTROL_MAXPATH_LEN - 1);
    } else {
       retdir[len] = '\0';
    }

    return std::string(retdir);
}

std::string AASBConfigProviderImpl::getDataRootPath() {
    std::string bindingDir(GetBindingDirectoryPath(m_api));
    return bindingDir + "/var/config/";
}

void AASBConfigProviderImpl::logCurrentConfiguration() {
    std::stringstream locationStr;
    locationStr << m_currentLocation.first << "," << m_currentLocation.second;

    m_logger->log(Level::DEBUG, TAG, "**********Alexa-VoiceAgent configuration***********");
    m_logger->log(Level::DEBUG, TAG, std::string("Product Id ") + m_productId);
    m_logger->log(Level::DEBUG, TAG, std::string("Client Id ") + m_clientId);
    m_logger->log(Level::DEBUG, TAG, std::string("Device SerialNumber ") + m_deviceSerialNumber);
    m_logger->log(Level::DEBUG, TAG, std::string("Manufacturer Name ") + m_manufacturerName);
    m_logger->log(Level::DEBUG, TAG, std::string("Description") + m_description);
    m_logger->log(Level::DEBUG, TAG, std::string("Certificate(s) root ") + m_certificatePath);
    m_logger->log(Level::DEBUG, TAG, std::string("Country ") + m_country);
    m_logger->log(Level::DEBUG, TAG, std::string("Current location ") + locationStr.str());
    m_logger->log(Level::DEBUG, TAG, std::string("Feature: Wakeword: ") +
        (shouldEnableWakeword() ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Feature: PhoneCall: ") +
        (shouldEnablePhoneCallControl() ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Feature: Navigation: ") +
        (shouldEnableNavigation() ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Feature: CBL: ") +
        (shouldEnableCBL() ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("BT Source: ") +
        (m_bluetooth ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("USB Source: ") +
        (m_usb ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("FM Radio: ") +
        (m_fmRadio ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("AM Radio: ") +
        (m_amRadio ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Satellite Radio: ") +
        (m_satelliteRadio ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Line In: ") +
        (m_LineIn ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Compact Disc: ") +
        (m_compactDisc ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("Sirius XM: ") +
        (m_siriusXM ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, std::string("DAB: ") +
        (m_dab ? "enabled" : "disabled"));
    m_logger->log(Level::DEBUG, TAG, "**********Alexa-VoiceAgent configuration***********");
}

}  // namespace alexa
}  // namespace agl