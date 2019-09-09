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
#ifndef AASB_BRIDGE_SYNC_OVER_ASYNC_H
#define AASB_BRIDGE_SYNC_OVER_ASYNC_H

#include <memory>
#include <future>
#include <string>
#include <chrono>

#include "ResponseDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace bridge {

/**
 * Helper class to make sync over async calls to AASB clients
 */
class SyncOverAsync {
public:
    SyncOverAsync(
            std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
            std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher,
            std::chrono::microseconds waitDuration);

    bool makeCallAndWaitForResponse(
            const std::string& topic, const std::string& action, const std::string& payload,
            std::string& response);
    void responseAvailable(const std::string& payload);

private:

    // Promise which will be resolved when the response for the msg arrives
    std::promise<const std::string> m_responsePromise;
    // Wait duration for response
    std::chrono::microseconds m_waitDuration;
    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
    // ResponseDispatcher to send status info
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;
};

}  // namespace bridge
}  // namespace aasb

#endif  // AASB_BRIDGE_SYNC_OVER_ASYNC_H