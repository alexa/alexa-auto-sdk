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

#ifndef SAMPLEAPP_ALEXA_ALEXACLIENTHANDLER_H
#define SAMPLEAPP_ALEXA_ALEXACLIENTHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Alexa/AlexaClient.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaClientHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlexaClientHandler : public aace::alexa::AlexaClient /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AlexaClientHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlexaClientHandler> {
        return std::shared_ptr<AlexaClientHandler>(new AlexaClientHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::alexa::AlexaClient interface

    auto dialogStateChanged(AlexaClient::DialogState state) -> void override;
    auto authStateChanged(AlexaClient::AuthState state, AlexaClient::AuthError error) -> void override;
    auto connectionStatusChanged(AlexaClient::ConnectionStatus status, AlexaClient::ConnectionChangedReason reason)
        -> void override;

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_authStateView{};
    std::weak_ptr<View> m_connectionStatusView{};
    std::weak_ptr<View> m_dialogStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_ALEXACLIENTHANDLER_H
