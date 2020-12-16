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

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#ifdef __linux__
#include <linux/limits.h>  // PATH_MAX
#endif
#include <array>    // std::array
#include <cstddef>  // std::size_t
#include <fstream>  // std::ofstream
#include <regex>    // std::regex
#include <stdio.h>  // std::stdio

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ApplicationContext
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ApplicationContext::ApplicationContext(const std::string& path) {
    char buffer[PATH_MAX];
    m_applicationPath = std::string(realpath(path.c_str(), buffer));
    auto pos = m_applicationPath.find_last_of('/');
    if (pos != std::string::npos) {
        // dir path does not include trailing slash
        m_applicationDirPath = m_applicationPath.substr(0, pos);
    }

    m_menuRegister = json::object();
}

void ApplicationContext::addAudioFilePath(const std::string& audioFilePath) {
    m_audioFilePaths.push_back(audioFilePath);
    m_testAutomation = true;
}

void ApplicationContext::addConfigFilePath(const std::string& configFilePath) {
    m_configFilePaths.push_back(configFilePath);
}

void ApplicationContext::addMenuFilePath(const std::string& menuFilePath) {
    m_menuFilePaths.push_back(menuFilePath);
}

bool ApplicationContext::checkDcmConfiguration(const std::vector<json>& configs) {
    for (auto const& j : configs) {
        try {
            auto obj = j.at("aace.dcm");
            if (obj.is_object()) {
                return true;
            }
        } catch (json::exception& e) {
        }
    }
    return false;
}

void ApplicationContext::clearLevel() {
    m_logEnabled = false;
}

void ApplicationContext::clearUserConfigFilePath() {
    m_userConfigFilePath.clear();
}

