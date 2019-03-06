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
    AFB_ApiT api) {
    return std::shared_ptr<AASBConfigProviderImpl>(new AASBConfigProviderImpl(logger, api));
}

AASBConfigProviderImpl::AASBConfigProviderImpl(std::shared_ptr<agl::common::interfaces::ILogger> logger, AFB_ApiT api) :
        m_logger(logger),
        m_api(api),
        m_enableWakewordByDefault(false),
        m_enableCBL(false) {
    std::string alexaConfigFile = getDataRootPath() + ALEXA_CONFIG_FILE_NAME;
    initConfigFromFile(alexaConfigFile);
    logCurrentConfiguration();
}

AASBConfigProviderImpl::AudioIOConfiguration AASBConfigProviderImpl::getAudioIOConfig() {
    AudioIOConfiguration audioConfig;
    audioConfig.audioInputDevice = m_inputDevice;
    audioConfig.audioPlayerDevice = m_audioPlayerDevice;
    audioConfig.platformData = m_api;
    audioConfig.speechSynthesizerDevice = m_speechSynthesizerDevice;
    audioConfig.usePlatformAudioIO = true;
    return audioConfig;
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
        if (root.HasMember("aace.audio") && root["aace.audio"].IsObject()) {
            auto audioConfig = root["aace.audio"].GetObject();
            if (audioConfig.HasMember("speechRecognizer") && audioConfig["speechRecognizer"].IsString()) {
                m_inputDevice = audioConfig["speechRecognizer"].GetString();
            }
            if (audioConfig.HasMember("speechSynthesizer") && audioConfig["speechSynthesizer"].IsString()) {
                m_speechSynthesizerDevice = audioConfig["speechSynthesizer"].GetString();
            }
            if (audioConfig.HasMember("audioPlayer") && audioConfig["audioPlayer"].IsString()) {
                m_audioPlayerDevice = audioConfig["audioPlayer"].GetString();
            }
        }
        if (root.HasMember("aace.cbl") && root["aace.cbl"].IsObject()) {
            auto cblConfig = root["aace.cbl"].GetObject();
            if (cblConfig.HasMember("enabled") && cblConfig["enabled"].IsBool()) {
                m_enableCBL = cblConfig["enabled"].GetBool();
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
    }
}

// GetBindingDirPath() method provided by AGL SDK crashes every single time.
// It turns out that on latest AGL platforms, GetBindingDirPath(AFB_ApiT) version
// is supposed to be the correct version. However when we include filescan-utils.h
// it compiles a version without "AFB_ApiT" parameter. For now, I have made a
// copy of this method here which accepts "AFB_ApiT" parameter.
// TODO: Fix it
std::string GetBindingDirectoryPath(AFB_ApiT api) {
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
    m_logger->log(Level::DEBUG, TAG, std::string("Audio input device ") + m_inputDevice);
    m_logger->log(Level::DEBUG, TAG, std::string("Audio output (Speech) device ") + m_speechSynthesizerDevice);
    m_logger->log(Level::DEBUG, TAG, std::string("Audio output (Media) device ") + m_audioPlayerDevice);
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
    m_logger->log(Level::DEBUG, TAG, "**********Alexa-VoiceAgent configuration***********");
}

}  // namespace alexa
}  // namespace agl