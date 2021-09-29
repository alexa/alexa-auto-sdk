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

#include "SampleApp/LocalNavigation/LocalSearchProviderHandler.h"

namespace sampleApp {
namespace localNavigation {

REGISTER_EXTENSION("LocalNavigation", LocalSearchProviderExtension);

using json = nlohmann::json;

/// String to identify log entries originating from this file.
static const std::string TAG("LocalSearchProviderHandler");

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalSearchProviderExtension
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool LocalSearchProviderExtension::initialize(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::weak_ptr<aace::propertyManager::PropertyManager> propertyManager) {
    m_localSearchProviderHandler = LocalSearchProviderHandler::create(loggerHandler);
    return true;
}

bool LocalSearchProviderExtension::validate(const std::vector<nlohmann::json>& configs) {
    // LVC Configuration must be passed
    for (auto const& j : configs) {
        try {
            if (j.contains("aace.localVoiceControl") && j.contains("aace.localSkillService") &&
                j.contains("aace.localNavigation")) {
                return true;
            }
        } catch (nlohmann::json::exception& e) {
        }
    }
    return false;
}

std::shared_ptr<aace::core::PlatformInterface> LocalSearchProviderExtension::getPlatformInterfaceHandler() {
    return m_localSearchProviderHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalSearchProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

LocalSearchProviderHandler::LocalSearchProviderHandler(std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_loggerHandler{std::move(loggerHandler)} {
}

bool LocalSearchProviderHandler::searchRequest(const std::string& request) {
    log(logger::LoggerHandler::Level::INFO, "Handling search request");
    std::string id;
    bool hasId = getRequestId(request, id);
    if (!hasId) {
        log(logger::LoggerHandler::Level::ERROR, "Could not extract requestId from search request");
        return false;
    }
    // Your implementation should delegate the request to a navigation provider, return from
    // this method immediately, and call the searchResponse() callback when the async result
    // is available from the provider. This sample produces an error response instead.
    std::string response = createErrorResponse(id);
    log(logger::LoggerHandler::Level::INFO, "Using error response: " + response);
    searchResponse(response);
    return true;
}

bool LocalSearchProviderHandler::lookupRequest(const std::string& request) {
    log(logger::LoggerHandler::Level::INFO, "Handling lookup request");
    std::string id;
    bool hasId = getRequestId(request, id);
    if (!hasId) {
        log(logger::LoggerHandler::Level::ERROR, "Could not extract requestId from lookup request");
        return false;
    }
    // Your implementation should delegate the request to a navigation provider, return from
    // this method immediately, and call the lookupResponse() callback when the async result
    // is available from the provider. This sample produces an error response instead.
    std::string response = createErrorResponse(id);
    log(logger::LoggerHandler::Level::INFO, "Using error response: " + response);
    lookupResponse(response);
    return true;
}

bool LocalSearchProviderHandler::getRequestId(const std::string request, std::string& requestId) {
    json jrequest;
    try {
        jrequest = json::parse(request);
    } catch (json::parse_error& ex) {
        log(logger::LoggerHandler::Level::ERROR, "Request is not valid JSON");
        return false;
    }
    try {
        requestId = jrequest.at("requestId");
    } catch (json::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "Missing requestId");
        return false;
    }
    return true;
}

std::string LocalSearchProviderHandler::createErrorResponse(std::string requestId) {
    // clang-format off
    json errorResponse = {
        {"requestId", requestId},
        {"status", "FAIL"},
        {"error", {
            {"errorCode", "INTERNAL_ERROR"},
            {"errorMessage", "Default sample does not implement search"},
        }
    }};
    // clang-format on
    return errorResponse.dump();
}

void LocalSearchProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, TAG, message);
}

}  // namespace localNavigation
}  // namespace sampleApp
