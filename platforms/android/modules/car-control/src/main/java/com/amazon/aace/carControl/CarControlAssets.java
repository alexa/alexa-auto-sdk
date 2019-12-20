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

package com.amazon.aace.carControl;

public class CarControlAssets {
    private CarControlAssets() {
    }

    public class Device {
        public static final String AIR_CONDITIONER      = "Alexa.DeviceName.AirConditioner";
        public static final String AMBIENT_LIGHT        = "Alexa.DeviceName.AmbientLight";
        public static final String CABIN_LIGHT          = "Alexa.DeviceName.CabinLight";
        public static final String CAR                  = "Alexa.DeviceName.Car";
        public static final String CLIMATE_CONTROL      = "Alexa.DeviceName.ClimateControl";
        public static final String COOLER               = "Alexa.DeviceName.Cooler";
        public static final String DOME_LIGHT           = "Alexa.DeviceName.DomeLight";
        public static final String FAN                  = "Alexa.DeviceName.Fan";
        public static final String HEATER               = "Alexa.DeviceName.Heater";
        public static final String LIGHT                = "Alexa.DeviceName.Light";
        public static final String READING_LIGHT        = "Alexa.DeviceName.ReadingLight";
        public static final String TRUNK_LIGHT          = "Alexa.DeviceName.TrunkLight";
        public static final String VENT                 = "Alexa.DeviceName.Vent";
        public static final String WINDOW               = "Alexa.DeviceName.Window";
        public static final String WINDSHIELD           = "Alexa.DeviceName.Windshield";
    };

    public class Location {
        public static final String ALL                  = "Alexa.Location.All";
        public static final String DRIVER               = "Alexa.Location.Driver";
        public static final String DRIVER_ROW           = "Alexa.Location.DriverRow";
        public static final String FIRST_ROW            = "Alexa.Location.FirstRow";
        public static final String FOURTH_ROW           = "Alexa.Location.FourthRow";
        public static final String FRONT                = "Alexa.Location.Front";
        public static final String PASSENGER            = "Alexa.Location.Passenger";
        public static final String PASSENGER_ROW        = "Alexa.Location.PassengerRow";
        public static final String REAR                 = "Alexa.Location.Rear";
        public static final String REAR_DRIVER          = "Alexa.Location.RearDriver";
        public static final String REAR_PASSENGER       = "Alexa.Location.RearPassenger";
        public static final String SECOND_ROW           = "Alexa.Location.SecondRow";
        public static final String THIRD_ROW            = "Alexa.Location.ThirdRow";
    };

    public class Setting {
        public static final String AIR_RECIRCULATION    = "Alexa.Setting.AirRecirculation";
        public static final String AUTO                 = "Alexa.Setting.Auto";
        public static final String BODY_VENTS           = "Alexa.Setting.BodyVents";
        public static final String CLIMATE_SYNC         = "Alexa.Setting.ClimateSync";
        public static final String COLOR                = "Alexa.Setting.Color";
        public static final String DEFOG                = "Alexa.Setting.Defog";
        public static final String DEFROST              = "Alexa.Setting.Defrost";
        public static final String ECONOMY              = "Alexa.Setting.Economy";
        public static final String FAN_SPEED            = "Alexa.Setting.FanSpeed";
        public static final String FLOOR_VENTS          = "Alexa.Setting.FloorVents";
        public static final String HEAT                 = "Alexa.Setting.Heat";
        public static final String INTENSITY            = "Alexa.Setting.Intensity";
        public static final String MANUAL               = "Alexa.Setting.Manual";
        public static final String MIX_VENTS            = "Alexa.Setting.MixVents";
        public static final String MODE                 = "Alexa.Setting.Mode";
        public static final String POSITION             = "Alexa.Setting.Position";
        public static final String TEMPERATURE          = "Alexa.Setting.Temperature";
        public static final String WINDSHIELD_VENTS     = "Alexa.Setting.WindshieldVents";
    };

    public class Unit {
        public static final String CELSIUS              = "Alexa.Unit.Temperature.Celsius";
        public static final String FAHRENHEIT           = "Alexa.Unit.Temperature.Fahrenheit";
    };

    public class Color {
        public static final String WHITE                = "Alexa.Value.White";
        public static final String RED                  = "Alexa.Value.Red";
        public static final String ORANGE               = "Alexa.Value.Orange";
        public static final String YELLOW               = "Alexa.Value.Yellow";
        public static final String GREEN                = "Alexa.Value.Green";
        public static final String BLUE                 = "Alexa.Value.Blue";
        public static final String INDIGO               = "Alexa.Value.Indigo";
        public static final String VIOLET               = "Alexa.Value.Violet";
    };

    public class Value {
        public static final String HIGH                 = "Alexa.Value.High";
        public static final String LOW                  = "Alexa.Value.Low";
        public static final String MAXIMUM              = "Alexa.Value.Maximum";
        public static final String MEDIUM               = "Alexa.Value.Medium";
        public static final String MINIMUM              = "Alexa.Value.Minimum";
    };
}