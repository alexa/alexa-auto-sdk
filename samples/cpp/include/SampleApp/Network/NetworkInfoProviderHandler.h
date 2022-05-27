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

#ifndef SAMPLEAPP_NETWORK_NETWORKINFOPROVIDERHANDLER_H
#define SAMPLEAPP_NETWORK_NETWORKINFOPROVIDERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>
#include <AACE/Network/NetworkInfoProvider.h>

#include <AASB/Message/Network/NetworkInfoProvider/NetworkStatusChangedMessage.h>
#include <AASB/Message/Network/NetworkInfoProvider/GetNetworkStatusMessage.h>
#include <AASB/Message/Network/NetworkInfoProvider/GetWifiSignalStrengthMessage.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace network {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NetworkInfoProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class NetworkInfoProviderHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    NetworkInfoProviderHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<NetworkInfoProviderHandler> {
        return std::shared_ptr<NetworkInfoProviderHandler>(new NetworkInfoProviderHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    auto subscribeToAASBMessages() -> void;

    /**
     * Handles the GetNetworkStatusMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetNetworkStatusMessage(const std::string& message);

    /**
     * Handles the GetWifiSignalStrengthMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetWifiSignalStrengthMessage(const std::string& message);

    /**
     * Notifies the Engine of a WiFi network status change on the platform
     *
     * @param [in] status The connection status of the WiFi network
     * @param [in] wifiSignalStrength The RSSI of the WiFi connection
     */
    void networkStatusChanged(
        aasb::message::network::networkInfoProvider::NetworkStatus status,
        int wifiSignalStrength);

    /**
     * Handles the implementation of the getNetworkStatus message
     */
    aasb::message::network::networkInfoProvider::NetworkStatus getNetworkStatus();

    /**
     * Handles the implementation of the getWifiSignalStrength message
     */
    int getWifiSignalStrength();

    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;

    /// Current network status
    aasb::message::network::networkInfoProvider::NetworkStatus m_networkStatus;

    /// Current WiFi signal strength
    int m_wifiSignalStrength;
};

}  // namespace network
}  // namespace sampleApp

#endif  // SAMPLEAPP_NETWORK_NETWORKINFOPROVIDERHANDLER_H
