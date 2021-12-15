/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <gsl/gsl-lite.hpp>

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
    realpath(path.c_str(), buffer);
    m_applicationPath = std::string(buffer);
    auto pos = m_applicationPath.find_last_of('/');
    if (pos != std::string::npos) {
        // dir path does not include trailing slash
        m_applicationDirPath = m_applicationPath.substr(0, pos);
    }

    auto input = std::ifstream(m_applicationDirPath + "/activeAuthorization", std::ifstream::in);
    if (input.good()) {
        input >> m_activeAuthorization;
        input.close();
    }

    m_menuRegister = json::object();
}

void ApplicationContext::addAudioFilePath(const std::string& audioFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_audioFilePaths.push_back(audioFilePath);
    m_testAutomation = true;
}

void ApplicationContext::addConfigFilePath(const std::string& configFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_configFilePaths.push_back(configFilePath);
}

void ApplicationContext::addMenuFilePath(const std::string& menuFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_menuFilePaths.push_back(menuFilePath);
}

void ApplicationContext::clearLevel() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logEnabled = false;
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_applicationDirPath;
}

std::string ApplicationContext::getApplicationPath() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_applicationPath;
}

std::string ApplicationContext::getAudioInputDevice() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_audioInputDevice;
}

std::string ApplicationContext::getBrowserCommand() {
    std::lock_guard<std::mutex> lock(m_mutex);
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_configFilePaths[index];
}

std::vector<std::string> ApplicationContext::getConfigFilePaths() {
    std::lock_guard<std::mutex> lock(m_mutex);
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_level;
}

int ApplicationContext::getMaximumAVSVolume() {
    return 100;
};

std::string ApplicationContext::getMediaPlayerCommand() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_mediaPlayerCommand;
}

json ApplicationContext::getMenu(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_menuRegister.count(id)) {
        return m_menuRegister.at(id);
    }
    return nullptr;
}

std::vector<std::string> ApplicationContext::getMenuFilePaths() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_menuFilePaths;
}

json ApplicationContext::getMenuItemValue(const std::string& id, const json defaultValue) {
    std::lock_guard<std::mutex> lock(m_mutex);
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
    return getMenuValueLocked(id, defaultValue);
}

json* ApplicationContext::getMenuPtr(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_menuRegister.count(id)) {
        return &m_menuRegister.at(id);
    }
    return nullptr;
}

int ApplicationContext::getMinimumAVSVolume() {
    return 0;
};

std::string ApplicationContext::getNetworkIdentifier() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_networkIdentifier;
}

std::string ApplicationContext::getPayloadScriptCommand() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_payloadScriptCommand;
}

bool ApplicationContext::hasMenu(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_menuRegister.count(id) == 1;
}

bool ApplicationContext::hasRefreshToken(const std::string& service) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_authorizationData[service]["refreshToken"].empty()) {
        return true;
    }
    std::ifstream input(m_applicationDirPath + "/token-" + service, std::ifstream::in);
    if (input.good()) {
        input >> m_authorizationData[service]["refreshToken"];
        input.close();
    }
    return !m_authorizationData[service]["refreshToken"].empty();
}

bool ApplicationContext::isAlexaCommsSupported() {
#ifdef AAC_ALEXACOMMS
    return true;
#else
    return false;
#endif  // ALEXACOMMS
}

bool ApplicationContext::isAudioFileSupported() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_audioFileSupported;
}

bool ApplicationContext::isAutoAuthorizationDisabled() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_disableAutoAuthorization;
}

bool ApplicationContext::isConnectivitySupported() {
#ifdef AAC_CONNECTIVITY
    return true;
#else
    return false;
#endif  // CONNECTIVITY
}

bool ApplicationContext::isDcmSupported() {
#ifdef AAC_DCM
    return true;
#else
    return false;
#endif  // DCM
}

bool ApplicationContext::isLocalVoiceControlSupported() {
#ifdef AAC_LOCAL_VOICE_CONTROL
    return true;
#else
    return false;
#endif  // LOCALVOICECONTROL
}

bool ApplicationContext::isLogEnabled() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logEnabled;
}

bool ApplicationContext::isSingleThreadedUI() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_singleThreadedUI;
}

bool ApplicationContext::isTestAutomation() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_testAutomation;
}

bool ApplicationContext::isWakeWordSupported() {
#ifdef AAC_AMAZONLITE
    return true;
#else
    return false;
#endif  // AMAZONLITE
}

bool ApplicationContext::isMessagingResponsesEnabled() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_messagingResponsesEnabled;
}

bool ApplicationContext::isAuthProviderAuthorizationActive() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_activeAuthorization == "alexa:auth-provider") {
        return true;
    }
    return false;
}

bool ApplicationContext::isAuthProviderAuthorizationInProgress() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_authorizationInProgress == "alexa:auth-provider") {
        return true;
    }
    return false;
}

