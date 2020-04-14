/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_COASSISTANT_COASSISTANTHANDLER_H
#define SAMPLEAPP_COASSISTANT_COASSISTANTHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/CoAssistant/CoAssistant.h>

namespace sampleApp {

class ApplicationContext; // forward declare

namespace coassistant {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CoAssistantHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CoAssistantHandler : public aace::coassistant::CoAssistant /* isa PlatformInterface */ {
  private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

  protected:
    CoAssistantHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

  public:
    template <typename... Args> static auto create(Args &&... args) -> std::shared_ptr<CoAssistantHandler> {
        return std::shared_ptr<CoAssistantHandler>(new CoAssistantHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::coassistant::CoAssistant interface

    auto activeAgentChanged(const std::string& agent) -> void override;


  private:
    std::shared_ptr<ApplicationContext> m_applicationContext{};
    bool m_busy{};

    auto log(logger::LoggerHandler::Level level, const std::string &message) -> void;
    auto setupUI() -> void;
};

} // namespace coassistant
} // namespace sampleApp

#endif // SAMPLEAPP_COASSISTANT_COASSISTANTHANDLER_H
