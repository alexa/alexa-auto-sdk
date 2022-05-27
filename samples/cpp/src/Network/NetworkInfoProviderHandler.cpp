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

#include "SampleApp/Network/NetworkInfoProviderHandler.h"

// C++ Standard Library
#include <regex>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace network {

using namespace aasb::message::network::networkInfoProvider;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NetworkInfoProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using MessageBroker = aace::core::MessageBroker;

NetworkInfoProviderHandler::NetworkInfoProviderHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)},
        m_networkStatus{NetworkStatus::CONNECTED},
        m_wifiSignalStrength{100} {
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> NetworkInfoProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> NetworkInfoProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// private
void NetworkInfoProviderHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetNetworkStatus Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetNetworkStatusMessage(message); },
        GetNetworkStatusMessage::topic(),
        GetNetworkStatusMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetWifiSignalStrength Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetWifiSignalStrengthMessage(message); },
        GetWifiSignalStrengthMessage::topic(),
        GetWifiSignalStrengthMessage::action());
}

void NetworkInfoProviderHandler::handleGetNetworkStatusMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetNetworkStatusMessage");
    GetNetworkStatusMessage _msg = json::parse(message);

    // Publish the reply message for getNetworkStatus.
    GetNetworkStatusMessageReply _replyMsg;
    _replyMsg.header.messageDescription.replyToId = _msg.header.id;
    _replyMsg.payload.status = getNetworkStatus();

    m_messageBroker->publish(_replyMsg.toString());
}

void NetworkInfoProviderHandler::handleGetWifiSignalStrengthMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetWifiSignalStrengthMessage");
    GetWifiSignalStrengthMessage _msg = json::parse(message);

    // Publish the reply message for getWifiSignalStrength.
    GetWifiSignalStrengthMessageReply _replyMsg;
    _replyMsg.header.messageDescription.replyToId = _msg.header.id;
    _replyMsg.payload.wifiSignalStrength = getWifiSignalStrength();

    m_messageBroker->publish(_replyMsg.toString());
}

void NetworkInfoProviderHandler::networkStatusChanged(NetworkStatus status, int wifiSignalStrength) {
    // Publish the "NetworkStatusChanged" message
    NetworkStatusChangedMessage _msg;
    _msg.payload.status = status;
    _msg.payload.wifiSignalStrength = wifiSignalStrength;

    m_messageBroker->publish(_msg.toString());
}

NetworkStatus NetworkInfoProviderHandler::getNetworkStatus() {
    return m_networkStatus;
}

int NetworkInfoProviderHandler::getWifiSignalStrength() {
    return m_wifiSignalStrength;
}

void NetworkInfoProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "NetworkInfoProviderHandler", message);
}

void NetworkInfoProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // networkStatusChanged
    activity->registerObserver(Event::onNetworkInfoProviderNetworkStatusChanged, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onNetworkInfoProviderNetworkStatusChanged:" + value);
        static std::regex r("([^/]+)(?:/([-+]?\\d+))?", std::regex::optimize);
        std::smatch sm{};
        if (!std::regex_match(value, sm, r) || ((sm.size() - 1) < 2)) {
            return false;
        }
        // clang-format off
        static const std::map<std::string, NetworkStatus> NetworkStatusEnumerator{
            {"UNKNOWN", NetworkStatus::UNKNOWN},
            {"DISCONNECTED", NetworkStatus::DISCONNECTED},
            {"DISCONNECTING", NetworkStatus::DISCONNECTING},
            {"CONNECTED", NetworkStatus::CONNECTED},
            {"CONNECTING", NetworkStatus::CONNECTING}
        };
        // clang-format on
        if (NetworkStatusEnumerator.count(sm[1]) == 0) {
            return false;
        }
        // Notify the Engine of a WiFi network status change on the platform
        // Store the current network status for accessor functions
        m_networkStatus = NetworkStatusEnumerator.at(sm[1]);
        if (sm[2].matched) {
            // Optional WiFi signal strength
            m_wifiSignalStrength = std::stoi(sm[2]);
        }
        networkStatusChanged(m_networkStatus, m_wifiSignalStrength);
        return true;
    });
}

}  // namespace network
}  // namespace sampleApp
