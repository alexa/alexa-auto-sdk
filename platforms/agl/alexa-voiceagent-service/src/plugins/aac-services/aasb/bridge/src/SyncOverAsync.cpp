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

#include "SyncOverAsync.h"

/**
 * Specifies the severity level of a log message
 * @sa @c aace::logger::LoggerEngineInterface::Level
 */
using Level = aace::logger::LoggerEngineInterface::Level;
using namespace aasb::bridge;

namespace aasb {
namespace bridge {

const std::string TAG = "aasb::bridge::SyncOverAsync";


SyncOverAsync::SyncOverAsync(
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher,
        std::chrono::microseconds waitDuration) :
        m_logger(logger),
        m_responseDispatcher(responseDispatcher),
        m_waitDuration(waitDuration) {

}

bool SyncOverAsync::makeCallAndWaitForResponse(
        const std::string& topic, const std::string& action, const std::string& payload,
        std::string& response) {
    m_logger->log(Level::VERBOSE, TAG, "Making async call for topic " + topic + " action " + action);

    auto responseDispatcher = m_responseDispatcher.lock();
    if (!responseDispatcher) {
        m_logger->log(Level::WARN, TAG, "responseDispatcher doesn't exist.");
        return false;
    }

    m_responsePromise = std::promise<const std::string>();
    auto responseFuture = m_responsePromise.get_future();
    responseDispatcher->sendDirective(topic, action, payload);

    std::future_status status = responseFuture.wait_for(m_waitDuration);
    if (status == std::future_status::ready) {
        response = responseFuture.get();
        return true;
    } else {
        m_logger->log(Level::WARN, TAG, "Failed to get result for topic " + topic + " action " + action);
        return false;
    }
}

void SyncOverAsync::responseAvailable(const std::string& payload) {
    m_responsePromise.set_value(payload);
}

}  // namespace bridge
}  // namespace aasb