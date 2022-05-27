/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/FeatureDiscoveryHandler.h"

#include <AASB/Message/Alexa/FeatureDiscovery/GetFeaturesMessage.h>

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::featureDiscovery;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FeatureDiscoveryHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

FeatureDiscoveryHandler::FeatureDiscoveryHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> FeatureDiscoveryHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> FeatureDiscoveryHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void FeatureDiscoveryHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetFeaturesReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetFeaturesReplyMessage(message); },
        GetFeaturesMessageReply::topic(),
        GetFeaturesMessageReply::action());
}

void FeatureDiscoveryHandler::handleGetFeaturesReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetFeaturesReply");
    GetFeaturesMessageReply msg = json::parse(message);
    parseDiscoveryResponses(msg.payload.discoveryResponses);
}

void FeatureDiscoveryHandler::parseDiscoveryResponses(const std::string& discoveryResponses) {
    const auto& responseArray = json::parse(discoveryResponses);
    std::stringstream ss;
    if (responseArray.empty()) {
        ss << "No feature was discovered." << std::endl;
    } else {
        for (const auto& response : responseArray) {
            if (!response.contains("localizedContent") || !response["localizedContent"].is_array()) {
                continue;
            }
            for (const auto& feature : response["localizedContent"]) {
                if (feature.contains("utteranceText")) {
                    ss << feature["utteranceText"].get<std::string>() << std::endl;
                }
            }
        }
    }
    auto activity = m_activity.lock();
    std::string consoleOutput = ss.str();
    if (activity) {
        activity->runOnUIThread([=]() {
            auto console = activity->findViewById("id:console").lock();
            if (console) {
                console->printRuler();
                console->printLine(consoleOutput);
                console->printRuler();
            }
        });
    }
}

void FeatureDiscoveryHandler::getFeatures(const std::string& domain) {
    json requestsArray = json::array();
    requestsArray.push_back({{"domain", domain}, {"eventType", "THINGS_TO_TRY"}, {"limit", 5}});
    GetFeaturesMessage msg;
    msg.payload.discoveryRequests = requestsArray.dump();
    m_messageBroker->publish(msg.toString());
}

// private

void FeatureDiscoveryHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "FeatureDiscoveryHandler", message);
}

void FeatureDiscoveryHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->registerObserver(Event::onGetFeatures, [=](const std::string& domain) {
        log(logger::LoggerHandler::Level::VERBOSE, "onGetFeatures");
        getFeatures(domain);
        return true;
    });
}

}  // namespace alexa
}  // namespace sampleApp
