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

AASBConfigProviderImpl::LVCConfiguration AASBConfigProviderImpl::getLocalVoiceControlConfig() {
    LVCConfiguration lvcConfig;
    lvcConfig.socketRootDirectory = m_LocalVoiceControlConfiguration->socketRootDirectory;
    return lvcConfig;
}

AASBConfigProviderImpl::CarControlConfiguration AASBConfigProviderImpl::getCarControlConfig() {
    CarControlConfiguration  carControlConfig;
    carControlConfig.enabled = m_carControlConfiguration->enabled;
    carControlConfig.zones = m_carControlConfiguration->zones;
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
                if (carControlConfig.HasMember("cannedzones") && carControlConfig["cannedzones"].IsArray()) {
                    auto zonesConfig = carControlConfig["cannedzones"].GetArray();
                    for (auto& zone : zonesConfig) {
                        if (zone.IsObject()) {
                            auto zoneValue = zone.GetObject();
                            CarControlConfiguration::Zone zoneMember;
                            if (zoneValue.HasMember("enabled") && zoneValue["enabled"].IsBool()) {
                                if (!zoneValue["enabled"].GetBool())
                                    continue;
                            }                            
                            if (zoneValue.HasMember("name") && zoneValue["name"].IsString()) {
                                zoneMember.name = zoneValue["name"].GetString();
                            }
                            if (zoneValue.HasMember("climate") && zoneValue["climate"].IsObject()) {
                                auto climate = zoneValue["climate"].GetObject();
                                zoneMember.climate.enabled = true;
                                if (climate.HasMember("syncController") && climate["syncController"].IsBool()) {
                                   zoneMember.climate.addSyncController = climate["syncController"].GetBool();
                                }
                                if (climate.HasMember("recirculationController") && climate["recirculationController"].IsBool()) {
                                    zoneMember.climate.addRecirculationController = climate["recirculationController"].GetBool();
                                }
                            }
                            if (zoneValue.HasMember("airconditioner") && zoneValue["airconditioner"].IsObject()) {
                                auto airConditioner = zoneValue["airconditioner"].GetObject();
                                zoneMember.airConditioner.enabled = true;
                                if (airConditioner.HasMember("modes") && airConditioner["modes"].IsObject()) {
                                    auto modesConfig = airConditioner["modes"].GetObject();
                                    if (modesConfig.HasMember("AUTO") && modesConfig["AUTO"].IsBool()) { 
                                        if (modesConfig["AUTO"].GetBool()) {
                                            zoneMember.airConditioner.modes.push_back("AUTO");
                                        }
                                    }
                                    if (modesConfig.HasMember("ECONOMY") && modesConfig["ECONOMY"].IsBool()) { 
                                        if (modesConfig["ECONOMY"].GetBool()) {
                                            zoneMember.airConditioner.modes.push_back("ECONOMY");
                                        }
                                    }
                                    if (modesConfig.HasMember("MANUAL") && modesConfig["MANUAL"].IsBool()) { 
                                        if (modesConfig["MANUAL"].GetBool()) {
                                            zoneMember.airConditioner.modes.push_back("MANUAL");
                                        }
                                    }
                                    if (modesConfig.HasMember("MAXIMUM") && modesConfig["MAXIMUM"].IsBool()) { 
                                        if (modesConfig["MAXIMUM"].GetBool()) {
                                            zoneMember.airConditioner.modes.push_back("MAXIMUM");
                                        }
                                    }
                                }                                
                            }
                            if (zoneValue.HasMember("heater") && zoneValue["heater"].IsObject()) {
                                auto heater = zoneValue["heater"].GetObject();
                                zoneMember.heater.enabled = true;
                                if (heater.HasMember("minimum") && heater["minimum"].IsDouble()) {
                                    zoneMember.heater.minimum = heater["minimum"].GetDouble();
                                }
                                if (heater.HasMember("maximum") && heater["maximum"].IsDouble()) {
                                    zoneMember.heater.maximum = heater["maximum"].GetDouble();
                                }
                                if (heater.HasMember("precision") && heater["precision"].IsDouble()) {
                                    zoneMember.heater.precision = heater["precision"].GetDouble();
                                }
                                if (heater.HasMember("unit") && heater["unit"].IsString()) {
                                    zoneMember.heater.unit = heater["unit"].GetString();
                                }                                
                            }
                            if (zoneValue.HasMember("fan") && zoneValue["fan"].IsObject()) {
                                auto fan = zoneValue["fan"].GetObject();
                                zoneMember.fan.enabled = true;
                                if (fan.HasMember("minimum") && fan["minimum"].IsDouble()) {
                                    zoneMember.fan.minimum = fan["minimum"].GetDouble();
                                }
                                if (fan.HasMember("maximum") && fan["maximum"].IsDouble()) {
                                    zoneMember.fan.maximum = fan["maximum"].GetDouble();
                                }
                                if (fan.HasMember("precision") && fan["precision"].IsDouble()) {
                                    zoneMember.fan.precision = fan["precision"].GetDouble();
                                }
                            }
                            if (zoneValue.HasMember("vent") && zoneValue["vent"].IsObject()) {
                                auto ventsConfig = zoneValue["vent"].GetObject();
                                zoneMember.vent.enabled = true;
                                if (ventsConfig.HasMember("positions") && ventsConfig["positions"].IsObject()) {
                                    auto positions = ventsConfig["positions"].GetObject();
                                    if (positions.HasMember("BODY") && positions["BODY"].IsBool()) { 
                                        if (positions["BODY"].GetBool()) {
                                            zoneMember.vent.positions.push_back("BODY");
                                        }
                                    }
                                    if (positions.HasMember("MIX") && positions["MIX"].IsBool()) { 
                                        if (positions["MIX"].GetBool()) {
                                            zoneMember.vent.positions.push_back("MIX");
                                        }
                                    }
                                    if (positions.HasMember("FLOOR") && positions["FLOOR"].IsBool()) { 
                                        if (positions["FLOOR"].GetBool()) {
                                            zoneMember.vent.positions.push_back("FLOOR");
                                        }
                                    }
                                    if (positions.HasMember("WINDSHIELD") && positions["WINDSHIELD"].IsBool()) { 
                                        if (positions["WINDSHIELD"].GetBool()) {
                                            zoneMember.vent.positions.push_back("WINDSHIELD");
                                        }
                                    }
                                }
                            }
                            if (zoneValue.HasMember("window") && zoneValue["window"].IsObject()) {
                                auto window = zoneValue["window"].GetObject();
                                zoneMember.window.enabled = true;
                                if (window.HasMember("defrost") && window["defrost"].IsBool()) {
                                    zoneMember.window.defrost = window["defrost"].GetBool();
                                }
                            }
                            if (zoneValue.HasMember("light") && zoneValue["light"].IsObject()) {
                                auto light = zoneValue["light"].GetObject();
                                zoneMember.light.enabled = true;

                                if (light.HasMember("type") && light["type"].IsString()) {
                                    zoneMember.light.type = light["type"].GetString();
                                }

                                if (light.HasMember("colors") && light["colors"].IsObject()) {
                                    auto colorsConfig = light["colors"].GetObject();
                                    if (colorsConfig.HasMember("WHITE") && colorsConfig["WHITE"].IsBool()) { 
                                        if (colorsConfig["WHITE"].GetBool()) {
                                            zoneMember.light.colors.push_back("WHITE");
                                        }
                                    }
                                    if (colorsConfig.HasMember("RED") && colorsConfig["RED"].IsBool()) { 
                                        if (colorsConfig["RED"].GetBool()) {
                                            zoneMember.light.colors.push_back("RED");
                                        }
                                    }
                                    if (colorsConfig.HasMember("ORANGE") && colorsConfig["ORANGE"].IsBool()) { 
                                        if (colorsConfig["ORANGE"].GetBool()) {
                                            zoneMember.light.colors.push_back("ORANGE");
                                        }
                                    }
                                    if (colorsConfig.HasMember("YELLOW") && colorsConfig["YELLOW"].IsBool()) { 
                                        if (colorsConfig["YELLOW"].GetBool()) {
                                            zoneMember.light.colors.push_back("YELLOW");
                                        }
                                    }
                                    if (colorsConfig.HasMember("GREEN") && colorsConfig["GREEN"].IsBool()) { 
                                        if (colorsConfig["GREEN"].GetBool()) {
                                            zoneMember.light.colors.push_back("GREEN");
                                        }
                                    }
                                    if (colorsConfig.HasMember("BLUE") && colorsConfig["BLUE"].IsBool()) { 
                                        if (colorsConfig["BLUE"].GetBool()) {
                                            zoneMember.light.colors.push_back("BLUE");
                                        }
                                    }
                                    if (colorsConfig.HasMember("INDIGO") && colorsConfig["INDIGO"].IsBool()) { 
                                        if (colorsConfig["INDIGO"].GetBool()) {
                                            zoneMember.light.colors.push_back("INDIGO");
                                        }
                                    }
                                    if (colorsConfig.HasMember("VIOLET") && colorsConfig["VIOLET"].IsBool()) { 
                                        if (colorsConfig["VIOLET"].GetBool()) {
                                            zoneMember.light.colors.push_back("VIOLET");
                                        }
                                    }
                                }                                
                            }                            
                            // Add this zone to the configuration
                            m_carControlConfiguration->zones.push_back(zoneMember);
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
            }
        }        
        if (root.HasMember("aace.wakeword") && root["aace.wakeword"].IsObject()) {
            auto wakewordConfig = root["aace.wakeword"].GetObject();
            if (wakewordConfig.HasMember("enabled") && wakewordConfig["enabled"].IsBool()) {
                m_enableWakewordByDefault = wakewordConfig["enabled"].GetBool();
            }
        }
        if (root.HasMember("libcurlUtils") && root["libcurlUtils"].IsObject()) {
            auto curlConfig = root["libcurlUtils"].GetObject();
            if (curlConfig.HasMember("CURLOPT_CAPATH") && curlConfig["CURLOPT_CAPATH"].IsString()) {
                m_certificatePath = curlConfig["CURLOPT_CAPATH"].GetString();
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
    m_logger->log(Level::DEBUG, TAG, "**********Alexa-VoiceAgent configuration***********");
}

}  // namespace alexa
}  // namespace agl