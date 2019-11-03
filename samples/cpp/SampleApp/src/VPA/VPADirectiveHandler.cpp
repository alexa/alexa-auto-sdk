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

#include "SampleApp/VPA/VPADirectiveHandler.h"
#include "SampleApp/ApplicationContext.h"

#include <fstream>

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "SampleApp/VPA/AIDaemon-IPC.h"
#include "SampleApp/VPA/IPCHandler.h"

namespace sampleApp {
namespace vpa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  VPADirectiveHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

VPADirectiveHandler::VPADirectiveHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler)
    : m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
}

std::weak_ptr<Activity> VPADirectiveHandler::getActivity() { return m_activity; }

std::weak_ptr<logger::LoggerHandler> VPADirectiveHandler::getLoggerHandler() { return m_loggerHandler; }

// aace::vpa::Navigation interface

bool VPADirectiveHandler::sendDirective(const std::string &payload) {
    log(logger::LoggerHandler::Level::INFO, "sendDirective");

    AIDAEMON::IPCHandler::GetInstance()->sendMessage(AIDAEMON::METHODID_NOTI_DIRECTIVE, payload);    
    return true;
}

void VPADirectiveHandler::setLocalStorage (std::shared_ptr<aace::engine::storage::LocalStorageInterface> storage) {
    log(logger::LoggerHandler::Level::INFO, "setLocalStorage");
    m_storage = storage;
}

// private

void VPADirectiveHandler::log(logger::LoggerHandler::Level level, const std::string &message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "VPADirectiveHandler", message);
}

} // namespace vpa
} // namespace sampleApp