bool ApplicationContext::isAuthProviderSupported() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_authProviderAvailable;
}

bool ApplicationContext::isCBLAuthorizationActive() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_activeAuthorization == "alexa:cbl") {
        return true;
    }
    return false;
}

bool ApplicationContext::isCBLAuthorizationInProgress() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_authorizationInProgress == "alexa:cbl") {
        return true;
    }
    return false;
}

std::string ApplicationContext::makeTempPath(const std::string& name, const std::string& extension) {
    std::lock_guard<std::mutex> lock(m_mutex);
    static std::map<std::string, unsigned> Count{};
    if (Count.count(name) == 0) {
        Count[name] = 0;
    }
    Count.at(name) += 1;
    auto count = std::to_string(Count.at(name));
    return m_applicationDirPath + '/' + name + '-' + count + '.' + extension;
}

std::string ApplicationContext::popAudioFilePath() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_audioFilePaths.empty()) {
        return {};
    }
    auto path = m_audioFilePaths.front();
    m_audioFilePaths.pop_front();
    return path;
}

std::size_t ApplicationContext::registerMenu(const std::string& id, const json& menu) {
    std::lock_guard<std::mutex> lock(m_mutex);
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
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activeAuthorization = service;
    m_authorizationInProgress = "";

    // IMPORTANT: YOUR PRODUCT IS RESPONSIBLE FOR STORING ANY RELATED AUTHORIZATION DATA SECURELY.
    // VISIT THIS PAGE (https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html)
    // FOR MORE INFORMATION ON SECURITY REQUIREMENTS.

    // JUST TO FACILITATE EASY TESTING AND VERIFICATION, THIS SAMPLE APP SHALL STORE AND RETRIVE ACTIVE
    // AUTHORIZATION FROM THE FILE SYSTEM.
    auto path = m_applicationDirPath + "/activeAuthorization";
    if (service.empty()) {
        std::remove(path.c_str());
    } else {
        saveContent(path, service);
    }
}

void ApplicationContext::setAudioFileSupported(bool audioFileSupported) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_audioFileSupported = audioFileSupported;
}

void ApplicationContext::setAudioInputDevice(const std::string& audioInputDevice) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_audioInputDevice = audioInputDevice;
}

void ApplicationContext::setAuthorizationInProgress(const std::string& service) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_authorizationInProgress = service;
}

void ApplicationContext::setAuthProviderAvailability(bool available) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_authProviderAvailable = available;
}

void ApplicationContext::setBrowserCommand(const std::string& browserCommand) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_browserCommand = browserCommand;
}

void ApplicationContext::setDisableAutoAuthorizationCommand(bool disable) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_disableAutoAuthorization = disable;
    if (m_disableAutoAuthorization == true) {
        // Reset the active authorization state that is read from the file system
        m_activeAuthorization = "";
    }
}

void ApplicationContext::setLevel(const logger::LoggerHandler::Level level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logEnabled = true;
    m_level = level;
}

void ApplicationContext::setMediaPlayerCommand(const std::string& mediaPlayerCommand) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_mediaPlayerCommand = mediaPlayerCommand;
}

void ApplicationContext::setNetworkIdentifier(const std::string& networkIdentifier) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_networkIdentifier = networkIdentifier;
}

void ApplicationContext::setPayloadScriptCommand(const std::string& payloadScriptCommand) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_payloadScriptCommand = payloadScriptCommand;
}

void ApplicationContext::setSingleThreadedUI(bool singleThreadedUI) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_singleThreadedUI = singleThreadedUI;
}

void ApplicationContext::setMessagingResponses(bool messagingResponses) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messagingResponsesEnabled = messagingResponses;
}

// private

std::string ApplicationContext::getAuthorizationData(const std::string& service, const std::string& key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    try {
        return m_authorizationData.at(service).at(key);
    } catch (std::out_of_range& ex) {
        return "";
    }
}

void ApplicationContext::setAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    // IMPORTANT: YOUR PRODUCT IS RESPONSIBLE FOR STORING ANY AUTHORIZATION DATA SECURELY.
    // VISIT THIS PAGE (https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/avs-security-reqs.html)
    // FOR MORE INFORMATION ON SECURITY REQUIREMENTS.

    // JUST TO FACILITATE TESTING AND VERIFICATION, THIS SAMPLE APP SHALL STORE TOKEN ON THE FILE SYSTEM.
    std::lock_guard<std::mutex> lock(m_mutex);
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

json ApplicationContext::getMenuValueLocked(const std::string& id, const json defaultValue) {
    if (m_menuRegister.count(id)) {
        auto menu = m_menuRegister.at(id);
        if (menu.count("value")) {
            return menu.at("value");
        }
    }
    return defaultValue;
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
