/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
static const std::string AIR_CONDITIONER = "Alexa.DeviceName.AirConditioner";
static const std::string AMBIENT_LIGHT = "Alexa.DeviceName.AmbientLight";
static const std::string CABIN_LIGHT = "Alexa.DeviceName.CabinLight";
static const std::string CAR = "Alexa.DeviceName.Car";
static const std::string CLIMATE_CONTROL = "Alexa.DeviceName.ClimateControl";
static const std::string COOLER = "Alexa.DeviceName.Cooler";
static const std::string DOME_LIGHT = "Alexa.DeviceName.DomeLight";
static const std::string FAN = "Alexa.DeviceName.Fan";
static const std::string HEATER = "Alexa.DeviceName.Heater";
static const std::string LIGHT = "Alexa.DeviceName.Light";
static const std::string READING_LIGHT = "Alexa.DeviceName.ReadingLight";
static const std::string TRUNK_LIGHT = "Alexa.DeviceName.TrunkLight";
static const std::string VENT = "Alexa.DeviceName.Vent";
static const std::string WINDOW = "Alexa.DeviceName.Window";
static const std::string WINDSHIELD = "Alexa.DeviceName.Windshield";

}  // namespace device

namespace location {
static const std::string ALL = "Alexa.Location.All";
static const std::string DRIVER = "Alexa.Location.Driver";
static const std::string DRIVER_ROW = "Alexa.Location.DriverRow";
static const std::string FIRST_ROW = "Alexa.Location.FirstRow";
static const std::string FOURTH_ROW = "Alexa.Location.FourthRow";
static const std::string FRONT = "Alexa.Location.Front";
static const std::string PASSENGER = "Alexa.Location.Passenger";
static const std::string PASSENGER_ROW = "Alexa.Location.PassengerRow";
static const std::string REAR = "Alexa.Location.Rear";
static const std::string REAR_DRIVER = "Alexa.Location.RearDriver";
static const std::string REAR_PASSENGER = "Alexa.Location.RearPassenger";
static const std::string SECOND_ROW = "Alexa.Location.SecondRow";
static const std::string THIRD_ROW = "Alexa.Location.ThirdRow";
}  // namespace location

namespace setting {
static const std::string AIR_RECIRCULATION = "Alexa.Setting.AirRecirculation";
static const std::string AUTO = "Alexa.Setting.Auto";
static const std::string BODY_VENTS = "Alexa.Setting.BodyVents";
static const std::string CLIMATE_SYNC = "Alexa.Setting.ClimateSync";
static const std::string COLOR = "Alexa.Setting.Color";
static const std::string DEFOG = "Alexa.Setting.Defog";
static const std::string DEFROST = "Alexa.Setting.Defrost";
static const std::string ECONOMY = "Alexa.Setting.Economy";
static const std::string FAN_SPEED = "Alexa.Setting.FanSpeed";
static const std::string FLOOR_VENTS = "Alexa.Setting.FloorVents";
static const std::string HEAT = "Alexa.Setting.Heat";
static const std::string INTENSITY = "Alexa.Setting.Intensity";
static const std::string MANUAL = "Alexa.Setting.Manual";
static const std::string MIX_VENTS = "Alexa.Setting.MixVents";
static const std::string MODE = "Alexa.Setting.Mode";
static const std::string POSITION = "Alexa.Setting.Position";
static const std::string TEMPERATURE = "Alexa.Setting.Temperature";
static const std::string WINDSHIELD_VENTS = "Alexa.Setting.WindshieldVents";
}  // namespace setting

namespace unit {
static const std::string CELSIUS = "Alexa.Unit.Temperature.Celsius";
static const std::string FAHRENHEIT = "Alexa.Unit.Temperature.Fahrenheit";
}  // namespace unit

namespace color {
static const std::string BLUE = "Alexa.Value.Blue";
static const std::string GREEN = "Alexa.Value.Green";
static const std::string INDIGO = "Alexa.Value.Indigo";
static const std::string ORANGE = "Alexa.Value.Orange";
static const std::string RED = "Alexa.Value.Red";
static const std::string VIOLET = "Alexa.Value.Violet";
static const std::string WHITE = "Alexa.Value.White";
static const std::string YELLOW = "Alexa.Value.Yellow";
}  // namespace color

namespace value {
static const std::string HIGH = "Alexa.Value.High";
static const std::string LOW = "Alexa.Value.Low";
static const std::string MAXIMUM = "Alexa.Value.Maximum";
static const std::string MEDIUM = "Alexa.Value.Medium";
static const std::string MINIMUM = "Alexa.Value.Minimum";
}  // namespace value

}  // namespace alexa
}  // namespace assets
}  // namespace carControl
}  // namespace aace

#endif  // AACE_CAR_CONTROL_CAR_CONTROL_ASSETS_H
