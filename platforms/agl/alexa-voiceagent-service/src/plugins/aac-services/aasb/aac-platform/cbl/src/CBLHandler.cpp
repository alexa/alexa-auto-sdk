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
#include "CBLHandler.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <aasb/Consts.h>
#include "DirectiveDispatcher.h"
#include "PlatformSpecificLoggingMacros.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace cbl {

using namespace rapidjson;

const std::string TAG = "aasb::alexa::CBLHandler";

std::shared_ptr<CBLHandler> CBLHandler::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) {
    auto cblHandler = std::shared_ptr<CBLHandler>(new CBLHandler(directiveDispatcher));
    cblHandler->m_logger = logger;
    return cblHandler;
}

CBLHandler::CBLHandler(std::weak_ptr<aasb::bridge::DirectiveDispatcher> directiveDispatcher) :
        m_directiveDispatcher(directiveDispatcher) {
}

void CBLHandler::cblStateChanged(CBLState state, CBLStateChangedReason reason, const std::string& url, const std::string& code) {
    std::string info = std::string(": CBLState: ") + std::string(convertCBLStateToString(state)) +
                       std::string(": CBLStateChangedReason: ") +
                       std::string(convertCBLStateChangedReasonToString(reason) +
                       std::string(": url: ") + url +
                       std::string(": code: ") + code);

    m_logger->log(Level::INFO, TAG, info);

    if (auto directiveDispatcher = m_directiveDispatcher.lock()) {
        if (state == CBLState::CODE_PAIR_RECEIVED) {
            rapidjson::Document document;
            document.SetObject();
            rapidjson::Value payloadElement;

            payloadElement.SetObject();
            payloadElement.AddMember("url", rapidjson::Value().SetString(url.c_str(), url.length()), document.GetAllocator());
            payloadElement.AddMember("code", rapidjson::Value().SetString(code.c_str(), code.length()), document.GetAllocator());

            document.AddMember("payload", payloadElement, document.GetAllocator());

            // create event string
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );

            document.Accept( writer );

            directiveDispatcher->sendDirective(
                aasb::bridge::TOPIC_CBL,
                aasb::bridge::ACTION_CBL_CODEPAIR_RECEIVED,
                buffer.GetString());
        } else if ((state == CBLState::STOPPING) && (reason == CBLStateChangedReason::CODE_PAIR_EXPIRED)) {
            m_logger->log(Level::WARN, TAG, "The code has expired. Retry to generate a new code.");
            directiveDispatcher->sendDirective(
                aasb::bridge::TOPIC_CBL,
                aasb::bridge::ACTION_CBL_CODEPAIR_EXPIRED,
                "");
        }
    }
}

void CBLHandler::clearRefreshToken() {
    m_logger->log(Level::VERBOSE, TAG, "clearRefreshToken");

    clearRefreshTokenInternal();

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "clearRefreshToken: Directive dispatcher is out of scope");
        return;
    }

    directiveDispatcher->sendDirective(aasb::bridge::TOPIC_CBL, aasb::bridge::ACTION_CBL_CLEAR_REFRESH_TOKEN, "");
}

void CBLHandler::setRefreshToken(const std::string& refreshToken) {
    m_logger->log(Level::VERBOSE, TAG, "setRefreshToken");

    setRefreshTokenInternal(refreshToken);

    auto directiveDispatcher = m_directiveDispatcher.lock();
    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "setRefreshToken: Directive dispatcher is out of scope");
        return;
    }

    directiveDispatcher->sendDirective(aasb::bridge::TOPIC_CBL, aasb::bridge::ACTION_CBL_SET_REFRESH_TOKEN, refreshToken);
}

std::string CBLHandler::getRefreshToken() {
    auto directiveDispatcher = m_directiveDispatcher.lock();

    m_logger->log(Level::VERBOSE, TAG, "getRefreshToken");

    if (!getRefreshTokenInternal().empty()) {
        m_logger->log(Level::VERBOSE, TAG, "getRefreshToken: Returning the in-memory refresh token");
        return getRefreshTokenInternal();
    }

    if (!directiveDispatcher) {
        m_logger->log(Level::WARN, TAG, "getRefreshToken: Directive dispatcher is out of scope");
        // Return available refresh token
        return getRefreshTokenInternal();
    }

    directiveDispatcher->sendDirective(aasb::bridge::TOPIC_CBL, aasb::bridge::ACTION_CBL_GET_REFRESH_TOKEN, "");

    // This specific thread could have been unblocked either after retrieving the refresh token
    // from the platform layer or we failed to get it from the platform layer, a timeout occurred
    // and so we will provide refresh token that we already have stored in memory.
    return getRefreshTokenInternal();
}

void CBLHandler::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::INFO, TAG, "onReceivedEvent: " + action);

    if (action == aasb::bridge::ACTION_CBL_GET_REFRESH_TOKEN_RESPONSE) {
        setRefreshTokenInternal(payload);
    } else if (action == aasb::bridge::ACTION_CBL_START) {
        start();
    } else if (action == aasb::bridge::ACTION_CBL_CANCEL) {
        cancel();
    } else {
        AASB_ERROR("CBLHandler: action %s is unknown.", action.c_str());
    }

    return;
}

std::string CBLHandler::convertCBLStateToString(CBLState state) {
    switch (state) {
        case CBLState::STARTING:
            return "STARTING";
        case CBLState::REQUESTING_CODE_PAIR:
            return "REQUESTING_CODE_PAIR";
        case CBLState::CODE_PAIR_RECEIVED:
            return "CODE_PAIR_RECEIVED";
        case CBLState::REFRESHING_TOKEN:
            return "REFRESHING_TOKEN";
        case CBLState::REQUESTING_TOKEN:
            return "REQUESTING_TOKEN";
        case CBLState::STOPPING:
            return "STOPPING";
        default:
            return std::string("UNKNOWN");
    }
}

std::string CBLHandler::convertCBLStateChangedReasonToString(CBLStateChangedReason reason) {
    switch (reason) {
        case CBLStateChangedReason::SUCCESS:
            return "SUCCESS";
        case CBLStateChangedReason::ERROR:
            return "ERROR";
        case CBLStateChangedReason::TIMEOUT:
            return "TIMEOUT";
        case CBLStateChangedReason::CODE_PAIR_EXPIRED:
            return "CODE_PAIR_EXPIRED";
        case CBLStateChangedReason::NONE:
            return "NONE";
        default:
            return std::string("UNKNOWN");
    }
}

void CBLHandler::setRefreshTokenInternal(const std::string& refreshToken) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_RefreshToken = refreshToken;
}

std::string CBLHandler::getRefreshTokenInternal() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_RefreshToken;
}

void CBLHandler::clearRefreshTokenInternal() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_RefreshToken.clear();
}

}  // namespace cbl
}  // namespace aasb