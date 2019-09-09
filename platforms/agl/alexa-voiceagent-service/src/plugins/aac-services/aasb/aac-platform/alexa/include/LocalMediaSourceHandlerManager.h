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
#ifndef AASB_ALEXA_LOCALMEDIASOURCEHANDLERMANAGER_H
#define AASB_ALEXA_LOCALMEDIASOURCEHANDLERMANAGER_H

#include <memory>

#include "LocalMediaSourceHandler.h"
#include "ResponseDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

class LocalMediaSourceHandlerManager {
public:
    ~LocalMediaSourceHandlerManager() {
    }

    static std::shared_ptr<LocalMediaSourceHandlerManager> create(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    // Create BT Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getBluetoothSource();

    // Create USB Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getUSBSource();

    // Create FM Radio Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getFMRadioSource();

    // Create AM Radio Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getAMRadioSource();

    // Create Satellite Radio Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getSatelliteRadioSource();

    // Create Line In Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getLineInSource();

    // Create Compact Disc Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> getCompactDiscSource();

    // Received event from application
    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    LocalMediaSourceHandlerManager(std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher);

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // aasb::bridge::DirectiveDispatcher
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;

    // BT Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_btSource;

    // USB Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_usbSource;

    // FM Radio Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_fmRadioSource;

    // AM Radio Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_amRadioSource;

    // Satellite Radio Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_satelliteRadioSource;

    // Line In Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_lineInSource;

    // Compact Disc Source
    std::shared_ptr<aasb::alexa::LocalMediaSourceHandler> m_compactDiscSource;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_LOCALMEDIASOURCEHANDLERMANAGER_H