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

#include "SampleApp/Location/LocationProviderHandler.h"
#include "SampleApp/ApplicationContext.h"

#include <AASB/Message/Location/LocationProvider/GetCountryMessage.h>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace location {

using namespace aasb::message::location::locationProvider;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocationProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

using MessageBroker = aace::core::MessageBroker;

LocationProviderHandler::LocationProviderHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> LocationProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> LocationProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void LocationProviderHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetCountry Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetCountryMessage(message); },
        GetCountryMessage::topic(),
        GetCountryMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetLocation Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetLocationMessage(message); },
        GetLocationMessage::topic(),
        GetLocationMessage::action());
}

void LocationProviderHandler::handleGetCountryMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetCountryMessage");
    GetCountryMessage msg = json::parse(message);

    // Publish the reply message for getCountry.
    GetCountryMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.country = getCountry();

    m_messageBroker->publish(replyMsg.toString());
}

void LocationProviderHandler::handleGetLocationMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetLocationMessage");
    GetLocationMessage msg = json::parse(message);

    // Publish the reply message for getLocation.
    GetLocationMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.location = getLocation();

    m_messageBroker->publish(replyMsg.toString());
}

void LocationProviderHandler::locationServiceAccessChanged(
    aasb::message::location::locationProvider::LocationServiceAccess access) {
    // Publish the "LocationServiceAccessChanged" message
    LocationServiceAccessChangedMessage msg;
    msg.payload.access = access;
    m_messageBroker->publish(msg.toString());
}

std::string LocationProviderHandler::getCountry() {
    log(logger::LoggerHandler::Level::INFO, "getCountry");
    return "";
}

aasb::message::location::locationProvider::Location LocationProviderHandler::getLocation() {
    double latitude = 0.0, longitude = 0.0;
    if (auto activity = m_activity.lock()) {
        auto applicationContext = activity->getApplicationContext();
        auto value = applicationContext->getMenuItemValue("LOCATION", nullptr);
        if (value.is_object() && value.count("latitude") && value.count("longitude")) {
            latitude = value.at("latitude").get<double>();
            longitude = value.at("longitude").get<double>();
        }
    }

    if (latitude == 0.0 && longitude == 0.0) {
        latitude = -1;
        longitude = -1;
    }

    if (auto console = m_console.lock()) {
        console->printLine("Location", latitude, longitude);
    }

    aasb::message::location::locationProvider::Location location;
    location.latitude = latitude;
    location.longitude = longitude;

    return location;
}

void LocationProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "LocationProviderHandler", message);
}

void LocationProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // LocationServiceAccess set to ENABLED
    activity->registerObserver(Event::onLocationProviderLocationServiceAccessEnabled, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onLocationProviderLocationServiceAccessEnabled");
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("location service access enabled");
                console->printRuler();
            }
        });
        locationServiceAccessChanged(LocationServiceAccess::ENABLED);
        return true;
    });

    // LocationServiceAccess set to DISABLED
    activity->registerObserver(Event::onLocationProviderLocationServiceAccessDisabled, [=](const std::string&) {
        log(logger::LoggerHandler::Level::VERBOSE, "onLocationProviderLocationServiceAccessDisabled");
        activity->runOnUIThread([=]() {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("location service access disabled");
                console->printRuler();
            }
        });
        locationServiceAccessChanged(LocationServiceAccess::DISABLED);
        return true;
    });
}

}  // namespace location
}  // namespace sampleApp