std::string ApplicationContext::executeCommand(const char* command) {
    std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe || !pipe.get()) {
        return "Error: popen() failed";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string ApplicationContext::getApplicationDirPath() {
    return m_applicationDirPath;
}

std::string ApplicationContext::getApplicationPath() {
    return m_applicationPath;
}

std::string ApplicationContext::getAudioInputDevice() {
    return m_audioInputDevice;
}

std::string ApplicationContext::getBrowserCommand() {
    return m_browserCommand;
}

std::string ApplicationContext::getBuildIdentifier() {
#ifdef BUILD_IDENTIFIER
    return MAKE_STR(BUILD_IDENTIFIER);
#else
    return "";
#endif  // BUILD_IDENTIFIER
}

std::string ApplicationContext::getConfigFilePath(size_t index) {
    return m_configFilePaths[index];
}

std::vector<std::string> ApplicationContext::getConfigFilePaths() {
    return m_configFilePaths;
}

std::string ApplicationContext::getDirPath(const std::string& path) {
    char buffer[PATH_MAX];
    auto dirPath = std::string(realpath(path.c_str(), buffer));
    auto pos = dirPath.find_last_of('/');
    if (pos != std::string::npos) {
        // dir path does not include trailing slash
        dirPath = dirPath.substr(0, pos);
    }
    return dirPath;
}

logger::LoggerHandler::Level ApplicationContext::getLevel() {
    return m_level;
}

int ApplicationContext::getMaximumAVSVolume() {
    return 100;
};

std::string ApplicationContext::getMediaPlayerCommand() {
    return m_mediaPlayerCommand;
}

json ApplicationContext::getMenu(const std::string& id) {
    if (m_menuRegister.count(id)) {
        return m_menuRegister.at(id);
    }
    return nullptr;
}

std::vector<std::string> ApplicationContext::getMenuFilePaths() {
    return m_menuFilePaths;
}

json ApplicationContext::getMenuItemValue(const std::string& id, const json defaultValue) {
    if (m_menuRegister.count(id)) {
        auto menu = m_menuRegister.at(id);
        if (menu.count("index") && menu.count("item")) {
            auto index = menu.at("index").get<ssize_t>();
            if ((index >= 0) && (index < (ssize_t)menu.at("item").size())) {
                auto item = menu.at("item")[index];
                if (item.count("value")) {
                    return item.at("value");
                }
            }
        }
    }
    return getMenuValue(id, defaultValue);
}

json* ApplicationContext::getMenuPtr(const std::string& id) {
    if (m_menuRegister.count(id)) {
        return &m_menuRegister.at(id);
    }
    return nullptr;
}

json ApplicationContext::getMenuValue(const std::string& id, const json defaultValue) {
    if (m_menuRegister.count(id)) {
        auto menu = m_menuRegister.at(id);
        if (menu.count("value")) {
            return menu.at("value");
        }
    }
    return defaultValue;
}

int ApplicationContext::getMinimumAVSVolume() {
    return 0;
};

std::string ApplicationContext::getPayloadScriptCommand() {
    return m_payloadScriptCommand;
}

std::string ApplicationContext::getUserConfigFilePath() {
    return m_userConfigFilePath;
}

bool ApplicationContext::hasMenu(const std::string& id) {
    return m_menuRegister.count(id) == 1;
}

bool ApplicationContext::hasRefreshToken(const std::string& service) {
    if (!m_authorizationData[service]["refreshToken"].empty()) {
        return true;
    }
    auto input = std::ifstream(m_applicationDirPath + "/token-" + service, std::ifstream::in);
    if (input.good()) {
        input >> m_authorizationData[service]["refreshToken"];
        input.close();
    }
    return !m_authorizationData[service]["refreshToken"].empty();
}

bool ApplicationContext::hasUserConfigFilePath() {
    return !m_userConfigFilePath.empty();
}

bool ApplicationContext::isAlexaCommsSupported() {
#ifdef ALEXACOMMS
    return true;
#else
    return false;
#endif  // ALEXACOMMS
}

bool ApplicationContext::isAudioFileSupported() {
    return m_audioFileSupported;
}

bool ApplicationContext::isConnectivitySupported() {
#ifdef CONNECTIVITY
    return true;
#else
    return false;
#endif  // CONNECTIVITY
}

bool ApplicationContext::isDcmSupported() {
#ifdef DCM
    return true;
#else
    return false;
#endif  // DCM
}

bool ApplicationContext::isLocalVoiceControlSupported() {
#ifdef LOCALVOICECONTROL
    return true;
#else
    return false;
#endif  // LOCALVOICECONTROL
}

bool ApplicationContext::isLogEnabled() {
    return m_logEnabled;
}

bool ApplicationContext::isSingleThreadedUI() {
    return m_singleThreadedUI;
}

bool ApplicationContext::isTestAutomation() {
    return m_testAutomation;
}

bool ApplicationContext::isWakeWordSupported() {
#ifdef AMAZONLITE
    return true;
#else
    return false;
#endif  // AMAZONLITE
}

bool ApplicationContext::isMessagingResponsesEnabled() {
    return m_messagingResponsesEnabled;
}

bool ApplicationContext::isAuthProviderAuthorizationActive() {
    if (m_activeAuthorization == "alexa:auth-provider") {
        return true;
    }
    return false;
}

bool ApplicationContext::isAuthProviderAuthorizationInProgress() {
    if (m_authorizationInProgress == "alexa:auth-provider") {
        return true;
    }
    return false;
}

bool ApplicationContext::isAuthProviderSupported() {
    return m_authProviderAvailable;
}

bool ApplicationContext::isCBLAuthorizationActive() {
    if (m_activeAuthorization == "alexa:cbl") {
        return true;
    }
    return false;
}

bool ApplicationContext::isCBLAuthorizationInProgress() {
    if (m_authorizationInProgress == "alexa:cbl") {
        return true;
    }
    return false;
}

std::string ApplicationContext::makeTempPath(const std::string& name, const std::string& extension) {
    static std::map<std::string, unsigned> Count{};
    if (Count.count(name) == 0) {
        Count[name] = 0;
    }
    Count.at(name) += 1;
    auto count = std::to_string(Count.at(name));
    return m_applicationDirPath + '/' + name + '-' + count + '.' + extension;
}

std::string ApplicationContext::popAudioFilePath() {
    if (m_audioFilePaths.empty()) {
        return {};
    }
    auto path = m_audioFilePaths.front();
    m_audioFilePaths.pop_front();
    return path;
}

std::size_t ApplicationContext::registerMenu(const std::string& id, const json& menu) {
    std::size_t result = m_menuRegister.count(id) + 1;
    m_menuRegister[id] = menu;
    return result;
}

bool ApplicationContext::saveContent(const std::string& path, const std::string& content) {
    auto output = std::make_shared<std::ofstream>(path, std::ofstream::out | std::ofstream::trunc);
    if (!output->good()) {
        return false;
    }
    output->write(content.c_str(), content.length());
    output->flush();
    output->close();
    return true;
}

void ApplicationContext::setActiveAuthorization(const std::string& service) {
    m_activeAuthorization = service;
    m_authorizationInProgress = "";
}

void ApplicationContext::setAudioFileSupported(bool audioFileSupported) {
    m_audioFileSupported = audioFileSupported;
}

void ApplicationContext::setAudioInputDevice(const std::string& audioInputDevice) {
    m_audioInputDevice = audioInputDevice;
}

void ApplicationContext::setAuthorizationInProgress(const std::string& service) {
    m_authorizationInProgress = service;
}

void ApplicationContext::setAuthProviderAvailability(bool available) {
    m_authProviderAvailable = available;
}

void ApplicationContext::setBrowserCommand(const std::string& browserCommand) {
    m_browserCommand = browserCommand;
}

void ApplicationContext::setLevel(const logger::LoggerHandler::Level level) {
    m_logEnabled = true;
    m_level = level;
}

void ApplicationContext::setMediaPlayerCommand(const std::string& mediaPlayerCommand) {
    m_mediaPlayerCommand = mediaPlayerCommand;
}

void ApplicationContext::setPayloadScriptCommand(const std::string& payloadScriptCommand) {
    m_payloadScriptCommand = payloadScriptCommand;
}

void ApplicationContext::setSingleThreadedUI(bool singleThreadedUI) {
    m_singleThreadedUI = singleThreadedUI;
}

void ApplicationContext::setUserConfigFilePath(const std::string& userConfigFilePath) {
    m_userConfigFilePath = userConfigFilePath;
}

void ApplicationContext::setMessagingResponses(bool messagingResponses) {
    m_messagingResponsesEnabled = messagingResponses;
}

// private

std::string ApplicationContext::getAuthorizationData(const std::string& service, const std::string& key) {
    return m_authorizationData[service][key];
}

void ApplicationContext::setAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    // IMPORTANT: YOUR PRODUCT IS RESPONSIBLE FOR STORING THE AUTHORIZATION DATA SECURELY.
    // VISIT THIS PAGE (https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html)
    // FOR MORE INFORMATION SECURITY REQUIREMENTS.

    // JUST TO FACILITATE TESTING AND VERIFICATION, THIS SAMPLE APP SHALL STORE TOKEN ON THE FILE SYSTEM.
    m_authorizationData[service][key] = data;
    if (key == "refreshToken") {
        auto path = m_applicationDirPath + "/token-" + service;
        if (data.empty()) {
            std::remove(path.c_str());
        } else {
            saveContent(path, data);
        }
    }
}

