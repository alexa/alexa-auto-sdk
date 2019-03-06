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
#include "TemplateRuntimeHandler.h"

#include <aasb/Consts.h>
#include "DirectiveDispatcher.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

const std::string TAG = "aasb::alexa::TemplateRuntimeHandler";

std::shared_ptr<TemplateRuntimeHandler> TemplateRuntimeHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) {
    auto templateRuntimeHandler =
        std::shared_ptr<TemplateRuntimeHandler>(new TemplateRuntimeHandler(directiveDispatcher));

    templateRuntimeHandler->m_logger = logger;
    return templateRuntimeHandler;
}

TemplateRuntimeHandler::TemplateRuntimeHandler(std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) :
        m_directiveDispatcher(directiveDispatcher) {
}

void TemplateRuntimeHandler::renderTemplate(const std::string& payload) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__ + payload);
    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_TEMPLATE_RUNTIME, aasb::bridge::ACTION_RENDER_TEMPLATE, payload);
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }
}

void TemplateRuntimeHandler::clearTemplate() {
    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_TEMPLATE_RUNTIME, aasb::bridge::ACTION_CLEAR_TEMPLATE, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }
}

void TemplateRuntimeHandler::renderPlayerInfo(const std::string& payload) {
    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_TEMPLATE_RUNTIME, aasb::bridge::ACTION_RENDER_PLAYERINFO, payload);
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }
}

void TemplateRuntimeHandler::clearPlayerInfo() {
    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        directiveDispatcher->sendDirective(
            aasb::bridge::TOPIC_TEMPLATE_RUNTIME, aasb::bridge::ACTION_CLEAR_PLAYERINFO, "");
    } else {
        m_logger->log(Level::ERROR, TAG, "directiveDispatcher doesn't exist.");
    }
}

}  // namespace alexa
}  // namespace aasb