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
static const std::string AVM_CAMERA = "Alexa.Automotive.DeviceName.AVMCamera";
static const std::string ARMREST = "Alexa.Automotive.DeviceName.Armrest";
static const std::string CABIN_LIGHT = "Alexa.Automotive.DeviceName.CabinLight";
static const std::string CAMERA_360 = "Alexa.Automotive.DeviceName.360Camera";
static const std::string CAR = "Alexa.Automotive.DeviceName.Car";
static const std::string CHARGE_DOOR = "Alexa.Automotive.DeviceName.ChargeDoor";
static const std::string CLIMATE_CONTROL = "Alexa.Automotive.DeviceName.ClimateControl";
static const std::string COOLER = "Alexa.Automotive.DeviceName.Cooler";
static const std::string CUP_HOLDER = "Alexa.Automotive.DeviceName.CupHolder";
static const std::string DISPLAY = "Alexa.Automotive.DeviceName.Display";
static const std::string DISPLAY_SCREEN = "Alexa.Automotive.DeviceName.DisplayScreen";
static const std::string DOME_LIGHT = "Alexa.Automotive.DeviceName.DomeLight";
static const std::string FAN = "Alexa.Automotive.DeviceName.Fan";
static const std::string FOG_LIGHT = "Alexa.Automotive.DeviceName.FogLight";
static const std::string GAS_DOOR = "Alexa.Automotive.DeviceName.GasDoor";
static const std::string HAZARD_LIGHTS = "Alexa.Automotive.DeviceName.HazardLights";
static const std::string HEADLIGHT = "Alexa.Automotive.DeviceName.Headlight";
static const std::string HEATER = "Alexa.Automotive.DeviceName.Heater";
static const std::string HIGH_BEAM = "Alexa.Automotive.DeviceName.HighBeam";
static const std::string HOOD = "Alexa.Automotive.DeviceName.Hood";
static const std::string HUD = "Alexa.Automotive.DeviceName.HUD";
static const std::string INFO_SCREEN = "Alexa.Automotive.DeviceName.InfoScreen";
static const std::string LIGHT = "Alexa.Automotive.DeviceName.Light";
static const std::string MOONROOF = "Alexa.Automotive.DeviceName.Moonroof";
static const std::string PARKING_LIGHTS = "Alexa.Automotive.DeviceName.ParkingLights";
static const std::string READING_LIGHT = "Alexa.Automotive.DeviceName.ReadingLight";
static const std::string SEAT = "Alexa.Automotive.DeviceName.Seat";
static const std::string SEAT_HEATER = "Alexa.Automotive.DeviceName.SeatHeater";
static const std::string STEERING_WHEEL = "Alexa.Automotive.DeviceName.SteeringWheel";
static const std::string SUNROOF = "Alexa.Automotive.DeviceName.Sunroof";
static const std::string SUNSHADE = "Alexa.Automotive.DeviceName.Sunshade";
static const std::string TAILGATE = "Alexa.Automotive.DeviceName.Tailgate";
static const std::string TRUNK = "Alexa.Automotive.DeviceName.Trunk";
static const std::string TRUNK_LIGHT = "Alexa.Automotive.DeviceName.TrunkLight";
static const std::string VENT = "Alexa.Automotive.DeviceName.Vent";
static const std::string WINDOW = "Alexa.Automotive.DeviceName.Window";
static const std::string WINDSHIELD = "Alexa.Automotive.DeviceName.Windshield";
static const std::string WINDSHIELD_WIPERS = "Alexa.Automotive.DeviceName.WindshieldWipers";
}  // namespace device

namespace location {
static const std::string ALL = "Alexa.Automotive.Location.All";
static const std::string DRIVER = "Alexa.Automotive.Location.Driver";
static const std::string DRIVER_ROW = "Alexa.Automotive.Location.DriverRow";
static const std::string FIRST_ROW = "Alexa.Automotive.Location.FirstRow";
static const std::string FOURTH_ROW = "Alexa.Automotive.Location.FourthRow";
static const std::string FRONT = "Alexa.Automotive.Location.Front";
static const std::string FRONT_LEFT = "Alexa.Automotive.Location.FrontLeft";
static const std::string FRONT_RIGHT = "Alexa.Automotive.Location.FrontRight";
static const std::string LEFT = "Alexa.Automotive.Location.Left";
static const std::string PASSENGER = "Alexa.Automotive.Location.Passenger";
static const std::string PASSENGER_ROW = "Alexa.Automotive.Location.PassengerRow";
static const std::string REAR = "Alexa.Automotive.Location.Rear";
static const std::string REAR_LEFT = "Alexa.Automotive.Location.RearLeft";
static const std::string REAR_RIGHT = "Alexa.Automotive.Location.RearRight";
static const std::string REAR_DRIVER = "Alexa.Automotive.Location.RearDriver";
static const std::string REAR_PASSENGER = "Alexa.Automotive.Location.RearPassenger";
static const std::string RIGHT = "Alexa.Automotive.Location.Right";
static const std::string SECOND_ROW = "Alexa.Automotive.Location.SecondRow";
static const std::string THIRD_ROW = "Alexa.Automotive.Location.ThirdRow";
}  // namespace location

