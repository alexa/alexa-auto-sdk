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
#include "ListRendererHandler.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <aasb/Consts.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;
using namespace aasb::bridge;

namespace aasb {
namespace gloria {

const std::string TAG = "aasb::gloria::ListRendererHandler";

std::shared_ptr<ListRendererHandler> ListRendererHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto handler =
        std::shared_ptr<ListRendererHandler>(new ListRendererHandler(responseDispatcher));

    handler->m_logger = logger;
    return handler;
}

ListRendererHandler::ListRendererHandler(std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) :
        m_responseDispatcher(responseDispatcher) {
}

void ListRendererHandler::handleDirective(
        const std::string& ns,
        const std::string& name,
        const std::string& messageId,
        const std::string& dialogRequestId,
        const std::string& payload) {
    m_logger->log(Level::INFO, TAG, __FUNCTION__ + payload);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::ERROR, TAG, "responseDispatcher doesn't exist.");
        return;
    }

    rapidjson::Document document;
    document.SetObject();
    rapidjson::Value payloadElement;

    payloadElement.SetObject();
    payloadElement.AddMember(
        rapidjson::Value(JSON_ATTR_GLORIA_LISTRENDERER_NAMESPACE.c_str(), document.GetAllocator()).Move(),
        rapidjson::Value().SetString(ns.c_str(), ns.length()),
        document.GetAllocator());
    payloadElement.AddMember(
        rapidjson::Value(JSON_ATTR_GLORIA_LISTRENDERER_NAME.c_str(), document.GetAllocator()).Move(),
        rapidjson::Value().SetString(name.c_str(), name.length()),
        document.GetAllocator());
    payloadElement.AddMember(
        rapidjson::Value(JSON_ATTR_GLORIA_LISTRENDERER_MESSAGEID.c_str(), document.GetAllocator()).Move(),
        rapidjson::Value().SetString(messageId.c_str(), messageId.length()),
        document.GetAllocator());
    payloadElement.AddMember(
        rapidjson::Value(JSON_ATTR_GLORIA_LISTRENDERER_DIALOGREQUESTID.c_str(), document.GetAllocator()).Move(),
        rapidjson::Value().SetString(dialogRequestId.c_str(), dialogRequestId.length()),
        document.GetAllocator());
    payloadElement.AddMember(
        rapidjson::Value(JSON_ATTR_GLORIA_LISTRENDERER_PAYLOAD.c_str(), document.GetAllocator()).Move(),
        rapidjson::Value().SetString(payload.c_str(), payload.length()),
        document.GetAllocator());

    document.AddMember("payload", payloadElement, document.GetAllocator());

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );
    document.Accept( writer );

    responseDispatcher->sendDirective(
        TOPIC_GLORIA_LISTRENDERER,
        ACTION_GLORIA_LISTRENDERER_HANDLE_DIRECTIVE,
        buffer.GetString());
}

void ListRendererHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, "processing action: " + action);

    if (action == ACTION_GLORIA_LISTRENDERER_SEND_EVENT) {
        sendEvent(payload);
    } else if (action == ACTION_GLORIA_LISTRENDERER_RENDERED_LISTSTATE_CHANGED) {
        renderedListStateChanged(payload);
    } else {
        m_logger->log(Level::WARN, TAG, "Unknown action: " + action);
    }
}

void ListRendererHandler::sendEvent(const std::string& payload) {
    rapidjson::Document document;
    rapidjson::ParseResult parseResult = document.Parse(payload.c_str());
    if (!parseResult) {
        m_logger->log(Level::WARN, TAG, rapidjson::GetParseError_En(parseResult.Code()));
        return;
    }
    auto root = document.GetObject();

    std::string ns, name, eventPayload;

    if (root.HasMember(JSON_ATTR_GLORIA_LISTRENDERER_NAMESPACE.c_str()) && root[JSON_ATTR_GLORIA_LISTRENDERER_NAMESPACE.c_str()].IsString()) {
        ns = root[JSON_ATTR_GLORIA_LISTRENDERER_NAMESPACE.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "sendEvent: gloria list renderer namespace not found.");
        return;
    }

    if (root.HasMember(JSON_ATTR_GLORIA_LISTRENDERER_NAME.c_str()) && root[JSON_ATTR_GLORIA_LISTRENDERER_NAME.c_str()].IsString()) {
        name = root[JSON_ATTR_GLORIA_LISTRENDERER_NAME.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "sendEvent: gloria list renderer name not found.");
        return;
    }

    if (root.HasMember(JSON_ATTR_GLORIA_LISTRENDERER_PAYLOAD.c_str()) && root[JSON_ATTR_GLORIA_LISTRENDERER_PAYLOAD.c_str()].IsString()) {
        eventPayload = root[JSON_ATTR_GLORIA_LISTRENDERER_PAYLOAD.c_str()].GetString();
    } else {
        m_logger->log(Level::WARN, TAG, "sendEvent: gloria list renderer payload not found.");
        return;
    }

    ListRenderer::sendEvent(ns, name, eventPayload);
}

void ListRendererHandler::renderedListStateChanged(const std::string& payload) {
    ListRenderer::renderedListStateChanged(payload);
}


}  // namespace gloria
}  // namespace aasb