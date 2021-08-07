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

#include "SampleApp/Connectivity/AlexaConnectivityHandler.h"

// C++ Standard Library
#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace connectivity {

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

AlexaConnectivityHandler::AlexaConnectivityHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> AlexaConnectivityHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlexaConnectivityHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::connectivity::AlexaConnectivity interface

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

void AlexaConnectivityHandler::connectivityEventResponse(const std::string& id, StatusCode statusCode) {
    if (statusCode == StatusCode::SUCCESS) {
        log(logger::LoggerHandler::Level::INFO, "sendConnectivityEvent Success token=" + id);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "sendConnectivityEvent Failed token=" + id);
    }
}

// private

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

}  // namespace connectivity
}  // namespace sampleApp
