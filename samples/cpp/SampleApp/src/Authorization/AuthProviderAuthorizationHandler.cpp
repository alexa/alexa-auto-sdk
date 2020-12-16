/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Authorization/AuthProviderAuthorizationHandler.h"

// C++ Standard Library
#include <sstream>
#include <fstream>
#include <condition_variable>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace authorization {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AuthProviderAuthorizationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using json = nlohmann::json;

/// Represents Application-Provided authorization
static const std::string APPLICATION_PROVIDED = "alexa:auth-provider";

static const std::chrono::seconds HEAD_START_TIME(600);

// clang-format off
static const json UNAUTHORIZED_EVENT_JSON = {
    {"type", "authStateChangeEvent"},
    {"payload", {
        {"state", "UNAUTHORIZED"}
    }}
};
static const json AUTHORIZED_EVENT_JSON = {
    {"type", "authStateChangeEvent"},
    {"payload", {
        {"state", "AUTHORIZED"}
    }}
};
// clang-format on

/**
 * Function to convert the number of times we have already retried to the time to perform the next retry.
 *
 * @param retryCount The number of times we have retried
 * @return The time that the next retry should be attempted
 */
static std::chrono::steady_clock::time_point calculateTimeToRetry(int retryCount) {
    const static std::vector<int> retryBackoffTimes = {1, 3, 5, 10, 15};
    if (retryCount < 0) {
        retryCount = 0;
    } else if ((size_t)retryCount >= retryBackoffTimes.size()) {
        retryCount = retryBackoffTimes.size() - 1;
    }
    auto randSecs = rand() % retryBackoffTimes[retryCount];

    return std::chrono::steady_clock::now() + std::chrono::seconds(randSecs);
}

AuthProviderAuthorizationHandler::AuthProviderAuthorizationHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<AuthProviderAuthorizationListenerInterface> listener) :
        m_activity{activity}, m_loggerHandler{loggerHandler}, m_listener{std::move(listener)} {
    auto m_activity_lock = m_activity.lock();
    if (!m_activity_lock) {
        return;
    }
    m_applicationContext = m_activity_lock->getApplicationContext();
    m_consumerTokenScript = m_applicationContext->getApplicationDirPath() + "/scripts/consumer_token.sh";
    m_deviceRegistrationScript = m_applicationContext->getApplicationDirPath() + "/scripts/device_registration.sh";
    m_refreshTokenScript = m_applicationContext->getApplicationDirPath() + "/scripts/refresh_token.sh";
    m_deviceDeregistrationScript = m_applicationContext->getApplicationDirPath() + "/scripts/device_deregistration.sh";
}

bool AuthProviderAuthorizationHandler::isAuthProviderSupported() {
    std::ifstream consumerTokenScriptscriptFile(m_consumerTokenScript);
    std::ifstream deviceRegistrationScriptFile(m_deviceRegistrationScript);
    std::ifstream refreshTokenScriptFile(m_refreshTokenScript);
    std::ifstream deviceDeregistrationScriptFile(m_deviceDeregistrationScript);

    if (consumerTokenScriptscriptFile.good() && deviceRegistrationScriptFile.good() && refreshTokenScriptFile.good() &&
        deviceDeregistrationScriptFile.good()) {
        return true;
    }
    return false;
}

