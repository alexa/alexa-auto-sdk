/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Message/Authorization/Authorization/AuthorizationErrorMessage.h>
#include <AASB/Message/Authorization/Authorization/AuthorizationStateChangedMessage.h>
#include <AASB/Message/Authorization/Authorization/CancelAuthorizationMessage.h>
#include <AASB/Message/Authorization/Authorization/EventReceivedMessage.h>
#include <AASB/Message/Authorization/Authorization/GetAuthorizationDataMessage.h>
#include <AASB/Message/Authorization/Authorization/LogoutMessage.h>
#include <AASB/Message/Authorization/Authorization/SendEventMessage.h>
#include <AASB/Message/Authorization/Authorization/SetAuthorizationDataMessage.h>
#include <AASB/Message/Authorization/Authorization/StartAuthorizationMessage.h>

// C++ Standard Library
#include <sstream>
#include <fstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace authorization {

using json = nlohmann::json;
using namespace aasb::message::authorization::authorization;

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
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    subscribeToAASBMessages();
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

void AuthorizationHandler::startAuth() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    if (m_applicationContext->isCBLAuthorizationActive()) {
        activity->notify(Event::onStartCBLAuthorization);
        return;
    }
    if (m_applicationContext->isAuthProviderAuthorizationActive()) {
        activity->notify(Event::onStartAuthProviderAuthorization);
        return;
    }
}

void AuthorizationHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to EventReceived Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleEventReceivedMessage(message); },
        EventReceivedMessage::topic(),
        EventReceivedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AuthorizationStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAuthorizationStateChangedMessage(message); },
        AuthorizationStateChangedMessage::topic(),
        AuthorizationStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to AuthorizationError Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleAuthorizationErrorMessage(message); },
        AuthorizationErrorMessage::topic(),
        AuthorizationErrorMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetAuthorizationData Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetAuthorizationDataMessage(message); },
        GetAuthorizationDataMessage::topic(),
        GetAuthorizationDataMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetAuthorizationData Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetAuthorizationDataMessage(message); },
        SetAuthorizationDataMessage::topic(),
        SetAuthorizationDataMessage::action());
}

void AuthorizationHandler::handleEventReceivedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received EventReceivedMessage");
    EventReceivedMessage msg = json::parse(message);
    eventReceived(msg.payload.service, msg.payload.event);
}

void AuthorizationHandler::handleAuthorizationStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AuthorizationStateChangedMessage");
    AuthorizationStateChangedMessage msg = json::parse(message);
    authorizationStateChanged(msg.payload.service, msg.payload.state);
}

void AuthorizationHandler::handleAuthorizationErrorMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received AuthorizationErrorMessage");
    AuthorizationErrorMessage msg = json::parse(message);
    authorizationError(msg.payload.service, msg.payload.error, msg.payload.message);
}

void AuthorizationHandler::handleGetAuthorizationDataMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetAuthorizationDataMessage");
    GetAuthorizationDataMessage msg = json::parse(message);

    // Publish the reply message for getAuthorizationData.
    GetAuthorizationDataMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.data = getAuthorizationData(msg.payload.service, msg.payload.key);

    m_messageBroker->publish(replyMsg.toString());
}

void AuthorizationHandler::handleSetAuthorizationDataMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetAuthorizationDataMessage");
    SetAuthorizationDataMessage msg = json::parse(message);
    setAuthorizationData(msg.payload.service, msg.payload.key, msg.payload.data);
}

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

void AuthorizationHandler::authorizationStateChanged(const std::string& service, AuthorizationState state) {
    std::stringstream ss;
    std::string authState;
    switch (state) {
        case AuthorizationState::AUTHORIZING:
            authState = "AUTHORIZING";
            break;
        case AuthorizationState::UNAUTHORIZED:
            authState = "UNAUTHORIZED";
            break;
        case AuthorizationState::AUTHORIZED:
            authState = "AUTHORIZED";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "Invalid AuthorizationState value");
            return;
    }
    ss << "authorizationStateChanged:state=" << authState;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            std::stringstream ss;
            ss << "{\"service\":\"" << service << "\",\"state\":\"" << authState << "\"}";
            card->set(ss.str(), View::Type::AuthorizationStateChanged);
        }
    });
    if (state == AuthorizationState::AUTHORIZING) {
        m_applicationContext->setAuthorizationInProgress(service);
    } else if (state == AuthorizationState::UNAUTHORIZED) {
        m_applicationContext->setActiveAuthorization("");
        m_cv.notify_one();
    } else if (state == AuthorizationState::AUTHORIZED) {
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

void AuthorizationHandler::startAuthorization(const std::string& service, const std::string& data) {
    // Publish the "StartAuthorization" message
    StartAuthorizationMessage msg;
    msg.payload.service = service;
    msg.payload.data = data;

    m_messageBroker->publish(msg.toString());
}

void AuthorizationHandler::cancelAuthorization(const std::string& service) {
    // Publish the "CancelAuthorization" message
    CancelAuthorizationMessage msg;
    msg.payload.service = service;

    m_messageBroker->publish(msg.toString());
}

void AuthorizationHandler::sendEvent(const std::string& service, const std::string& event) {
    // Publish the "SendEvent" message
    SendEventMessage msg;
    msg.payload.service = service;
    msg.payload.event = event;

    m_messageBroker->publish(msg.toString());
}

void AuthorizationHandler::logout(const std::string& service) {
    // Publish the "Logout" message
    LogoutMessage msg;
    msg.payload.service = service;

    m_messageBroker->publish(msg.toString());
}

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
