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

#ifndef SAMPLEAPP_ALEXA_ALERTSHANDLER_H
#define SAMPLEAPP_ALEXA_ALERTSHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/Alerts/AlertState.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlertsHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlertsHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AlertsHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlertsHandler> {
        return std::shared_ptr<AlertsHandler>(new AlertsHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the AlertStateChanged messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAlertStateChangedMessage(const std::string& message);

    /**
     * Handles the AlertCreated messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAlertCreatedMessage(const std::string& message);

    /**
     * Handles the AlertDeleted messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAlertDeletedMessage(const std::string& message);

    /**
     * Notifies the Engine of a platform request to stop any active alert, such as when a user presses a physical 'stop' button.
     */
    void localStop();

    /**
     * Notifies the Engine of a platform request to clear the user's
     * pending alerts from local storage. This may be useful for a scenario in which a user's pending alerts should not go
     * off after he logs out of the application. This does not clear the user's pending alerts from the Alexa cloud since it
     * does not support a local 'Remove all alerts' feature.
     */
    void removeAllAlerts();

    /**
     * Provides the implementation for an alert state change
     *
     * @param [in] alertToken The opaque token that uniquely identifies the alert
     * @param [in] state The new alert state
     * @param [in] reason The reason for the state change
     */
    void alertStateChanged(
        const std::string& alertToken,
        aasb::message::alexa::alerts::AlertState state,
        const std::string& reason);

    /**
     * Provides the implementation for an alert created, with detailed alert info.
     *
     * @param [in] alertToken The AVS token of the alert.
     * @param [in] detailedInfo The alert info payload :
     * {
     *      "time" : <String>
     *      "type" : <String>
     *      "label" : <String>
     * }
     * time The time string ( Scheduled Time ISO_8601 ).
     * type The type of the alert ( ALERT, REMINDER, TIMER ).
     * label The label of the TIMER, description for REMINDER, or empty string for ALARM.
     */
    void alertCreated(const std::string& alertToken, const std::string& detailedInfo);

    /**
     * Provides the implementation for an alert deleted, with the alertToken.
     *
     * @param [in] alertToken The AVS token of the alert.
     */
    void alertDeleted(const std::string& alertToken);

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_alertStateView{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_ALERTSHANDLER_H