void AuthProviderAuthorizationHandler::startAuthorization() {
    m_executer.submit([=]() {
        // If the refresh token exists use that instead of starting the new authorization flow.
        if (m_applicationContext->hasRefreshToken(APPLICATION_PROVIDED)) {
            // m_tokenExpirationTime set to seconds(0) so that refresh token gets refreshed immediately
            // in the handleRefreshingToken thread.
            m_tokenExpirationTime = std::chrono::seconds(0);
            startRefreshThread();
            return;
        }

        // Retrieve consumer token
        auto commandResult = m_applicationContext->executeCommand(m_consumerTokenScript.c_str());
        auto consumerToken = json::parse(commandResult);

        // Check that the token was created successfully
        if (!consumerToken.contains("access_token")) {
            std::stringstream ss;
            ss << "startAuthorization error"
               << "Failed to get LWA consumer token" << commandResult << std::endl;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
            showMessage("Failed to get LWA consumer token: " + commandResult, true);
            if (auto m_listener_lock = m_listener.lock()) {
                m_listener_lock->sendHandlerEvent(APPLICATION_PROVIDED, UNAUTHORIZED_EVENT_JSON.dump());
            }
            return;
        }

        // Create an AVS token by registering device
        std::stringstream deviceRegistrationCommand;
        deviceRegistrationCommand << m_deviceRegistrationScript << " --token " << consumerToken["access_token"]
                                  << " --product-id " << m_productId << " --device-serial-number "
                                  << m_deviceSerialNumber;
        commandResult = m_applicationContext->executeCommand(deviceRegistrationCommand.str().c_str());
        auto deviceToken = json::parse(commandResult);

        if (deviceToken.contains("accessToken") && deviceToken.contains("refreshToken")) {
            m_refreshToken = deviceToken["refreshToken"];
            m_accessToken = deviceToken["accessToken"];
            m_tokenExpirationTime = std::chrono::seconds(deviceToken["expiresIn"]) - HEAD_START_TIME;
            m_applicationContext->setAuthorizationData(APPLICATION_PROVIDED, "refreshToken", m_refreshToken);
            if (auto m_listener_lock = m_listener.lock()) {
                m_listener_lock->sendHandlerEvent(APPLICATION_PROVIDED, AUTHORIZED_EVENT_JSON.dump());
            }
            startRefreshThread();
            return;
        }

        std::stringstream ss;
        ss << "startAuthorization error,"
           << "Failed to get LWA token:" << commandResult << std::endl;
        log(logger::LoggerHandler::Level::ERROR, ss.str());

        showMessage("Failed to get LWA token: " + commandResult, true);
        if (auto m_listener_lock = m_listener.lock()) {
            m_listener_lock->sendHandlerEvent(APPLICATION_PROVIDED, UNAUTHORIZED_EVENT_JSON.dump());
        }
    });
}

void AuthProviderAuthorizationHandler::logout() {
    auto future = m_executer.submit([=]() {
        stopRefreshThread();
        // Retrieve consumer token
        auto commandResult = m_applicationContext->executeCommand(m_consumerTokenScript.c_str());
        auto consumerToken = json::parse(commandResult);

        // Check that the token was created successfully
        if (!consumerToken.contains("access_token")) {
            std::stringstream ss;
            ss << "logout error"
               << "Failed to get LWA consumer token" << commandResult << std::endl;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
            showMessage("Failed to get LWA Consumer token: " + commandResult, true);
            return;
        }

        std::stringstream deviceDeregistrationCommand;
        deviceDeregistrationCommand << m_deviceDeregistrationScript << " --token " << consumerToken["access_token"]
                                    << " --product-id " << m_productId << " --device-serial-number "
                                    << m_deviceSerialNumber;

        commandResult = m_applicationContext->executeCommand(deviceDeregistrationCommand.str().c_str());
        if (commandResult.find("204") != std::string::npos) {
            m_refreshToken = "";
            m_accessToken = "";
            m_applicationContext->setAuthorizationData(APPLICATION_PROVIDED, "refreshToken", "");
            std::stringstream ss;
            ss << "Successfully logged out" << std::endl;
            log(logger::LoggerHandler::Level::INFO, ss.str());
            showMessage("Successfully logged out");
        } else {
            std::stringstream ss;
            ss << "Failed to delete device registration" << std::endl;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
            showMessage("Failed to delete device registration: " + commandResult, true);
        }
    });
    if (future.valid()) {
        future.get();  // Wait for complete
    }
}

std::string AuthProviderAuthorizationHandler::getAccessToken() {
    std::unique_lock<std::mutex> lock(m_refreshTokenMutex);
    json accessToken = {{"accessToken", m_accessToken}};
    return accessToken.dump();
}

// private
void AuthProviderAuthorizationHandler::stopRefreshThread() {
    if (m_threadActive == true) {
        m_isStopping = true;
        m_wake.notify_one();
        if (m_refreshTokenThread.joinable()) {
            m_refreshTokenThread.join();
        }
        m_threadActive = false;
    }
}

void AuthProviderAuthorizationHandler::startRefreshThread() {
    stopRefreshThread();
    m_isStopping = false;
    m_threadActive = true;
    m_refreshTokenThread = std::thread(&AuthProviderAuthorizationHandler::handleRefreshingToken, this);
}

