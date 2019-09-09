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

#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

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
static const std::chrono::seconds TIME_OUT_IN_SECS{5};

namespace aasb {
namespace location {

std::shared_ptr<LocationProviderHandler> LocationProviderHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::shared_ptr<aasb::bridge::IConfigurationProvider> config,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    return std::shared_ptr<LocationProviderHandler>(new LocationProviderHandler(logger, config, responseDispatcher));
}

LocationProviderHandler::LocationProviderHandler(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::shared_ptr<aasb::bridge::IConfigurationProvider> config,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        m_logger(logger),
        m_config(config),
        m_responseDispatcher(responseDispatcher),
        m_location_set(false),
        m_current_location(0, 0) {
}

aace::location::Location LocationProviderHandler::getLocation() {
    m_logger->log(Level::VERBOSE, TAG, "getLocation");

    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        responseDispatcher->sendDirective(TOPIC_LOCATIONPROVIDER, ACTION_LOCATION_REQUEST_CURRENT_LOCATION, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "LocationProviderHandler: responseDispatcher doesn't exist.");
        return m_current_location;
    }

    std::unique_lock<std::mutex> lock(m_mutex_location);
    // Block until we receive Location
    if (m_cv_location.wait_for(lock, TIME_OUT_IN_SECS, [this]() { return m_location_set == true; })) {
        m_location_set = false;
    } else {
        std::stringstream logTxt;
        logTxt << "Timeout in fetching location. Sending last location " << m_current_location.getLatitude() << ":"
               << m_current_location.getLongitude();
        m_logger->log(Level::WARN, TAG, logTxt.str().c_str());
    }

    return m_current_location;
}

void LocationProviderHandler::onLocationReceived(const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "onLocationReceived " + payload);

    rapidjson::Document document;
    rapidjson::ParseResult parseResult = document.Parse(payload.c_str());
    if (!parseResult) {
        m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
        return;
    }

    auto root = document.GetObject();

    double latitude = aace::location::Location::UNDEFINED, longitude = aace::location::Location::UNDEFINED;
    double altitude = aace::location::Location::UNDEFINED, accuracy = aace::location::Location::UNDEFINED;
    uint64_t timeSinceEpohMsRaw = 0;

    do {
        if (root.HasMember(JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED.c_str()) &&
            root[JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED.c_str()].IsBool()) {
            bool isSucceeded = root[JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED.c_str()].GetBool();
            if (!isSucceeded) {
                m_logger->log(Level::VERBOSE, TAG, "Cannot obtain location.");
                break;
            }
        } else {
            m_logger->log(
                Level::WARN, TAG, "onLocationReceived: " + JSON_ATTR_LOCATION_IS_REQUEST_SUCCEEDED + " not found");
            break;
        }

        if (root.HasMember(JSON_ATTR_LOCATION_LATITUDE.c_str()) &&
            root[JSON_ATTR_LOCATION_LATITUDE.c_str()].IsDouble()) {
            latitude = root[JSON_ATTR_LOCATION_LATITUDE.c_str()].GetDouble();
        } else {
            m_logger->log(Level::WARN, TAG, "onLocationReceived: " + JSON_ATTR_LOCATION_LATITUDE + " not found");
            break;
        }

        if (root.HasMember(JSON_ATTR_LOCATION_LONGITUDE.c_str()) &&
            root[JSON_ATTR_LOCATION_LONGITUDE.c_str()].IsDouble()) {
            longitude = root[JSON_ATTR_LOCATION_LONGITUDE.c_str()].GetDouble();
        } else {
            m_logger->log(Level::WARN, TAG, "onLocationReceived: " + JSON_ATTR_LOCATION_LONGITUDE + " not found");
            break;
        }

        if (root.HasMember(JSON_ATTR_LOCATION_ALTITUDE.c_str()) &&
            root[JSON_ATTR_LOCATION_ALTITUDE.c_str()].IsDouble()) {
            altitude = root[JSON_ATTR_LOCATION_ALTITUDE.c_str()].GetDouble();
        }

        if (root.HasMember(JSON_ATTR_LOCATION_ACCURACY.c_str()) &&
            root[JSON_ATTR_LOCATION_ACCURACY.c_str()].IsDouble()) {
            accuracy = root[JSON_ATTR_LOCATION_ACCURACY.c_str()].GetDouble();
        }

        if (root.HasMember(JSON_ATTR_LOCATION_TIME_SINCE_EPOH_MS.c_str()) &&
            root[JSON_ATTR_LOCATION_TIME_SINCE_EPOH_MS.c_str()].IsUint64()) {
            timeSinceEpohMsRaw = root[JSON_ATTR_LOCATION_TIME_SINCE_EPOH_MS.c_str()].GetUint64();
        }
    } while (0);

    {
        std::lock_guard<std::mutex> lock(m_mutex_location);
        if (timeSinceEpohMsRaw != 0) {
            std::chrono::microseconds timeSinceEpohMs(timeSinceEpohMsRaw);
            std::chrono::system_clock::time_point locationTime(timeSinceEpohMs);
            m_current_location = aace::location::Location(latitude, longitude, altitude, accuracy, locationTime);
        } else {
            m_current_location = aace::location::Location(latitude, longitude, altitude, accuracy);
        }
        m_location_set = true;
    }
    m_cv_location.notify_one();
}

std::string LocationProviderHandler::getCountry() {
    m_logger->log(Level::VERBOSE, TAG, "getCountry");

    return m_config->getCountry();
}

void LocationProviderHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "processing action: " + action);

    if (action == ACTION_LOCATION_RESPONSE_CURRENT_LOCATION) {
        onLocationReceived(payload);
    } else {
        m_logger->log(Level::WARN, TAG, "Unrecognized action: " + action);
    }
}

}  // namespace location
}  // namespace aasb