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

package com.amazon.aace.carControl;

public class CarControlAssets {
    private CarControlAssets() {
    }

    public class Device {
        public static final String AIR_CONDITIONER      = "Alexa.Automotive.DeviceName.AirConditioner";
        public static final String AMBIENT_LIGHT        = "Alexa.Automotive.DeviceName.AmbientLight";
        public static final String CABIN_LIGHT          = "Alexa.Automotive.DeviceName.CabinLight";
        public static final String CAR                  = "Alexa.Automotive.DeviceName.Car";
        public static final String CLIMATE_CONTROL      = "Alexa.Automotive.DeviceName.ClimateControl";
        public static final String COOLER               = "Alexa.Automotive.DeviceName.Cooler";
        public static final String DOME_LIGHT           = "Alexa.Automotive.DeviceName.DomeLight";
        public static final String DRIVER_SEAT          = "Alexa.Automotive.DeviceName.DriverSeat";
        public static final String FAN                  = "Alexa.Automotive.DeviceName.Fan";
        public static final String HEATER               = "Alexa.Automotive.DeviceName.Heater";
        public static final String LEFT_SEAT            = "Alexa.Automotive.DeviceName.LeftSeat";
        public static final String LIGHT                = "Alexa.Automotive.DeviceName.Light";
        public static final String PASSENGER_SEAT       = "Alexa.Automotive.DeviceName.PassengerSeat";
        public static final String READING_LIGHT        = "Alexa.Automotive.DeviceName.ReadingLight";
        public static final String RIGHT_SEAT           = "Alexa.Automotive.DeviceName.RightSeat";
        public static final String SEAT_HEATER          = "Alexa.Automotive.DeviceName.SeatHeater";
        public static final String TRUNK_LIGHT          = "Alexa.Automotive.DeviceName.TrunkLight";
        public static final String VENT                 = "Alexa.Automotive.DeviceName.Vent";
        public static final String WINDOW               = "Alexa.Automotive.DeviceName.Window";
        public static final String WINDSHIELD           = "Alexa.Automotive.DeviceName.Windshield";
    };

    public class Location {
        public static final String ALL                  = "Alexa.Automotive.Location.All";
        public static final String DRIVER               = "Alexa.Automotive.Location.Driver";
        public static final String DRIVER_ROW           = "Alexa.Automotive.Location.DriverRow";
        public static final String FIRST_ROW            = "Alexa.Automotive.Location.FirstRow";
        public static final String FOURTH_ROW           = "Alexa.Automotive.Location.FourthRow";
        public static final String FRONT                = "Alexa.Automotive.Location.Front";
        public static final String LEFT                 = "Alexa.Automotive.Location.Left";
        public static final String PASSENGER            = "Alexa.Automotive.Location.Passenger";
        public static final String PASSENGER_ROW        = "Alexa.Automotive.Location.PassengerRow";
        public static final String REAR                 = "Alexa.Automotive.Location.Rear";
        public static final String REAR_DRIVER          = "Alexa.Automotive.Location.RearDriver";
        public static final String REAR_PASSENGER       = "Alexa.Automotive.Location.RearPassenger";
        public static final String RIGHT                = "Alexa.Automotive.Location.Right";
        public static final String SECOND_ROW           = "Alexa.Automotive.Location.SecondRow";
        public static final String THIRD_ROW            = "Alexa.Automotive.Location.ThirdRow";
    };

    public class Setting {
        public static final String AIR_RECIRCULATION    = "Alexa.Automotive.Setting.AirRecirculation";
        public static final String AUTO                 = "Alexa.Automotive.Setting.Auto";
        public static final String BODY_VENTS           = "Alexa.Automotive.Setting.BodyVents";
        public static final String CLIMATE_SYNC         = "Alexa.Automotive.Setting.ClimateSync";
        public static final String COLOR                = "Alexa.Automotive.Setting.Color";
        public static final String DEFOG                = "Alexa.Automotive.Setting.Defog";
        public static final String DEFROST              = "Alexa.Automotive.Setting.Defrost";
        public static final String ECONOMY              = "Alexa.Automotive.Setting.Economy";
        public static final String FAN_SPEED            = "Alexa.Automotive.Setting.FanSpeed";
        public static final String FLOOR_VENTS          = "Alexa.Automotive.Setting.FloorVents";
        public static final String HEAT                 = "Alexa.Automotive.Setting.Heat";
        public static final String INTENSITY            = "Alexa.Automotive.Setting.Intensity";
        public static final String MANUAL               = "Alexa.Automotive.Setting.Manual";
        public static final String MIX_VENTS            = "Alexa.Automotive.Setting.MixVents";
        public static final String MODE                 = "Alexa.Automotive.Setting.Mode";
        public static final String POSITION             = "Alexa.Automotive.Setting.Position";
        public static final String TEMPERATURE          = "Alexa.Automotive.Setting.Temperature";
        public static final String WINDSHIELD_VENTS     = "Alexa.Automotive.Setting.WindshieldVents";
    };

    public class Unit {
        public static final String CELSIUS              = "Alexa.Unit.Temperature.Celsius";
        public static final String FAHRENHEIT           = "Alexa.Unit.Temperature.Fahrenheit";
    };

    public class Color {
        public static final String WHITE                = "Alexa.Automotive.Value.White";
        public static final String RED                  = "Alexa.Automotive.Value.Red";
        public static final String ORANGE               = "Alexa.Automotive.Value.Orange";
        public static final String YELLOW               = "Alexa.Automotive.Value.Yellow";
        public static final String GREEN                = "Alexa.Automotive.Value.Green";
        public static final String BLUE                 = "Alexa.Automotive.Value.Blue";
        public static final String INDIGO               = "Alexa.Automotive.Value.Indigo";
        public static final String VIOLET               = "Alexa.Automotive.Value.Violet";
    };

    public class Value {
        public static final String HIGH                 = "Alexa.Automotive.Value.High";
        public static final String LOW                  = "Alexa.Automotive.Value.Low";
        public static final String MAXIMUM              = "Alexa.Automotive.Value.Maximum";
        public static final String MEDIUM               = "Alexa.Automotive.Value.Medium";
        public static final String MINIMUM              = "Alexa.Automotive.Value.Minimum";
    };
}