void AuthProviderAuthorizationHandler::handleRefreshingToken() {
    int retryCount = 0;
    auto nextActionTime = std::chrono::steady_clock::now() + m_tokenExpirationTime;
    bool validToken = true;
    // If m_tokenExpirationTime == 0 then we are required to send the auth state change event
    // for the first time
    bool previousValidToken = m_tokenExpirationTime == std::chrono::seconds(0) ? false : true;

    while (!isStopping()) {
        log(logger::LoggerHandler::Level::INFO, "handleRefreshingToken");

        std::unique_lock<std::mutex> lock(m_mutex);
        m_wake.wait_until(lock, nextActionTime, [this] { return m_isStopping; });

        if (m_isStopping) {
            break;
        }

        std::stringstream refreshTokenCommand;
        refreshTokenCommand << m_refreshTokenScript << " --refresh-token "
                            << "\"" << m_applicationContext->getAuthorizationData(APPLICATION_PROVIDED, "refreshToken")
                            << "\"";

        std::unique_lock<std::mutex> refreshLock(m_refreshTokenMutex);
        auto commandResult = m_applicationContext->executeCommand(refreshTokenCommand.str().c_str());
        auto token = json::parse(commandResult);

        // Check that the token was created successfully
        if (token.contains("access_token")) {
            m_refreshToken = token["refresh_token"];
            m_accessToken = token["access_token"];
            std::chrono::seconds expirationTime = std::chrono::seconds(token["expires_in"]) - HEAD_START_TIME;
            nextActionTime = std::chrono::steady_clock::now() + expirationTime;
            m_applicationContext->setAuthorizationData(APPLICATION_PROVIDED, "refreshToken", m_refreshToken);
            validToken = true;
            retryCount = 0;
        } else {
            m_refreshToken = "";
            m_accessToken = "";
            validToken = false;
            nextActionTime = calculateTimeToRetry(retryCount++);
        }
        refreshLock.unlock();

        if (previousValidToken != validToken) {
            previousValidToken = validToken;
            if (auto m_listener_lock = m_listener.lock()) {
                if (!validToken) {
                    showMessage("Failed to get LWA access token: " + commandResult, true);
                    m_listener_lock->sendHandlerEvent(APPLICATION_PROVIDED, UNAUTHORIZED_EVENT_JSON.dump());
                } else {
                    showMessage("Successful in getting LWA access token: " + commandResult, true);
                    m_listener_lock->sendHandlerEvent(APPLICATION_PROVIDED, AUTHORIZED_EVENT_JSON.dump());
                }
            }
        }
    }
    log(logger::LoggerHandler::Level::INFO, "Stopping handleRefreshingToken");
}

AuthProviderAuthorizationHandler::~AuthProviderAuthorizationHandler() {
    m_isStopping = true;
    m_wake.notify_all();

    if (m_refreshTokenThread.joinable()) {
        m_refreshTokenThread.join();
    }
    m_threadActive = false;
}

bool AuthProviderAuthorizationHandler::isStopping() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isStopping;
}

void AuthProviderAuthorizationHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AuthProviderAuthorizationHandler", message);
}

void AuthProviderAuthorizationHandler::showMessage(const std::string& message, bool asError) {
    // Log as error or info
    if (asError) {
        log(logger::LoggerHandler::Level::ERROR, message);
    } else {
        log(logger::LoggerHandler::Level::INFO, message);
    }

    if (auto console = m_console.lock()) {
        console->printRuler();
        console->printLine(message);
        console->printRuler();
    }
}

void AuthProviderAuthorizationHandler::saveDeviceInfo(const std::vector<json>& jsons) {
    // Look for device info
    for (auto const& config : jsons) {
        try {
            if (config.contains("aace.alexa") && config["aace.alexa"].contains("avsDeviceSDK") &&
                config["aace.alexa"]["avsDeviceSDK"].contains("deviceInfo")) {
                auto deviceInfo = config["aace.alexa"]["avsDeviceSDK"]["deviceInfo"];
                m_clientId = deviceInfo["clientId"];
                m_deviceSerialNumber = deviceInfo["deviceSerialNumber"];
                m_productId = deviceInfo["productId"];
            }
        } catch (json::exception& e) {
        }
    }
}

}  // namespace authorization
}  // namespace sampleApp
