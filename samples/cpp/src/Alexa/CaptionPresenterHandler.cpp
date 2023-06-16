/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/Alexa/CaptionPresenterHandler.h"
#include "SampleApp/ApplicationContext.h"
#include <AASB/Message/Alexa/CaptionPresenter/SetCaptionMessage.h>

namespace sampleApp {
namespace alexa {

using namespace aasb::message::alexa::captionPresenter;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CaptionPresenterHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// aliases
using MessageBroker = aace::core::MessageBroker;

CaptionPresenterHandler::CaptionPresenterHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> CaptionPresenterHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> CaptionPresenterHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void CaptionPresenterHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetCaption Message");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetCaptionMessage(message); },
        SetCaptionMessage::topic(),
        SetCaptionMessage::action());
}

void CaptionPresenterHandler::handleSetCaptionMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetCaptionMessage");
    SetCaptionMessage aasbMessage = json::parse(message);

    log(logger::LoggerHandler::Level::INFO, aasbMessage.payload.caption);
}

void CaptionPresenterHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "CaptionPresenterHandler", message);
}

}  // namespace alexa
}  // namespace sampleApp