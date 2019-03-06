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

#include "LocationProviderHandler.h"

#include <rapidjson/document.h>

#include <aasb/Consts.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

// Namespaces.
using namespace aasb::bridge;

// Logging tag for this file.
const std::string TAG = "aasb::location::LocationProviderHandler";

namespace aasb {
namespace location {

std::shared_ptr<LocationProviderHandler> LocationProviderHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::shared_ptr<aasb::bridge::IConfigurationProvider> config,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) {
    return std::shared_ptr<LocationProviderHandler>(new LocationProviderHandler(logger, config, directiveDispatcher));
}

LocationProviderHandler::LocationProviderHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::shared_ptr<aasb::bridge::IConfigurationProvider> config,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) :
        m_logger(logger),
        m_config(config),
        m_directiveDispatcher(directiveDispatcher) {

}

aace::location::Location LocationProviderHandler::getLocation() {
    m_logger->log(Level::VERBOSE, TAG, "getLocation");

    auto location = m_config->getCurrentLocation();
    return aace::location::Location(location.first, location.second);
}

std::string LocationProviderHandler::getCountry() {
    m_logger->log(Level::VERBOSE, TAG, "getCountry");

    return m_config->getCountry();
}

}  // phoneCallController
}  // aasb