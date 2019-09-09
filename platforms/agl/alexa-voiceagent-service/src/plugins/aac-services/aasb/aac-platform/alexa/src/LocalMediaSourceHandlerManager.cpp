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
#include "LocalMediaSourceHandlerManager.h"

#include <aasb/Consts.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;

namespace aasb {
namespace alexa {

using namespace aasb::bridge;

const std::string TAG = "aasb::alexa::LocalMediaSourceHandlerManager";

std::shared_ptr<LocalMediaSourceHandlerManager> LocalMediaSourceHandlerManager::create(
    std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    auto localMediaSourceHandlerManager =
        std::shared_ptr<LocalMediaSourceHandlerManager>(new LocalMediaSourceHandlerManager(responseDispatcher));

    localMediaSourceHandlerManager->m_logger = logger;

    return localMediaSourceHandlerManager;
}

LocalMediaSourceHandlerManager::LocalMediaSourceHandlerManager(
    std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher) {
    m_responseDispatcher = responseDispatcher;
}

// Create BT Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getBluetoothSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_btSource.get()) {
        m_btSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::BLUETOOTH, m_logger, m_responseDispatcher);
    }

    return m_btSource;
}

// Create USB Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getUSBSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_usbSource.get()) {
        m_usbSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::USB, m_logger, m_responseDispatcher);
    }

    return m_usbSource;
}

// Create FM Radio Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getFMRadioSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_fmRadioSource.get()) {
        m_fmRadioSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::FM_RADIO, m_logger, m_responseDispatcher);
    }

    return m_fmRadioSource;
}

// Create AM Radio Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getAMRadioSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_amRadioSource.get()) {
        m_amRadioSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::AM_RADIO, m_logger, m_responseDispatcher);
    }

    return m_amRadioSource;
}

// Create Satellite Radio Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getSatelliteRadioSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_satelliteRadioSource.get()) {
        m_satelliteRadioSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO, m_logger, m_responseDispatcher);
    }

    return m_satelliteRadioSource;
}

// Create Line In Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getLineInSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_lineInSource.get()) {
        m_lineInSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::LINE_IN, m_logger, m_responseDispatcher);
    }

    return m_lineInSource;
}

// Create Compact Disc Source
std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> LocalMediaSourceHandlerManager::getCompactDiscSource() {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    if (nullptr == m_compactDiscSource.get()) {
        m_compactDiscSource = aasb::alexa::LocalMediaSourceHandler::create(
            aace::alexa::LocalMediaSource::Source::COMPACT_DISC, m_logger, m_responseDispatcher);
    }

    return m_compactDiscSource;
}

// Received event from application layer
void LocalMediaSourceHandlerManager::onReceivedEvent(const std::string& action, const std::string& payload) {
    m_logger->log(Level::VERBOSE, TAG, __FUNCTION__);

    rapidjson::Document document;
    document.Parse(payload.c_str());
    auto root = document.GetObject();

    if (root.HasMember("source") && root["source"].IsString()) {
        auto source = root["source"].GetString();
        if (VALUE_LOCAL_MEDIA_SOURCE_BLUETOOTH.compare(source) == 0) {
            m_btSource->onReceivedEvent(action, payload);
        } else if (VALUE_LOCAL_MEDIA_SOURCE_USB.compare(source) == 0) {
            m_usbSource->onReceivedEvent(action, payload);
        } else if (VALUE_LOCAL_MEDIA_SOURCE_FM_RADIO.compare(source) == 0) {
            m_fmRadioSource->onReceivedEvent(action, payload);
        } else if (VALUE_LOCAL_MEDIA_SOURCE_AM_RADIO.compare(source) == 0) {
            m_amRadioSource->onReceivedEvent(action, payload);
        } else if (VALUE_LOCAL_MEDIA_SOURCE_SATELLITE_RADIO.compare(source) == 0) {
            m_satelliteRadioSource->onReceivedEvent(action, payload);
        } else if (VALUE_LOCAL_MEDIA_SOURCE_LINE_IN.compare(source) == 0) {
            m_lineInSource->onReceivedEvent(action, payload);
        } else if (VALUE_LOCAL_MEDIA_SOURCE_COMPACT_DISC.compare(source) == 0) {
            m_compactDiscSource->onReceivedEvent(action, payload);
        } else {
            m_logger->log(Level::ERROR, TAG, "onReceivedEvent: Unknown source " + std::string(source));
        }
    } else {
        m_logger->log(Level::ERROR, TAG, "onReceivedEvent: No source specified " + payload);
    }
}

}  // namespace alexa
}  // namespace aasb