namespace setting {
static const std::string AIR_RECIRCULATION = "Alexa.Automotive.Setting.AirRecirculation";
static const std::string AUTO = "Alexa.Automotive.Setting.Auto";
static const std::string AUTO_BRAKE_HOLD = "Alexa.Automotive.Setting.AutoBrakeHold";
static const std::string BLUETOOTH_PAGE = "Alexa.Automotive.Setting.BluetoothPage";
static const std::string BODY_VENTS = "Alexa.Automotive.Setting.BodyVents";
static const std::string BRIGHTNESS = "Alexa.Automotive.Setting.Brightness";
static const std::string CHILD_DOOR_LOCK = "Alexa.Automotive.Setting.ChildDoorLock";
static const std::string CLIMATE_SYNC = "Alexa.Automotive.Setting.ClimateSync";
static const std::string COLOR = "Alexa.Automotive.Setting.Color";
static const std::string COOLING = "Alexa.Automotive.Setting.Cooling";
static const std::string CRUISE_CONTROL = "Alexa.Automotive.Setting.CruiseControl";
static const std::string DEFOG = "Alexa.Automotive.Setting.Defog";
static const std::string DEFROST = "Alexa.Automotive.Setting.Defrost";
static const std::string DIRECTION = "Alexa.Automotive.Setting.Direction";
static const std::string DRIVE_MODE = "Alexa.Automotive.Setting.DriveMode";
static const std::string ECONOMY = "Alexa.Automotive.Setting.Economy";
static const std::string FAN_SPEED = "Alexa.Automotive.Setting.FanSpeed";
static const std::string FLOOR_VENTS = "Alexa.Automotive.Setting.FloorVents";
static const std::string HEAT = "Alexa.Automotive.Setting.Heat";
static const std::string HEIGHT = "Alexa.Automotive.Setting.Height";
static const std::string HILL_ASSIST = "Alexa.Automotive.Setting.HillAssist";
static const std::string INFO_PAGE = "Alexa.Automotive.Setting.InfoPage";
static const std::string INTENSITY = "Alexa.Automotive.Setting.Intensity";
static const std::string LANE_ASSIST = "Alexa.Automotive.Setting.LaneAssist";
static const std::string MANUAL = "Alexa.Automotive.Setting.Manual";
static const std::string MAX_AIR_CONDITIONING = "Alexa.Automotive.Setting.MaxAirConditioning";
static const std::string MIX_VENTS = "Alexa.Automotive.Setting.MixVents";
static const std::string MODE = "Alexa.Automotive.Setting.Mode";
static const std::string NAVIGATION_PAGE = "Alexa.Automotive.Setting.NavigationPage";
static const std::string OPENNESS = "Alexa.Automotive.Setting.Openness";
static const std::string PARKING_BRAKE = "Alexa.Automotive.Setting.ParkingBrake";
static const std::string POSITION = "Alexa.Automotive.Setting.Position";
static const std::string PRE_COLLISION_BRAKE = "Alexa.Automotive.Setting.PreCollisionBrake";
static const std::string RADIO_PAGE = "Alexa.Automotive.Setting.RadioPage";
static const std::string SATELLITE_RADIO_PAGE = "Alexa.Automotive.Setting.SatelliteRadioPage";
static const std::string SERVICE_PAGE = "Alexa.Automotive.Setting.ServicePage";
static const std::string SETTINGS_PAGE = "Alexa.Automotive.Setting.SettingsPage";
static const std::string SPEED = "Alexa.Automotive.Setting.Speed";
static const std::string STORED_POSITION = "Alexa.Automotive.Setting.StoredPosition";
static const std::string SWAY_WARNING = "Alexa.Automotive.Setting.SwayWarning";
static const std::string TEMPERATURE = "Alexa.Automotive.Setting.Temperature";
static const std::string TOWING_MODE = "Alexa.Automotive.Setting.TowingMode";
static const std::string TRACTION_CONTROL = "Alexa.Automotive.Setting.TractionControl";
static const std::string WINDOW_LOCK = "Alexa.Automotive.Setting.WindowLock";
static const std::string WINDSHIELD_VENTS = "Alexa.Automotive.Setting.WindshieldVents";
}  // namespace setting

