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

#ifndef SAMPLEAPP_APPLICATIONCONTEXT_H
#define SAMPLEAPP_APPLICATIONCONTEXT_H

#include "SampleApp/Authorization/AuthorizationHandler.h"
#include "SampleApp/Logger/LoggerHandler.h"

// C++ Standard Library
#include <deque>
#include <map>
#include <string>
#include <vector>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "Event.h"

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ApplicationContext
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ApplicationContext {
private:
    bool m_audioFileSupported{false};
    bool m_authProviderAvailable{false};
    bool m_logEnabled{false};
    bool m_messagingResponsesEnabled{true};
    bool m_singleThreadedUI{false};
    bool m_testAutomation{false};
    json m_menuRegister{};
    logger::LoggerHandler::Level m_level{};
    std::string m_activeAuthorization{};
    std::string m_applicationDirPath{};
    std::string m_applicationPath{};
    std::string m_audioInputDevice{};
    std::string m_authorizationInProgress{};
    std::string m_browserCommand{};
    std::string m_mediaPlayerCommand{};
    std::string m_payloadScriptCommand{};
    std::string m_userConfigFilePath{};
    std::deque<std::string> m_audioFilePaths{};
    std::vector<std::string> m_configFilePaths{};
    std::vector<std::string> m_menuFilePaths{};

protected:
    ApplicationContext(const std::string& path);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<ApplicationContext> {
        return std::shared_ptr<ApplicationContext>(new ApplicationContext(args...));
    }
    auto addAudioFilePath(const std::string& audioFilePath) -> void;
    auto addConfigFilePath(const std::string& configFilePath) -> void;
    auto addMenuFilePath(const std::string& menuFilePath) -> void;
    auto checkDcmConfiguration(const std::vector<json>& configs) -> bool;
    auto clearLevel() -> void;
    auto clearUserConfigFilePath() -> void;
    auto executeCommand(const char* command) -> std::string;
    auto getApplicationDirPath() -> std::string;
    auto getApplicationPath() -> std::string;
    auto getAudioInputDevice() -> std::string;
    auto getBrowserCommand() -> std::string;
    auto getBuildIdentifier() -> std::string;
    auto getConfigFilePath(size_t index = 0) -> std::string;
    auto getConfigFilePaths() -> std::vector<std::string>;
    auto getDirPath(const std::string& path) -> std::string;
    auto getLevel() -> logger::LoggerHandler::Level;
    auto getMaximumAVSVolume() -> int;
    auto getMediaPlayerCommand() -> std::string;
    auto getMenu(const std::string& id) -> json;
    auto getMenuFilePaths() -> std::vector<std::string>;
    auto getMenuItemValue(const std::string& id, json defaultValue = nullptr) -> json;
    auto getMenuPtr(const std::string& id) -> json*;
    auto getMenuValue(const std::string& id, json defaultValue = nullptr) -> json;
    auto getMinimumAVSVolume() -> int;
    auto getPayloadScriptCommand() -> std::string;
    auto getUserConfigFilePath() -> std::string;
    auto hasDefaultMediaPlayer() -> bool;
    auto hasMenu(const std::string& id) -> bool;
    auto hasRefreshToken(const std::string& service) -> bool;
    auto hasUserConfigFilePath() -> bool;
    auto isAlexaCommsSupported() -> bool;
    auto isAudioFileSupported() -> bool;
    auto isConnectivitySupported() -> bool;
    auto isDcmSupported() -> bool;
    auto isLocalVoiceControlSupported() -> bool;
    auto isLogEnabled() -> bool;
    auto isSingleThreadedUI() -> bool;
    auto isTestAutomation() -> bool;
    auto isAuthProviderAuthorizationActive() -> bool;
    auto isAuthProviderAuthorizationInProgress() -> bool;
    auto isAuthProviderSupported() -> bool;
    auto isCBLAuthorizationActive() -> bool;
    auto isCBLAuthorizationInProgress() -> bool;
    auto isMessagingResponsesEnabled() -> bool;
    auto isWakeWordSupported() -> bool;
    auto makeTempPath(const std::string& name, const std::string& extension) -> std::string;
    auto popAudioFilePath() -> std::string;
    auto registerMenu(const std::string& id, const json& menu) -> std::size_t;
    auto saveContent(const std::string& path, const std::string& content) -> bool;
    auto setActiveAuthorization(const std::string& service) -> void;
    auto setAudioFileSupported(bool audioFileSupported) -> void;
    auto setAudioInputDevice(const std::string& audioInputDevice) -> void;
    auto setAuthorizationInProgress(const std::string& service) -> void;
    auto setAuthProviderAvailability(bool available) -> void;
    auto setBrowserCommand(const std::string& browserCommand) -> void;
    auto setLevel(logger::LoggerHandler::Level level) -> void;
    auto setMediaPlayerCommand(const std::string& mediaPlayerCommand) -> void;
    auto setMessagingResponses(bool messagingResponses) -> void;
    auto setPayloadScriptCommand(const std::string& payloadScriptCommand) -> void;
    auto setSingleThreadedUI(bool singleThreadedUI) -> void;
    auto setUserConfigFilePath(const std::string& userConfigFilePath) -> void;
    auto testExpression(const std::string& value) -> bool;
    auto testValue(const std::string& value) -> bool;

private:
    friend authorization::AuthorizationHandler;
    friend authorization::AuthProviderAuthorizationHandler;
    /// Map of service to map of key and value
    std::map<std::string, std::map<std::string, std::string>> m_authorizationData;

    auto getAuthorizationData(const std::string& service, const std::string& key) -> std::string;
    auto setAuthorizationData(const std::string& service, const std::string& key, const std::string& data) -> void;
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_APPLICATIONCONTEXT_H
