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

#include "SampleApp/PropertyManager/PropertyManagerHandler.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace propertyManager {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PropertyManagerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PropertyManagerHandler::PropertyManagerHandler(std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_loggerHandler{std::move(loggerHandler)} {
    // Expects(m_loggerHandler != nullptr);
}

std::weak_ptr<logger::LoggerHandler> PropertyManagerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::propertyManager::PropertyManager interface
void PropertyManagerHandler::propertyChanged(const std::string& key, const std::string& newValue) {
    std::stringstream ss;
    ss << "propertyChanged:key=" << key << ",new value=" << newValue;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void PropertyManagerHandler::propertyStateChanged(
    const std::string& name,
    const std::string& value,
    PropertyState state) {
    std::stringstream ss;
    ss << "propertyStateChanged:name=" << name << ",value=" << value << ",state=" << state;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

// private

void PropertyManagerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "PropertyManagerHandler", message);
}

}  // namespace propertyManager
}  // namespace sampleApp