bool ApplicationContext::testExpression(const std::string& value) {
    // NOTE: EXPRESSION ONLY SUPPORTS LOGICAL AND (&&) HERE
    // e.g. "!RefreshToken && AudioFileSupported"
    bool result = true;
    auto s = value;
    static std::regex r("\\s*(&&)\\s*", std::regex::optimize);
    std::smatch sm{};
    while (std::regex_search(s, sm, r)) {
        result = result && testValue(sm.prefix());
        s = sm.suffix();
    }
    result = result && testValue(s);
    return result;
}

bool ApplicationContext::testValue(const std::string& value) {
    std::string fn;
    bool negate = false;
    bool result = false;

    if (value.find('!') == 0) {
        fn = value.substr(1);
        negate = true;
    } else {
        fn = value;
    }
    // clang-format off
    static std::map<std::string, std::function<bool()>> DispatchTable{
        // has
        {"AuthProviderAuthorizationInProgress", std::bind(&ApplicationContext::isAuthProviderAuthorizationInProgress, this)},
        {"CBLAuthorizationInProgress", std::bind(&ApplicationContext::isCBLAuthorizationInProgress, this)},
        {"UserConfigFilePath", std::bind(&ApplicationContext::hasUserConfigFilePath, this)},
        // is
        {"AlexaCommsSupported", std::bind(&ApplicationContext::isAlexaCommsSupported, this)},
        {"AudioFileSupported", std::bind(&ApplicationContext::isAudioFileSupported, this)},
        {"AuthProviderAuthorizationActive", std::bind(&ApplicationContext::isAuthProviderAuthorizationActive, this)},
        {"AuthProviderSupported", std::bind(&ApplicationContext::isAuthProviderSupported, this)},
        {"CBLAuthorizationActive", std::bind(&ApplicationContext::isCBLAuthorizationActive, this)},
        {"ConnectivitySupported", std::bind(&ApplicationContext::isConnectivitySupported, this)},
        {"LocalVoiceControlSupported", std::bind(&ApplicationContext::isLocalVoiceControlSupported, this)},
        {"LogEnabled", std::bind(&ApplicationContext::isLogEnabled, this)},
        {"SingleThreadedUI", std::bind(&ApplicationContext::isSingleThreadedUI, this)},
        {"TestAutomation", std::bind(&ApplicationContext::isTestAutomation, this)},
        {"WakeWordSupported", std::bind(&ApplicationContext::isWakeWordSupported, this)},
        {"MessagingResponsesEnabled", std::bind(&ApplicationContext::isMessagingResponsesEnabled, this)}
    };
    // clang-format on
    if (DispatchTable.count(fn) != 0) {
        result = DispatchTable[fn]();
    }
    return negate ? !result : result;
}

}  // namespace sampleApp
