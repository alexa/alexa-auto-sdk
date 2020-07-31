/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/TemplateRuntimeHandler.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Views.h"

// C++ Standard Library
#include <chrono>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TemplateRuntimeHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

TemplateRuntimeHandler::TemplateRuntimeHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> TemplateRuntimeHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> TemplateRuntimeHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::TemplateRuntime interface

void TemplateRuntimeHandler::renderTemplate(const std::string& payload, FocusState focusState) {
    std::stringstream ss;
    ss << "focusState=" << focusState;
    log(logger::LoggerHandler::Level::INFO, "renderTemplate:payload=" + payload + "," + ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    if (wasPayloadJustSeen(payload)) {
        //ignore
        return;
    }

    m_startTemplate = std::chrono::system_clock::now();
    activity->runOnUIThread([=]() {
        auto applicationContext = activity->getApplicationContext();
        auto command = applicationContext->getPayloadScriptCommand();
        if (!command.empty()) {
            if (auto console = m_console.lock()) {
                auto path = applicationContext->makeTempPath("Template", "json");
                if (applicationContext->saveContent(path, payload)) {
                    auto result = applicationContext->executeCommand((command + " Template " + path).c_str());
                    if (!result.empty()) {
                        console->print(result);
                        return;
                    }
                } else {
                    console->printLine("Template script error");
                    return;
                }
            }
        }
        if (auto card = activity->findViewById("id:card").lock()) {
            card->set(payload, View::Type::Template);
        }
    });
}

void TemplateRuntimeHandler::clearTemplate() {
    auto elapsed = std::chrono::system_clock::now() - m_startTemplate;
    unsigned seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    log(logger::LoggerHandler::Level::INFO, "clearTemplate:?elapsed=" + std::to_string(seconds) + "s");
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->clear(View::Type::Template);
        }
    });
}

void TemplateRuntimeHandler::renderPlayerInfo(
    const std::string& payload,
    PlayerActivity audioPlayerState,
    std::chrono::milliseconds offset,
    FocusState focusState) {
    std::stringstream ss;
    ss << "audioPlayerState=" << audioPlayerState << ",offset=" << offset.count() << ",focusState=" << focusState;
    log(logger::LoggerHandler::Level::INFO, "renderPlayerInfo:payload=" + payload + "," + ss.str());
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    if (wasPayloadJustSeen(payload)) {
        //ignore
        return;
    }

    m_startPlayerInfo = std::chrono::system_clock::now();
    activity->runOnUIThread([=]() {
        auto applicationContext = activity->getApplicationContext();
        auto command = applicationContext->getPayloadScriptCommand();
        if (!command.empty()) {
            if (auto console = m_console.lock()) {
                auto path = applicationContext->makeTempPath("PlayerInfo", "json");
                if (applicationContext->saveContent(path, payload)) {
                    auto result = applicationContext->executeCommand((command + " PlayerInfo " + path).c_str());
                    if (!result.empty()) {
                        console->print(result);
                        return;
                    }
                } else {
                    console->printLine("PlayerInfo script error");
                    return;
                }
            }
        }
        if (auto card = activity->findViewById("id:card").lock()) {
            card->set(payload, View::Type::PlayerInfo);
        }
    });
}

void TemplateRuntimeHandler::clearPlayerInfo() {
    auto elapsed = std::chrono::system_clock::now() - m_startPlayerInfo;
    unsigned seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    log(logger::LoggerHandler::Level::INFO, "clearPlayerInfo:?elapsed=" + std::to_string(seconds) + "s");
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto card = activity->findViewById("id:card").lock()) {
            card->clear(View::Type::PlayerInfo);
        }
    });
}

// private

void TemplateRuntimeHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "TemplateRuntimeHandler", message);
}

void TemplateRuntimeHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

bool TemplateRuntimeHandler::wasPayloadJustSeen(const std::string& payload) {
    auto elapsed = std::chrono::steady_clock::now() - m_whenCachedLastpayload;
    unsigned milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    const unsigned MIN_MILLISECONDS_ELAPSED_FOR_PRINTING_SAME_RESULT = 1000;
    if (milliseconds < MIN_MILLISECONDS_ELAPSED_FOR_PRINTING_SAME_RESULT) {
        if (m_lastPayload == payload) {
            return true;
        }
    }

    m_lastPayload = payload;
    m_whenCachedLastpayload = std::chrono::steady_clock::now();

    return false;
}

}  // namespace alexa
}  // namespace sampleApp
