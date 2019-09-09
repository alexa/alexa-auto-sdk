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
#include "CardRendererHandler.h"

#include <aasb/Consts.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace gloria {

const std::string TAG = "aasb::gloria::CardRendererHandler";

std::shared_ptr<CardRendererHandler> CardRendererHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto handler =
        std::shared_ptr<CardRendererHandler>(new CardRendererHandler(responseDispatcher));

    handler->m_logger = logger;
    return handler;
}

CardRendererHandler::CardRendererHandler(std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        m_responseDispatcher(responseDispatcher) {
}

void CardRendererHandler::renderCard(const std::string& payload) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__ + payload);
    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        responseDispatcher->sendDirective(
            aasb::bridge::TOPIC_GLORIA_CARDRENDERER, aasb::bridge::ACTION_GLORIA_CARDRENDERER_RENDERCARD, payload);
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
    }
}

void CardRendererHandler::renderPlayerInfo(const std::string& payload) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__ + payload);
    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        responseDispatcher->sendDirective(
            aasb::bridge::TOPIC_GLORIA_CARDRENDERER,
            aasb::bridge::ACTION_GLORIA_CARDRENDERER_RENDERPLAYERINFO,
            payload);
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
    }
}

void CardRendererHandler::readCard(const std::string& payload) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__ + payload);
    if (auto responseDispatcher = m_responseDispatcher.lock()) {
        responseDispatcher->sendDirective(
            aasb::bridge::TOPIC_GLORIA_CARDRENDERER, aasb::bridge::ACTION_GLORIA_CARDRENDERER_READCARD, payload);
    } else {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
    }
}

}  // namespace gloria
}  // namespace aasb