/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_ALERTSHANDLER_H
#define SAMPLEAPP_ALEXA_ALERTSHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Alexa/Alerts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlertsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlertsHandler : public aace::alexa::Alerts /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AlertsHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlertsHandler> {
        return std::shared_ptr<AlertsHandler>(new AlertsHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::alexa::Alerts interface

    auto alertStateChanged(const std::string& alertToken, Alerts::AlertState state, const std::string& reason)
        -> void override;
    auto alertCreated(const std::string& alertToken, const std::string& detailedInfo) -> void override;
    auto alertDeleted(const std::string& alertToken) -> void override;

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_alertStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_ALERTSHANDLER_H