namespace unit {
static const std::string CELSIUS = "Alexa.Unit.Temperature.Celsius";
static const std::string FAHRENHEIT = "Alexa.Unit.Temperature.Fahrenheit";
static const std::string PERCENT = "Alexa.Unit.Percent";
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
static const std::string ALERTS_PAGE = "Alexa.Automotive.Value.AlertsPage";
static const std::string BLUETOOTH_PAGE = "Alexa.Automotive.Value.BluetoothPage";
static const std::string CLOSED = "Alexa.Automotive.Value.Closed";
static const std::string COMFORT = "Alexa.Automotive.Value.Comfort";
static const std::string CONTACTS_PAGE = "Alexa.Automotive.Value.ContactsPage";
static const std::string CONTROLS_PAGE = "Alexa.Automotive.Value.ControlsPage";
static const std::string DRIVER_VIEW = "Alexa.Automotive.Value.DriverView";
static const std::string DYNAMIC_COORDINATOR_PAGE = "Alexa.Automotive.Value.DynamicCoordinatorPage";
static const std::string FOUR_WD_AUTO = "Alexa.Automotive.Value.4WDAuto";
static const std::string FOUR_WD_HIGH = "Alexa.Automotive.Value.4WDHigh";
static const std::string FOUR_WD_LOW = "Alexa.Automotive.Value.4WDLow";
static const std::string FRONT_VIEW = "Alexa.Automotive.Value.FrontView";
static const std::string FULL = "Alexa.Automotive.Value.Full";
static const std::string GPS_PAGE = "Alexa.Automotive.Value.GPSPage";
static const std::string HALF = "Alexa.Automotive.Value.Half";
static const std::string HIGH = "Alexa.Automotive.Value.High";
static const std::string HOME_PAGE = "Alexa.Automotive.Value.HomePage";
static const std::string INFORMATION_PAGE = "Alexa.Automotive.Value.InformationPage";
static const std::string INSIDE_AIR = "Alexa.Automotive.Value.InsideAir";
static const std::string INTERMITTENT = "Alexa.Automotive.Value.Intermittent";
static const std::string LOW = "Alexa.Automotive.Value.Low";
static const std::string MAXIMUM = "Alexa.Automotive.Value.Maximum";
static const std::string MEDIUM = "Alexa.Automotive.Value.Medium";
static const std::string MINIMUM = "Alexa.Automotive.Value.Minimum";
static const std::string MULTIMEDIA_PAGE = "Alexa.Automotive.Value.MultimediaPage";
static const std::string MUSIC_PAGE = "Alexa.Automotive.Value.MusicPage";
static const std::string NAVIGATION_PAGE = "Alexa.Automotive.Value.NavigationPage";
static const std::string NORMAL = "Alexa.Automotive.Value.Normal";
static const std::string NOTIFICATIONS_PAGE = "Alexa.Automotive.Value.NotificationsPage";
static const std::string OPEN = "Alexa.Automotive.Value.Open";
static const std::string OPTIMAL = "Alexa.Automotive.Value.Optimal";
static const std::string OUTSIDE_AIR = "Alexa.Automotive.Value.OutsideAir";
static const std::string PASSENGER_VIEW = "Alexa.Automotive.Value.PassengerView";
static const std::string PERFORMANCE_PAGE = "Alexa.Automotive.Value.PerformancePage";
static const std::string POSITION_ONE = "Alexa.Automotive.Value.PositionOne";
static const std::string POSITION_THREE = "Alexa.Automotive.Value.PositionThree";
static const std::string POSITION_TWO = "Alexa.Automotive.Value.PositionTwo";
static const std::string RADIO_PAGE = "Alexa.Automotive.Value.RadioPage";
static const std::string REAR_VIEW = "Alexa.Automotive.Value.RearView";
static const std::string SATELLITE_RADIO_PAGE = "Alexa.Automotive.Value.SatelliteRadioPage";
static const std::string SERVICE_PAGE = "Alexa.Automotive.Value.ServicePage";
static const std::string SETTINGS_PAGE = "Alexa.Automotive.Value.SettingsPage";
static const std::string SPORT = "Alexa.Automotive.Value.Sport";
static const std::string SPORT_PLUS = "Alexa.Automotive.Value.SportPlus";
static const std::string TELEPHONE_PAGE = "Alexa.Automotive.Value.TelephonePage";
static const std::string VEHICLE_STATUS_PAGE = "Alexa.Automotive.Value.VehicleStatusPage";
}  // namespace value

}  // namespace alexa
}  // namespace assets
}  // namespace carControl
}  // namespace aace

#endif  // AACE_CAR_CONTROL_CAR_CONTROL_ASSETS_H
