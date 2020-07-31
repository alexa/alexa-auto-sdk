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

#ifndef SAMPLEAPP_CARCONTROL_CARCONTROLHANDLER_H
#define SAMPLEAPP_CARCONTROL_CARCONTROLHANDLER_H

#include <unordered_map>

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/CarControl/CarControl.h>
#include <AACE/CarControl/CarControlConfiguration.h>

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

class CarControlHandler : public aace::carControl::CarControl /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    CarControlHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<CarControlHandler> {
        return std::shared_ptr<CarControlHandler>(new CarControlHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    enum class ConfigType { LVC, CAR };

    // Car Control Platform Interface
public:
    // PowerController
    bool turnPowerControllerOn(const std::string& endpointId) override;
    bool turnPowerControllerOff(const std::string& endpointId) override;
    bool isPowerControllerOn(const std::string& endpointId, bool& isOn) override;

    // ToggleController
    bool turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId) override;
    bool turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId) override;
    bool isToggleControllerOn(const std::string& endpointId, const std::string& controllerId, bool& isOn) override;

    // RangeController
    bool setRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double value) override;
    bool adjustRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double delta)
        override;
    bool getRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double& value)
        override;

    // ModeController
    bool setModeControllerValue(
        const std::string& endpointId,
        const std::string& controllerId,
        const std::string& value) override;
    bool adjustModeControllerValue(const std::string& endpointId, const std::string& controllerId, int delta) override;
    bool getModeControllerValue(const std::string& endpointId, const std::string& controllerId, std::string& value)
        override;

public:
    static bool checkConfiguration(const std::vector<json>& jsons, ConfigType type);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto showMessage(std::string) -> void;
};

}  // namespace carControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_CARCONTROL_CARCONTROLHANDLER_H
