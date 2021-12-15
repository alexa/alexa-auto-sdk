/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_DEVICESETUPHANDLER_H_
#define SAMPLEAPP_ALEXA_DEVICESETUPHANDLER_H_

#include "SampleApp/Activity.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/DeviceSetup/StatusCode.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DeviceSetupHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DeviceSetupHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    DeviceSetupHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<DeviceSetupHandler> {
        return std::shared_ptr<DeviceSetupHandler>(new DeviceSetupHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the SetupCompletedResponseMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSetupCompletedResponseMessage(const std::string& message);

    /**
     * Handles the delivery status of @c setupCompleted.
     *
     * @param [in] statusCode The status that indicates if setup was completed sucessfully or not.
     */
    void setupCompletedResponse(aasb::message::alexa::deviceSetup::StatusCode statusCode);

    /**
     * Notify the Engine (by publishing SetupCompletedMessage) to send an event to inform Alexa when a product
     * has completed setup after an out-of-box experience (OOBE).
     * The response status of the sending the event will be reported through @c setupCompletedResponse.
     *
     * @note Notify only once after completion of OOBE. Subsequent calls may be ignored by the cloud.
     * @note Do not notify if user is in Connectivity Mode, Preview Mode, or if Alexa wake word is disabled.
     * Notifying in such conditions results in undesired user experience.
     */
    void setupCompleted();

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_DEVICESETUPHANDLER_H_
