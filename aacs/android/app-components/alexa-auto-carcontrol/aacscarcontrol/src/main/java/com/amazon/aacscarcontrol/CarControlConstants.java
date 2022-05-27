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

package com.amazon.aacscarcontrol;

public class CarControlConstants {
    public static final String AACS_CARCONTROL = "AACSCarControl";
    public static final String AACS_CC_ConfigMapping = "CarControlEndpointMapping.json";
    public static final String ENDPOINT_ID = "endpointId";
    public static final String CAPABILITIES = "capabilities";
    public static final String CONFIGURATION = "configuration";
    public static final String SUPPORTED_MODE = "supportedModes";
    public static final String INTERFACE = "interface";
    public static final String INSTANCE = "instance";
    public static final String DATA_TYPE = "dataType";
    public static final String AREA_ID = "areaId";
    public static final int AREA_GLOBAL = 0x0;
    public static final String PROPERTY_ID = "propertyId";
    public static final String POWER_CONTROLLER = "PowerController";
    public static final String RANGE_CONTROLLER = "RangeController";
    public static final String TOGGLE_CONTROLLER = "ToggleController";
    public static final String MODE_CONTROLLER = "ModeController";
    public static final String POWER = "POWER";
    public static final String TOGGLE = "TOGGLE";
    public static final String RANGE = "RANGE";
    public static final String MODE = "MODE";
    public static final String CONTROLLER_TYPE = "controllerType";
    public static final String CONTROLLER_ID = "controllerId";
    public static final String VALUE = "value";
    public static final String MODE_SETTINGS = "settings";
    public static final String DELTA = "delta";
    public static final String TURNON = "turnOn";
    public static final int CELSIUS_UNIT = 48;
    public static final int FAHRENHEIT_UNIT = 49;

    public enum DataType { INT, BOOLEAN, FLOAT, UNKNOWN }
    ;
}
