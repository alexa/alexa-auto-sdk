/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_DONOTDISTURBHANDLER_H
#define SAMPLEAPP_ALEXA_DONOTDISTURBHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DoNotDisturbHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DoNotDisturbHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    DoNotDisturbHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<DoNotDisturbHandler> {
        return std::shared_ptr<DoNotDisturbHandler>(new DoNotDisturbHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the SetDoNotDisturb message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSetDoNotDisturbMessage(const std::string& message);

    /**
     * Handle setting of DoNotDisturb state.
     *
     * @param [in] doNotDisturb setting state
     */
    void setDoNotDisturb(const bool doNotDisturb);

    /**
     * Notify the Engine of a platform request to set the DoNotDisturb state
     *
     * @param [in] doNotDisturb setting state
     */
    void doNotDisturbChanged(const bool doNotDisturb);

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_doNotDisturbStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_DONOTDISTURBHANDLER_H
