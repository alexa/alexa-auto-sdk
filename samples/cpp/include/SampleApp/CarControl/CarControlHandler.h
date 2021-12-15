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

#ifndef SAMPLEAPP_CARCONTROL_CARCONTROLHANDLER_H
#define SAMPLEAPP_CARCONTROL_CARCONTROLHANDLER_H

#include <unordered_map>

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/CarControl/CarControl.h>
#include <AACE/CarControl/CarControlConfiguration.h>

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/CarControl/CarControl/AdjustControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/AdjustRangeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/AdjustModeControllerValueMessage.h>

#include <AASB/Message/CarControl/CarControl/SetControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetRangeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetModeControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetPowerControllerValueMessage.h>
#include <AASB/Message/CarControl/CarControl/SetToggleControllerValueMessage.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace carControl {

using namespace aace::carControl::config;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CarControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class CarControlHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    CarControlHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<CarControlHandler> {
        return std::shared_ptr<CarControlHandler>(new CarControlHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the SetControllerValue messages received from the Engine.
     *
     * This message is received from the Engine for:
     * @li Power Controller - to power on or power off
     * @li Toggle Contoller - to turn on or off
     * @li Mode Controller - to set the mode value
     * @li Range Controller - to set the range value
     *
     * In each case, the message payload specifies the controller for which the message is intended,
     * the endpointId or a combination of endpointId and instanceId to identify the controller, and the
     * value that needs to be set.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSetControllerValueMessage(const std::string& message);

    /**
     * Handles the AdjustControllerValue messages received from the Engine.
     *
     * This message is received from the Engine for:
     * @li Mode Controller - to adjust the mode value
     * @li Range Controller - to adjust the range value
     *
     * In each case, the message payload specifies the controller for which the message is intended,
     * a combination of endpointId and instanceId to identify the controller, and the delta value by
     * which to adjust the setting.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAdjustControllerValueMessage(const std::string& message);

public:
    static bool checkConfiguration(const std::vector<json>& jsons);

private:
    /**
     * Set the power state to on/off for the controller identified by @c endpointId
     * and publish the reply message indicating the endpoint was successfully turned on/off
     *
     * @param [in] messageId The unique identifier of the message received from the Engine.
     * @param [in] endpointId The unique identifier of the endpoint, obtained from the message payload.
     * @param [in] value Value to indicate the setting. @c true to turn on,  @c false to turn off.
     */
    void setPowerControllerValue(const std::string& messageId, const std::string& endpointId, bool value);

    /**
     * Set the state to on/off for the controller identified by @c endpointId and @c instanceId
     * and publish the reply message indicating the property was successfully turned on/off
     *
     * @param [in] messageId The unique identifier of the message received from the Engine.
     * @param [in] endpointId The unique identifier of the endpoint, obtained from the message payload.
     * @param [in] instanceId The unique identifier of the controller instance, obtained from the message payload.
     * @param [in] value Value to indicate the setting. @c true to turn on,  @c false to turn off.
     */
    void setToggleControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        bool value);

    /**
     * Set the range setting for the controller identified by @c endpointId and @c instanceId
     * and publish the reply message indicating the range for the endpoint property was successfully set.
     *
     * @param [in] messageId The unique identifier of the message received from the Engine.
     * @param [in] endpointId The unique identifier of the endpoint, obtained from the message payload.
     * @param [in] instanceId The unique identifier of the controller instance, obtained from the message payload.
     * @param [in] value The new range setting, obtained from the message payload.
     */
    void setRangeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        double value);

    /**
     * Adjust the range setting for the controller identified by @c endpointId and @c instanceId
     * and publish the reply message indicating the mode for the endpoint property was successfully adjusted.
     *
     * @param [in] messageId The unique identifier of the message received from the Engine.
     * @param [in] endpointId The unique identifier of the endpoint, obtained from the message payload.
     * @param [in] instanceId The unique identifier of the controller instance, obtained from the message payload.
     * @param [in] delta The delta by which to adjust the range setting, obtained from the message payload.
     */
    void adjustRangeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        double delta);

    /**
     * Set the mode for the controller identified by @c endpointId and @c instanceId
     * and publish the reply message indicating the mode for the endpoint property was successfully set.
     *
     * @param [in] messageId The unique identifier of the message received from the Engine.
     * @param [in] endpointId The unique identifier of the endpoint, obtained from the message payload.
     * @param [in] instanceId The unique identifier of the controller instance, obtained from the message payload.
     * @param [in] value The new mode to set, obtained from the message payload.
     */
    void setModeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        const std::string& value);

    /**
     * Adjust the mode for the controller identified by @c endpointId and @c instanceId
     * and publish the reply message indicating the mode for the endpoint property was successfully adjusted.
     *
     * @param [in] messageId The unique identifier of the message received from the Engine.
     * @param [in] endpointId The unique identifier of the endpoint, obtained from the message payload.
     * @param [in] instanceId The unique identifier of the controller instance, obtained from the message payload.
     * @param [in] delta The delta by which to adjust the mode, obtained from the message payload.
     */
    void adjustModeControllerValue(
        const std::string& messageId,
        const std::string& endpointId,
        const std::string& instanceId,
        int delta);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto showMessage(std::string) -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace carControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_CARCONTROL_CARCONTROLHANDLER_H
