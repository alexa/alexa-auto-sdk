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

#include <nlohmann/json.hpp>

#include "SampleApp/Authorization/AuthorizationHandler.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <sstream>
#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace authorization {

using json = nlohmann::json;

/// Represents Application-Provided authorization
static const std::string APPLICATION_PROVIDED = "alexa:auth-provider";

/// CBL Authorization
static const std::string CBL = "alexa:cbl";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AuthorizationHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AuthorizationHandler::AuthorizationHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
}

void AuthorizationHandler::initialize() {
    m_authProviderHandler = AuthProviderAuthorizationHandler::create(m_activity, m_loggerHandler, shared_from_this());
    setupUI();
}

std::weak_ptr<Activity> AuthorizationHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AuthorizationHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::authorization::Authorization interface
void AuthorizationHandler::eventReceived(const std::string& service, const std::string& event) {
    if (service == APPLICATION_PROVIDED) {
        try {
            auto eventPayload = json::parse(event);
            if (eventPayload.find("type") != eventPayload.end() && eventPayload["type"].is_string()) {
                auto type = eventPayload["type"];
                if (type == "requestAuthorization") {
                    m_authProviderHandler->startAuthorization();
                } else if (type == "logout") {
                    m_authProviderHandler->logout();
                }
            }
        } catch (std::exception& e) {
            std::stringstream ss;
            ss << "eventReceived parser error" << e.what() << std::endl;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
        }
    }
    if (service == CBL) {
        auto activity = m_activity.lock();
        if (!activity) {
            return;
        }
        try {
            auto eventPayload = json::parse(event);
            if (eventPayload.find("type") != eventPayload.end() && eventPayload["type"].is_string()) {
                auto type = eventPayload["type"];
                if (type == "cbl-code") {
                    if (eventPayload.find("payload") != eventPayload.end() && eventPayload["payload"].is_object()) {
                        auto payload = eventPayload["payload"];
                        if ((payload.find("code") != payload.end() && payload["code"].is_string()) &&
                            (payload.find("url") != payload.end() && payload["url"].is_string())) {
                            std::string code = payload["code"];
                            std::string url = payload["url"];
                            activity->runOnUIThread([=]() {
                                if (auto card = activity->findViewById("id:card").lock()) {
                                    // clang-format off
                                    json string = {
                                        {"code",  code}, 
                                        {"url", url}
                                    };
                                    // clang-format on
                                    card->set(string.dump(), View::Type::CBLCode);
                                }
                            });
                            auto command = m_applicationContext->getBrowserCommand();
                            if (!command.empty()) {
                                m_applicationContext->executeCommand(
                                    (command + ' ' + url + "?cbl-code=" + code).c_str());
                            }
                        } else {
                            log(logger::LoggerHandler::Level::ERROR, "invalidCodeOrUrl");
                        }
                    } else {
                        log(logger::LoggerHandler::Level::ERROR, "invalidPayLoad");
                    }
                } else if (type == "user-profile") {
                    if (eventPayload.find("payload") != eventPayload.end() && eventPayload["payload"].is_object()) {
                        auto payload = eventPayload["payload"];
                        if ((payload.find("name") != payload.end() && payload["name"].is_string()) &&
                            (payload.find("email") != payload.end() && payload["email"].is_string())) {
                            std::string name = payload["name"];
                            std::string email = payload["email"];
                            std::stringstream ss;
                            ss << "UserProfile:name=" << name << ",email=" << email;
                            log(logger::LoggerHandler::Level::INFO, ss.str());
                            activity->runOnUIThread([=]() { showMessage("Welcome " + name + ", " + email); });
                        } else {
                            log(logger::LoggerHandler::Level::ERROR, "invalidNameOrEmail");
                        }
                    } else {
                        log(logger::LoggerHandler::Level::ERROR, "invalidPayLoad");
                    }
                }
            } else {
                log(logger::LoggerHandler::Level::ERROR, "invalidType");
            }
        } catch (std::exception& e) {
            std::stringstream ss;
            ss << "eventReceived parser error" << e.what() << std::endl;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
        }
    }
}

void AuthorizationHandler::authorizationStateChanged(
    const std::string& service,
    aace::authorization::Authorization::AuthorizationState state) {
    std::stringstream ss;
    ss << "authorizationStateChanged:state=" << state;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            std::stringstream ss;
            ss << "{\"service\":\"" << service << "\",\"state\":\"" << state << "\"}";
            card->set(ss.str(), View::Type::AuthorizationStateChanged);
        }
    });
    if (state == aace::authorization::Authorization::AuthorizationState::AUTHORIZING) {
        m_applicationContext->setAuthorizationInProgress(service);
    } else if (state == aace::authorization::Authorization::AuthorizationState::UNAUTHORIZED) {
        m_applicationContext->setActiveAuthorization("");
        m_cv.notify_one();
    } else if (state == aace::authorization::Authorization::AuthorizationState::AUTHORIZED) {
        m_applicationContext->setActiveAuthorization(m_selectedAuthorization);
    }
}

