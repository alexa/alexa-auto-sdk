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
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) {
    return std::shared_ptr<NavigationHandler>(new NavigationHandler(logger, directiveDispatcher));
}

NavigationHandler::NavigationHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) :
        m_logger(logger),
        m_directiveDispatcher(directiveDispatcher) {

}

bool NavigationHandler::setDestination( const std::string& payload ) {
    m_logger->log(Level::VERBOSE, TAG, "setDestination payload " + payload);

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "setDestination: Directive dispatcher is out of scope");
        return false;
    }

    directiveDispatcher->sendDirective(TOPIC_NAVIGATION, ACTION_NAVIGATION_SET_DESTINATION, payload);
    return true;
}

bool NavigationHandler::cancelNavigation() {
    m_logger->log(Level::VERBOSE, TAG, "cancelNavigation");

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "cancelNavigation: Directive dispatcher is out of scope");
        return false;
    }

    directiveDispatcher->sendDirective(TOPIC_NAVIGATION, ACTION_NAVIGATION_CANCEL, "");
    return true;
}

}  // phoneCallController
}  // aasb