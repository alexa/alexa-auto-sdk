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

#include "AACE/Engine/Core/EngineMacros.h"
#include "SampleApp/Connectivity/AlexaConnectivityHandler.h"

#include <AASB/Message/Connectivity/AlexaConnectivity/ConnectivityStateChangeMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/GetConnectivityStateMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/GetIdentifierMessage.h>
#include <AASB/Message/Connectivity/AlexaConnectivity/SendConnectivityEventMessage.h>

// C++ Standard Library
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace connectivity {

using namespace aasb::message::connectivity::alexaConnectivity;

std::string RFC3339(int offsetInHours = 0) {
    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now + std::chrono::hours(offsetInHours));
    std::stringstream ss;
    // The time zone provided by std::put_time %z doesn't include the colon separator specified in
    // RFC 3339 (https://tools.ietf.org/html/rfc3339#section-5.6). We either need to split the
    // [+-] hours from the minutes and rejoin with colon, or use std::gmtime and append Z.
    ss << std::put_time(std::gmtime(&t_c), "%FT%T");
    std::string value = ss.str() + "Z";
    return value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaConnectivityHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;
static const std::string CONNECTIVITY_STATE_CHANGE_REPLY("connectivityStateChangeReply");
static const std::string SEND_CONNECTIVITY_EVENT_REPLY("sendConnectivityEventReply");

using MessageBroker = aace::core::MessageBroker;

AlexaConnectivityHandler::AlexaConnectivityHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AlexaConnectivityHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlexaConnectivityHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AlexaConnectivityHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetConnectivityState Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetConnectivityStateMessage(message); },
        GetConnectivityStateMessage::topic(),
        GetConnectivityStateMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetIdentifier Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetIdentifierMessage(message); },
        GetIdentifierMessage::topic(),
        GetIdentifierMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to ConnectivityStateChangeReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleConnectivityStateChangeReplyMessage(message); },
        ConnectivityStateChangeMessageReply::topic(),
        ConnectivityStateChangeMessageReply::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to SendConnectivityEventReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSendConnectivityEventReplyMessage(message); },
        SendConnectivityEventMessageReply::topic(),
        SendConnectivityEventMessageReply::action());
}

void AlexaConnectivityHandler::handleGetConnectivityStateMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetConnectivityStateMessage");

    // Publish the "GetConnectivityStateReply" message
    GetConnectivityStateMessage msg = json::parse(message);
    GetConnectivityStateMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.connectivityState = getConnectivityState();
    m_messageBroker->publish(replyMsg.toString());
}

void AlexaConnectivityHandler::handleGetIdentifierMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetIdentifierMessage");

    // Publish the "GetIdentifierReply" message
    GetIdentifierMessage msg = json::parse(message);
    GetIdentifierMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.identifier = getIdentifier();
    m_messageBroker->publish(replyMsg.toString());
}

void AlexaConnectivityHandler::handleConnectivityStateChangeReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ConnectivityStateChangeReplyMessage");
    ConnectivityStateChangeMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    if (promise) {
        promise->set_value(msg);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "promise is null");
    }
}

void AlexaConnectivityHandler::handleSendConnectivityEventReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SendConnectivityEventReplyMessage");
    SendConnectivityEventMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    if (promise) {
        promise->set_value(msg);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "promise is null");
    }

    connectivityEventResponse(msg.header.messageDescription.replyToId, msg.payload.statusCode);
}

std::string AlexaConnectivityHandler::getConnectivityState() {
    return m_connectivityState;
}

std::string AlexaConnectivityHandler::getIdentifier() {
    auto activity = m_activity.lock();
    if (!activity) {
        log(logger::LoggerHandler::Level::ERROR, "getNetworkIdentifierFailed");
        return std::string();
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                                                                           //
    //  FOR SECURITY REASONS, GET THE IDENTIFIER FROM THE SYSTEM ONLY WHEN NEEDED, DO NOT STORE THE IDENTIFIER.  //
    //                                                                                                           //
    //  NOTE: RETURN AN EMPTY STRING TO AUTOMATICALLY USE VEHICLE_IDENTIFIER FROM ENGINE CONFIGURATION INSTEAD.  //
    //                                                                                                           //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    return activity->getApplicationContext()->getNetworkIdentifier();
}

bool AlexaConnectivityHandler::connectivityStateChange() {
    // Publish the "ConnectivityStateChange" message
    ConnectivityStateChangeMessage msg;
    m_messageBroker->publish(msg.toString());
    json replyMsg = waitForAsyncReply(msg.header.id);
    return replyMsg["payload"]["success"];
}

StatusCode AlexaConnectivityHandler::sendConnectivityEvent(const std::string& event) {
    //Publish the 'SendConnectivityEvent' message
    SendConnectivityEventMessage msg;
    msg.payload.event = event;
    m_messageBroker->publish(msg.toString());
    json replyMsg = waitForAsyncReply(msg.header.id);
    return replyMsg["payload"]["statusCode"];
}

void AlexaConnectivityHandler::connectivityEventResponse(const std::string& id, StatusCode statusCode) {
    if (statusCode == StatusCode::SUCCESS) {
        log(logger::LoggerHandler::Level::INFO, "sendConnectivityEvent Success token=" + id);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "sendConnectivityEvent Failed token=" + id);
    }
}

void AlexaConnectivityHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AlexaConnectivityHandler", message);
}

void AlexaConnectivityHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    // restore the saved connectivity state if available
    auto applicationContext = activity->getApplicationContext();
    auto path = applicationContext->getApplicationDirPath() + "/connectivityState";
    auto input = std::ifstream(path, std::ifstream::in);
    if (input.good()) {
        input >> m_connectivityState;
        input.close();
    } else {
        // initialize the connectivity state
        // clang-format off
        json value = {
            {"managedProvider",{
                {"type","NOT_MANAGED"}
            }}
        };
        // clang-format on
        m_connectivityState = value.dump();
    }
    log(logger::LoggerHandler::Level::VERBOSE, "setupUI:" + m_connectivityState);

    // connectivityStateChange
    activity->registerObserver(Event::onConnectivityConnectivityStateChange, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onConnectivityConnectivityStateChange:" + value);
        if (!value.empty()) {
            // clang-format off
            static const std::map<std::string, json> TestEnumerator{
                // Full Experience (Not Managed)
                {"NOT_MANAGED",{
                    {"managedProvider",{
                        {"type","NOT_MANAGED"}
                    }}
                }},
                // Full Experience (Paid)
                {"PAID",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","PAID"}
                    }},
                    {"termsStatus","ACCEPTED"},
                    {"termsVersion","1"}
                }},
                // Full Experience (Trial)
                {"TRIAL",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","TRIAL"},
                        {"endDate",RFC3339(30 * 24)} // 30 days from now
                    }},
                    {"dataPlansAvailable",{
                        "PAID","AMAZON_SPONSORED"
                    }},
                    {"termsStatus","ACCEPTED"},
                    {"termsVersion","1"}
                }},
                // Full Experience (Trial Expiring in 5 days)
                {"TRIAL_EXPIRING",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","TRIAL"},
                        {"endDate",RFC3339(5 * 24)} // 5 days from now
                    }},
                    {"dataPlansAvailable",{
                        "PAID","AMAZON_SPONSORED"
                    }},
                    {"termsStatus","ACCEPTED"},
                    {"termsVersion","1"}
                }},
                // Partial Experience (Amazon Sponsored)
                {"AMAZON_SPONSORED",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","AMAZON_SPONSORED"}
                    }},
                    {"dataPlansAvailable",{
                        "PAID","TRIAL"
                    }},
                    {"termsStatus","ACCEPTED"},
                    {"termsVersion","1"}
                }},
                // Partial Experience (Terms Declined)
                {"TERMS_DECLINED",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","AMAZON_SPONSORED"}
                    }},
                    {"termsStatus","DECLINED"},
                    {"termsVersion","1"}
                }},
                // Partial Experience (Terms Deferred)
                {"TERMS_DEFERRED",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","AMAZON_SPONSORED"}
                    }},
                    {"termsStatus","DEFERRED"},
                    {"termsVersion","1"}
                }},
                // Partial Experience (Trial Expired)
                {"TRIAL_EXPIRED",{
                    {"managedProvider",{
                        {"type","MANAGED"},
                        {"id","AMAZON"}
                    }},
                    {"dataPlan",{
                        {"type","AMAZON_SPONSORED"},
                    }},
                    {"dataPlansAvailable",{
                        "PAID"
                    }},
                    {"termsStatus","ACCEPTED"},
                    {"termsVersion","1"}
                }}
            };
            // clang-format on
            if (TestEnumerator.count(value) != 0) {
                m_connectivityState = TestEnumerator.at(value).dump();
            } else {
                m_connectivityState = value;  // direct string value
            }
            log(logger::LoggerHandler::Level::VERBOSE, "ConnectivityState:" + m_connectivityState);
            applicationContext->saveContent(path, m_connectivityState);
        }
        connectivityStateChange();
        return true;
    });
    activity->registerObserver(Event::onConnectivityEvent, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onConnectivityEvent:" + value);
        if (!value.empty()) {
            // clang-format off
            static const std::map<std::string, json> TestEnumerator{
                // Activate Trial Event
                {"TRIAL",{
                    {"type","ACTIVATE_TRIAL"}
                }},
                // Activate Paid Plan Event
                {"PAID_PLAN",{
                    {"type","ACTIVATE_PAID_PLAN"}
                }},
            };
            // clang-format on
            if (TestEnumerator.count(value) != 0) {
                sendConnectivityEvent(TestEnumerator.at(value).dump());
            } else {
                log(logger::LoggerHandler::Level::ERROR, "invalidConnectivityEvent.value=" + value);
            }
        }
        return true;
    });
}

json AlexaConnectivityHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the alexa connectivity reply message to fulfill
    std::shared_ptr<AlexaConnectivityPromise> promise = std::make_shared<AlexaConnectivityPromise>();

    // create a future to receive the promised alexa connectivity reply message when it is received
    std::shared_future<json> future(promise->get_future());

    json success;
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void AlexaConnectivityHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<AlexaConnectivityPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void AlexaConnectivityHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<AlexaConnectivityHandler::AlexaConnectivityPromise> AlexaConnectivityHandler::getReplyMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

}  // namespace connectivity
}  // namespace sampleApp
