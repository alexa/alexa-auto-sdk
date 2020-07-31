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

#ifndef SAMPLEAPP_PHONECONTROL_PHONECONTROLHANDLER_H
#define SAMPLEAPP_PHONECONTROL_PHONECONTROLHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/PhoneCallController/PhoneCallController.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace phoneControl {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PhoneCallControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class PhoneCallControllerHandler : public aace::phoneCallController::PhoneCallController /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    PhoneCallControllerHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<PhoneCallControllerHandler> {
        return std::shared_ptr<PhoneCallControllerHandler>(new PhoneCallControllerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::phoneCallController::PhoneCallController interface

    auto dial(const std::string& payload) -> bool override;
    auto redial(const std::string& payload) -> bool override;
    auto answer(const std::string& payload) -> void override;
    auto stop(const std::string& payload) -> void override;
    auto sendDTMF(const std::string& payload) -> void override;

public:
    /**
     * Helper method to display the current payload data.
     */
    void showPayload();

private:
    void createCall();
    void updateCallId(const std::string& payload);
    void updateCallerId(const std::string& value);
    void updatePayload(const std::string& payload = "");
    std::string getPhoneNumber(const json& payload);
    std::string getCallId(const json& payload);
    std::string callStateToString();

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;

    /// Payload for current call incoming or outgoing
    json m_currentCall;

    /// Last number dialed
    json m_outgoingCall;

    /// Call state
    CallState m_callState;

    /// Call error
    std::string m_callError;
};

}  // namespace phoneControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_PHONECONTROL_PHONECONTROLHANDLER_H
