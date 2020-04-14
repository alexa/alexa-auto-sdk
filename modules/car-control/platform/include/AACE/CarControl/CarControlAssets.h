/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_CAR_CONTROL_CAR_CONTROL_ASSETS_H
#define AACE_CAR_CONTROL_CAR_CONTROL_ASSETS_H

/** @file */

#include <string>

namespace aace {
namespace carControl {
namespace assets {
namespace alexa {

namespace device {
static const std::string AIR_CONDITIONER = "Alexa.Automotive.DeviceName.AirConditioner";
static const std::string AMBIENT_LIGHT = "Alexa.Automotive.DeviceName.AmbientLight";
static const std::string CABIN_LIGHT = "Alexa.Automotive.DeviceName.CabinLight";
static const std::string CAR = "Alexa.Automotive.DeviceName.Car";
static const std::string CLIMATE_CONTROL = "Alexa.Automotive.DeviceName.ClimateControl";
static const std::string COOLER = "Alexa.Automotive.DeviceName.Cooler";
static const std::string DOME_LIGHT = "Alexa.Automotive.DeviceName.DomeLight";
static const std::string DRIVER_SEAT = "Alexa.Automotive.DeviceName.DriverSeat";
static const std::string FAN = "Alexa.Automotive.DeviceName.Fan";
static const std::string HEATER = "Alexa.Automotive.DeviceName.Heater";
static const std::string LEFT_SEAT = "Alexa.Automotive.DeviceName.LeftSeat";
static const std::string LIGHT = "Alexa.Automotive.DeviceName.Light";
static const std::string PASSENGER_SEAT = "Alexa.Automotive.DeviceName.PassengerSeat";
static const std::string READING_LIGHT = "Alexa.Automotive.DeviceName.ReadingLight";
static const std::string RIGHT_SEAT = "Alexa.Automotive.DeviceName.RightSeat";
static const std::string SEAT_HEATER = "Alexa.Automotive.DeviceName.SeatHeater";
static const std::string TRUNK_LIGHT = "Alexa.Automotive.DeviceName.TrunkLight";
static const std::string VENT = "Alexa.Automotive.DeviceName.Vent";
static const std::string WINDOW = "Alexa.Automotive.DeviceName.Window";
static const std::string WINDSHIELD = "Alexa.Automotive.DeviceName.Windshield";

}  // namespace device

namespace location {
static const std::string ALL = "Alexa.Automotive.Location.All";
static const std::string DRIVER = "Alexa.Automotive.Location.Driver";
static const std::string DRIVER_ROW = "Alexa.Automotive.Location.DriverRow";
static const std::string FIRST_ROW = "Alexa.Automotive.Location.FirstRow";
static const std::string FOURTH_ROW = "Alexa.Automotive.Location.FourthRow";
static const std::string FRONT = "Alexa.Automotive.Location.Front";
static const std::string LEFT = "Alexa.Automotive.Location.Left";
static const std::string PASSENGER = "Alexa.Automotive.Location.Passenger";
static const std::string PASSENGER_ROW = "Alexa.Automotive.Location.PassengerRow";
static const std::string REAR = "Alexa.Automotive.Location.Rear";
static const std::string REAR_DRIVER = "Alexa.Automotive.Location.RearDriver";
static const std::string REAR_PASSENGER = "Alexa.Automotive.Location.RearPassenger";
static const std::string RIGHT = "Alexa.Automotive.Location.Right";
static const std::string SECOND_ROW = "Alexa.Automotive.Location.SecondRow";
static const std::string THIRD_ROW = "Alexa.Automotive.Location.ThirdRow";
}  // namespace location

namespace setting {
static const std::string AIR_RECIRCULATION = "Alexa.Automotive.Setting.AirRecirculation";
static const std::string AUTO = "Alexa.Automotive.Setting.Auto";
static const std::string BODY_VENTS = "Alexa.Automotive.Setting.BodyVents";
static const std::string CLIMATE_SYNC = "Alexa.Automotive.Setting.ClimateSync";
static const std::string COLOR = "Alexa.Automotive.Setting.Color";
static const std::string DEFOG = "Alexa.Automotive.Setting.Defog";
static const std::string DEFROST = "Alexa.Automotive.Setting.Defrost";
static const std::string ECONOMY = "Alexa.Automotive.Setting.Economy";
static const std::string FAN_SPEED = "Alexa.Automotive.Setting.FanSpeed";
static const std::string FLOOR_VENTS = "Alexa.Automotive.Setting.FloorVents";
static const std::string HEAT = "Alexa.Automotive.Setting.Heat";
static const std::string INTENSITY = "Alexa.Automotive.Setting.Intensity";
static const std::string MANUAL = "Alexa.Automotive.Setting.Manual";
static const std::string MIX_VENTS = "Alexa.Automotive.Setting.MixVents";
static const std::string MODE = "Alexa.Automotive.Setting.Mode";
static const std::string POSITION = "Alexa.Automotive.Setting.Position";
static const std::string TEMPERATURE = "Alexa.Automotive.Setting.Temperature";
static const std::string WINDSHIELD_VENTS = "Alexa.Automotive.Setting.WindshieldVents";
}  // namespace setting

namespace unit {
static const std::string CELSIUS = "Alexa.Unit.Temperature.Celsius";
static const std::string FAHRENHEIT = "Alexa.Unit.Temperature.Fahrenheit";
}  // namespace unit

namespace color {
static const std::string BLUE = "Alexa.Automotive.Value.Blue";
static const std::string GREEN = "Alexa.Automotive.Value.Green";
static const std::string INDIGO = "Alexa.Automotive.Value.Indigo";
static const std::string ORANGE = "Alexa.Automotive.Value.Orange";
static const std::string RED = "Alexa.Automotive.Value.Red";
static const std::string VIOLET = "Alexa.Automotive.Value.Violet";
static const std::string WHITE = "Alexa.Automotive.Value.White";
static const std::string YELLOW = "Alexa.Automotive.Value.Yellow";
}  // namespace color

namespace value {
static const std::string HIGH = "Alexa.Automotive.Value.High";
static const std::string LOW = "Alexa.Automotive.Value.Low";
static const std::string MAXIMUM = "Alexa.Automotive.Value.Maximum";
static const std::string MEDIUM = "Alexa.Automotive.Value.Medium";
static const std::string MINIMUM = "Alexa.Automotive.Value.Minimum";
}  // namespace value

}  // namespace alexa
}  // namespace assets
}  // namespace carControl
}  // namespace aace

#endif  // AACE_CAR_CONTROL_CAR_CONTROL_ASSETS_H
