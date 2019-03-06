/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/CBL/CBLHandler.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <sstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace cbl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CBLHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBLHandler::CBLHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> CBLHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> CBLHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::cbl::CBL interface

void CBLHandler::cblStateChanged(CBLState state, CBLStateChangedReason reason, const std::string &url, const std::string &code) {
    std::stringstream ss;
    ss << "cblStateChanged:state=" << state << ",reason=" << reason << ",url=" << url << ",code=" << code;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    switch (state) {
        case CBLState::CODE_PAIR_RECEIVED: {
            activity->runOnUIThread([=]() {
                if (auto card = activity->findViewById("id:card").lock()) {
                    auto string = "{\"code\":\"" + code + "\",\"url\":\"" + url + "\"}";
                    card->set(string, View::Type::CBLCode);
                }
            });
            auto command = m_applicationContext->getBrowserCommand();
            if (!command.empty()) {
                m_applicationContext->executeCommand((command + ' ' + url + "?cbl-code=" + code).c_str());
            }
            break;
        }
        case CBLState::STARTING:
            m_busy = true;
            break;
        case CBLState::STOPPING:
            m_busy = false;
            switch (reason) {
                case CBLStateChangedReason::CODE_PAIR_EXPIRED:
                    activity->runOnUIThread([=]() {
                        if (auto card = activity->findViewById("id:card").lock()) {
                            auto string = "{\"message\":\"The code has expired. Retry to generate a new code.\"}";
                            card->set(string, View::Type::CBLCodeExpired);
                        }
                    });
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void CBLHandler::clearRefreshToken() {
    Ensures(m_applicationContext != nullptr);
    return m_applicationContext->clearRefreshToken();
}

std::string CBLHandler::getRefreshToken() {
    Ensures(m_applicationContext != nullptr);
    return m_applicationContext->getRefreshToken();
}

void CBLHandler::setRefreshToken(const std::string &refreshToken) {
    // IMPORTANT: YOUR PRODUCT IS RESPONSIBLE FOR STORING THE REFRESH TOKEN SECURELY.
    // FOR SECURITY REASONS, AUTHENTICATION IS NOT PRESERVED IN THE C++ SAMPLE APP.
    Ensures(m_applicationContext != nullptr);
    m_applicationContext->setRefreshToken(refreshToken);
}

// private

void CBLHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "CBLHandler", message);
}

void CBLHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_applicationContext = activity->getApplicationContext();
    m_console = activity->findViewById("id:console");

    // start
    activity->registerObserver(Event::onCBLStart, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCBLStart");
        if (m_applicationContext->hasRefreshToken()) {
            if (auto console = m_console.lock()) {
                console->printLine("You already have your refresh token");
            }
            return false;
        }
        if (m_busy) {
            if (auto console = m_console.lock()) {
                console->printLine("You already started CBL login flow");
            }
            return false;
        }
        if (auto console = m_console.lock()) {
            console->printLine("Starting CBL login flow...");
        }
        start();
        return true;
    });

    // cancel
    activity->registerObserver(Event::onCBLCancel, [=](const std::string &) {
        log(logger::LoggerHandler::Level::VERBOSE, "onCBLCancel");
        if (m_applicationContext->hasRefreshToken()) {
            if (auto console = m_console.lock()) {
                console->printLine("You already have your refresh token");
            }
            return false;
        }
        if (!m_busy) {
            if (auto console = m_console.lock()) {
                console->printLine("You have not started CBL login flow");
            }
            return false;
        }
        if (auto console = m_console.lock()) {
            console->printLine("Canceling CBL login flow...");
        }
        cancel();
        return true;
    });
}

} // namespace cbl
} // namespace sampleApp
