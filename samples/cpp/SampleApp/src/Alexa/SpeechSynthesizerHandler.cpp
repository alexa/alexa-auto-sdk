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

#include "SampleApp/Alexa/SpeechSynthesizerHandler.h"

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SpeechSynthesizerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SpeechSynthesizerHandler::SpeechSynthesizerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    // Expects((mediaPlayer != nullptr) && (speaker != nullptr));
    setupUI();
}

std::weak_ptr<Activity> SpeechSynthesizerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> SpeechSynthesizerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// private

void SpeechSynthesizerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "SpeechSynthesizerHandler", message);
}

void SpeechSynthesizerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
}

}  // namespace alexa
}  // namespace sampleApp