void AuthorizationHandler::authorizationError(
    const std::string& service,
    const std::string& error,
    const std::string& message) {
    std::stringstream ss;
    ss << "authorizationError:service=" << service << ",error=" << error;
    if (!message.empty()) {
        ss << ",message=" << message;
    }
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            // clang-format off
            json string = {
                {"service",  service}, 
                {"error", error}
            };
            // clang-format on
            card->set(string.dump(), View::Type::AuthorizationError);
        }
    });
}

std::string AuthorizationHandler::getAuthorizationData(const std::string& service, const std::string& key) {
    if (service == APPLICATION_PROVIDED) {
        if (key == "accessToken") {
            return m_authProviderHandler->getAccessToken();
        } else {
            std::stringstream ss;
            ss << "getAuthorizationData:reason="
               << "invalidKey"
               << ",key=" << key;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
        }
    } else if (service == CBL) {
        if (key == "refreshToken") {
            return m_applicationContext->getAuthorizationData(service, key);
        } else {
            std::stringstream ss;
            ss << "getAuthorizationData:reason="
               << "invalidKey"
               << ",key=" << key;
            log(logger::LoggerHandler::Level::ERROR, ss.str());
        }
    }
    return "";
}

void AuthorizationHandler::sendHandlerEvent(const std::string& service, const std::string& event) {
    sendEvent(service, event);
}

void AuthorizationHandler::setAuthorizationData(
    const std::string& service,
    const std::string& key,
    const std::string& data) {
    m_applicationContext->setAuthorizationData(service, key, data);
}

// private
void AuthorizationHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AuthorizationHandler", message);
}

void AuthorizationHandler::showMessage(const std::string& message, bool asError) {
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

void AuthorizationHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_applicationContext = activity->getApplicationContext();
    m_console = activity->findViewById("id:console");

    m_applicationContext->setAuthProviderAvailability(false);
    m_applicationContext->setAuthProviderAvailability(m_authProviderHandler->isAuthProviderSupported());

    // startAuthorization for CBL
    activity->registerObserver(Event::onStartCBLAuthorization, [=](const std::string&) {
        showMessage("Starting CBL authorization flow...");
        m_selectedAuthorization = CBL;
        std::string refreshToken = "";
        if (m_applicationContext->hasRefreshToken(m_selectedAuthorization)) {
            refreshToken = m_applicationContext->getAuthorizationData(m_selectedAuthorization, "refreshToken");
        }
        startAuthorization(m_selectedAuthorization, refreshToken);
        return true;
    });

    // cancelAuthorization for CBL
    activity->registerObserver(Event::onCancelCBLAuthorization, [=](const std::string&) {
        showMessage("Canceling CBL authorization flow...");
        cancelAuthorization(CBL);
        return true;
    });

    // logoutAuthorization for CBL
    activity->registerObserver(Event::onLogoutCBLAuthorization, [=](const std::string&) {
        showMessage("Logout CBL authorization");
        logout(CBL);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock);
        return true;
    });

    if (m_authProviderHandler->isAuthProviderSupported()) {  // Check if Auth Provider is supported
        // startAuthorization for Auth Provider
        activity->registerObserver(Event::onStartAuthProviderAuthorization, [=](const std::string&) {
            showMessage("Starting Auth Provider authorization flow...");
            m_selectedAuthorization = APPLICATION_PROVIDED;
            startAuthorization(m_selectedAuthorization, "");
            return true;
        });

        // cancelAuthorization for Auth Provider
        activity->registerObserver(Event::onCancelAuthProviderAuthorization, [=](const std::string&) {
            showMessage("Canceling Auth Provider authorization flow...");
            cancelAuthorization(APPLICATION_PROVIDED);
            return true;
        });

        // logoutAuthorization for Auth Provider
        activity->registerObserver(Event::onLogoutAuthProviderAuthorization, [=](const std::string&) {
            showMessage("Logout Auth Provider authorization");
            logout(APPLICATION_PROVIDED);
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock);
            return true;
        });
    }
}

void AuthorizationHandler::saveDeviceInfo(const std::vector<json>& jsons) {
    m_authProviderHandler->saveDeviceInfo(jsons);
}

}  // namespace authorization
}  // namespace sampleApp
