/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AlertsHandler.h"

#include <sstream>
#include <aasb/Consts.h>
#include "PlatformSpecificLoggingMacros.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

const std::string TAG = "aasb::alexa::AlertsHandler";

std::shared_ptr<AlertsHandler> AlertsHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto alertsHandler = std::shared_ptr<AlertsHandler>(new AlertsHandler(logger, responseDispatcher));
    return alertsHandler;
}

AlertsHandler::AlertsHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        m_logger(logger), m_responseDispatcher(responseDispatcher) {
}

void AlertsHandler::alertStateChanged(const std::string &alertToken, Alerts::AlertState state, const std::string &reason) {
    std::stringstream ss;
    ss << state;
    m_logger->log(Level::INFO, TAG, "alertChanged: " + alertToken + " state: " + ss.str() + " reason: " + reason);
}

void AlertsHandler::alertCreated(const std::string &alertToken, const std::string &detailedInfo) {
    m_logger->log(Level::INFO, TAG, "alertCreated: " + alertToken + " info: " + detailedInfo);
}

void AlertsHandler::alertDeleted(const std::string &alertToken) {
    m_logger->log(Level::INFO, TAG, "alertDeleted: " + alertToken);
}

} // namespace alexa
} // namespace aasb