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

#ifndef SAMPLEAPP_CARCONTROL_CLIMATECONTROLHANDLER_H
#define SAMPLEAPP_CARCONTROL_CLIMATECONTROLHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/CarControl/ClimateControlInterface.h>

namespace sampleApp {
namespace carControl {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ClimateControlHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ClimateControlHandler : public aace::carControl::ClimateControlInterface /* isa PlatformInterface */ {
  private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

  protected:
    ClimateControlHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

  public:
    template <typename... Args> static auto create(Args &&... args) -> std::shared_ptr<ClimateControlHandler> {
        return std::shared_ptr<ClimateControlHandler>(new ClimateControlHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // Climate Control

    bool turnClimateControlOn() override;
    bool turnClimateControlOff() override;
    bool isClimateControlOn() override;

    // Air Conditioning

    bool turnAirConditioningOn() override;
    bool turnAirConditioningOff() override;
    bool isAirConditioningOn() override;

    bool setAirConditioningMode(AirConditioningMode mode) override;
    AirConditioningMode getAirConditioningMode() override;

    // Fan

    bool turnFanOn(FanZone zone) override;
    bool turnFanOff(FanZone zone) override;
    bool isFanOn(FanZone zone) override;

    bool setFanSpeed(FanZone zone, double value) override;
    bool adjustFanSpeed(FanZone zone, double delta) override;
    double getFanSpeed(FanZone zone) override;

    // Temperature

    bool setTemperature(TemperatureZone zone, double value) override;
    bool adjustTemperature(TemperatureZone zone, double delta) override;
    double getTemperature(TemperatureZone zone) override;

  private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string &message) -> void;
    auto setupUI() -> void;

  private:
    bool m_cc;
    bool m_ac;
    bool m_fan;
    double m_fanSpeed = 5.0;
    double m_temperature = 20.0;
    aace::carControl::ClimateControlInterface::AirConditioningMode m_ac_mode = aace::carControl::ClimateControlInterface::AirConditioningMode::MANUAL;
};

} // namespace carControl
} // namespace sampleApp

#endif // SAMPLEAPP_CARCONTROL_CLIMATECONTROLHANDLER_H
