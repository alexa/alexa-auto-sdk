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

#include "SampleApp/Alexa/TemplateRuntimeHandler.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Views.h"

#include <AASB/Message/Alexa/TemplateRuntime/ClearPlayerInfoMessage.h>
#include <AASB/Message/Alexa/TemplateRuntime/ClearTemplateMessage.h>
#include <AASB/Message/Alexa/TemplateRuntime/RenderTemplateMessage.h>

// C++ Standard Library
#include <chrono>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::templateRuntime;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TemplateRuntimeHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

TemplateRuntimeHandler::TemplateRuntimeHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr) && (m_messageBroker != nullptr));
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> TemplateRuntimeHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> TemplateRuntimeHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void TemplateRuntimeHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to ClearPlayerInfo Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleClearPlayerInfoMessage(message); },
        ClearPlayerInfoMessage::topic(),
        ClearPlayerInfoMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to ClearTemplate Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleClearTemplateMessage(message); },
        ClearTemplateMessage::topic(),
        ClearTemplateMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to RenderPlayerInfo Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleRenderPlayerInfoMessage(message); },
        RenderPlayerInfoMessage::topic(),
        RenderPlayerInfoMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to RenderTemplate Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleRenderTemplateMessage(message); },
        RenderTemplateMessage::topic(),
        RenderTemplateMessage::action());
}

void TemplateRuntimeHandler::handleClearPlayerInfoMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ClearPlayerInfoMessage");
    clearPlayerInfo();
}

void TemplateRuntimeHandler::handleClearTemplateMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ClearTemplateMessage");
    clearTemplate();
}

void TemplateRuntimeHandler::handleRenderPlayerInfoMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received RenderPlayerInfoMessage");
    RenderPlayerInfoMessage msg = json::parse(message);
    renderPlayerInfo(
        msg.payload.payload,
        msg.payload.audioPlayerState,
        std::chrono::milliseconds(msg.payload.offset),
        msg.payload.focusState);
}

void TemplateRuntimeHandler::handleRenderTemplateMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received RenderTemplateMessage");
    RenderTemplateMessage msg = json::parse(message);
    renderTemplate(msg.payload.payload, msg.payload.focusState);
}

void TemplateRuntimeHandler::renderTemplate(const std::string& payload, FocusState focusState) {
    std::stringstream ss;
    ss << "renderTemplate:payload=" << payload << ",focusState=" << convertFocusStateToString(focusState);
    log(logger::LoggerHandler::Level::INFO, ss.str());
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
    ss << "renderPlayerInfo:payload=" << payload
       << ",audioPlayerState=" << convertPlayerActivityToString(audioPlayerState) << ",offset=" << offset.count()
       << ",focusState=" << convertFocusStateToString(focusState);
    log(logger::LoggerHandler::Level::INFO, ss.str());
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

std::string TemplateRuntimeHandler::convertFocusStateToString(FocusState enumValue) {
    switch (enumValue) {
        case (FocusState::FOREGROUND):
            return "FOREGROUND";
        case (FocusState::BACKGROUND):
            return "BACKGROUND";
        case (FocusState::NONE):
            return "NONE";
    }
    // invalid focus type
    log(logger::LoggerHandler::Level::ERROR, "Invalid Focus State.");
    return "";
}

std::string TemplateRuntimeHandler::convertPlayerActivityToString(PlayerActivity activity) {
    switch (activity) {
        case PlayerActivity::IDLE:
            return "IDLE";
        case PlayerActivity::PLAYING:
            return "PLAYING";
        case PlayerActivity::STOPPED:
            return "STOPPED";
        case PlayerActivity::PAUSED:
            return "PAUSED";
        case PlayerActivity::BUFFER_UNDERRUN:
            return "BUFFER_UNDERRUN";
        case PlayerActivity::FINISHED:
            return "FINISHED";
    }
    // invalid player activity
    log(logger::LoggerHandler::Level::ERROR, "Invalid Player Activity.");
    return "";
}

}  // namespace alexa
}  // namespace sampleApp
