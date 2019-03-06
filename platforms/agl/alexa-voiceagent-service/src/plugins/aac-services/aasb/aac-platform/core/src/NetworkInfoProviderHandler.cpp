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
#include "NetworkInfoProviderHandler.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace core {
namespace network {

const std::string TAG = "aasb::core::network::NetworkInfoProviderHandler";

std::shared_ptr<NetworkInfoProviderHandler> NetworkInfoProviderHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger) {
    auto networkInfoProviderHandler = std::shared_ptr<NetworkInfoProviderHandler>(new NetworkInfoProviderHandler());

    networkInfoProviderHandler->m_logger = logger;

    return networkInfoProviderHandler;
}

NetworkStatus NetworkInfoProviderHandler::getNetworkStatus() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return NetworkStatus::CONNECTED;
}

int NetworkInfoProviderHandler::getWifiSignalStrength() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);
    return 0;
}

}  // namespace network
}  // namespace core
}  // namespace aasb