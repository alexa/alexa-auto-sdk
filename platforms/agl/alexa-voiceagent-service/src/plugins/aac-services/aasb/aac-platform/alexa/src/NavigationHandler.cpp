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

#include "NavigationHandler.h"

#include <aasb/Consts.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

// Namespaces.
using namespace aasb::bridge;

// Logging tag for this file.
const std::string TAG = "aasb::navigation::NavigationHandler";

namespace aasb {
namespace navigation {

std::shared_ptr<NavigationHandler>NavigationHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    return std::shared_ptr<NavigationHandler>(new NavigationHandler(logger, responseDispatcher));
}

NavigationHandler::NavigationHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        m_navigationState(""),
        m_logger(logger),
        m_responseDispatcher(responseDispatcher) {
    m_navigationState = createNavigationState("NOT_NAVIGATING");
}

bool NavigationHandler::setDestination( const std::string& payload ) {
    m_logger->log(Level::VERBOSE, TAG, "setDestination");

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::WARN, TAG, "setDestination: Directive dispatcher is out of scope");
        return false;
    }

    responseDispatcher->sendDirective(TOPIC_NAVIGATION, ACTION_NAVIGATION_SET_DESTINATION, payload);
    m_navigationState = createNavigationState("NAVIGATING");

    return true;
}

bool NavigationHandler::cancelNavigation() {
    m_logger->log(Level::VERBOSE, TAG, "cancelNavigation");

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::WARN, TAG, "cancelNavigation: Directive dispatcher is out of scope");
        return false;
    }

    responseDispatcher->sendDirective(TOPIC_NAVIGATION, ACTION_NAVIGATION_CANCEL, "");
    m_navigationState = createNavigationState("NOT_NAVIGATING");

    return true;
}

std::string NavigationHandler::getNavigationState() {
    m_logger->log(Level::VERBOSE,TAG, "getNavigationState: " + m_navigationState);

    return m_navigationState;
}

std::string NavigationHandler::createNavigationState(std::string state) {
    rapidjson::Document document;
    document.SetObject();

    // Add state
    document.AddMember(
        "state",
        rapidjson::Value().SetString(state.c_str(), state.length()),
        document.GetAllocator());

    // Add waypoints
    rapidjson::Value wayPoints(rapidjson::kArrayType);
    document.AddMember(
        "waypoints",
        wayPoints,
        document.GetAllocator());

    // Add shapes
    rapidjson::Value shapes(rapidjson::kArrayType);
    document.AddMember(
        "shapes",
        shapes,
        document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

}  // phoneCallController
}  // aasb