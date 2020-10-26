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

package com.amazon.sampleapp.impl.CarControl;

import com.amazon.aace.carControl.CarControlAssets;
import com.amazon.aace.carControl.CarControlConfiguration;
import com.amazon.aace.core.config.EngineConfiguration;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.util.Hashtable;

public class CarControlDataProvider {
    /**
     * Initialize data from car control configuration.
     */
    public static void initialize(String json) throws Exception {
        try {
            JSONObject config = parseFileAsJSONObject(json);
            JSONObject carConfig = config.getJSONObject("aace.carControl");
            JSONArray endpoints = carConfig.getJSONArray("endpoints");

            for (int i = 0; i < endpoints.length(); i++) {
                JSONObject endpoint = endpoints.getJSONObject(i);
                if (endpoint != null) {
                    String endpointId = endpoint.getString("endpointId");
                    JSONArray capabilities = endpoint.getJSONArray("capabilities");
                    for (int c = 0; c < capabilities.length(); c++) {
                        JSONObject capability = capabilities.getJSONObject(c);
                        if (capability != null) {
                            String controller = capability.getString("interface");
                            if (controller.equals("Alexa.ModeController")) {
                                String controllerId = capability.getString("instance");
                                JSONObject configuration = capability.getJSONObject("configuration");
                                if (configuration != null) {
                                    JSONArray supportedModes = configuration.getJSONArray("supportedModes");
                                    if (supportedModes != null && supportedModes.length() > 0) {
                                        ModeController modeController = new ModeController();
                                        for (int m = 0; m < supportedModes.length(); m++) {
                                            modeController.addMode(supportedModes.getJSONObject(m).getString("value"));
                                        }
                                        m_modeControllers.put(genKey(endpointId, controllerId), modeController);
                                    }
                                }
                            } else if (controller.equals("Alexa.RangeController")) {
                                String controllerId = capability.getString("instance");
                                JSONObject configuration = capability.getJSONObject("configuration");
                                if (configuration != null) {
                                    JSONObject supportedRange = configuration.getJSONObject("supportedRange");
                                    if (supportedRange != null && supportedRange.length() > 0) {
                                        double min = supportedRange.getDouble("minimumValue");
                                        double max = supportedRange.getDouble("maximumValue");
                                        RangeController rangeController = new RangeController(min, max);
                                        m_rangeControllers.put(genKey(endpointId, controllerId), rangeController);
                                    }
                                }
                            } else if (controller.equals("Alexa.ToggleController")) {
                                String controllerId = capability.getString("instance");
                                m_boolControllers.put(genKey(endpointId, controllerId), new BoolController());
                            } else if (controller.equals("Alexa.PowerController")) {
                                m_boolControllers.put(genKey(endpointId), new BoolController());
                            }
                        }
                    }
                }
            }
        } catch (JSONException e) {
            throw e;
        }
    }

    /**
     * Build an example car control configuration for the Auto SDK Engine.
     * See modules/car-control/assets/assets-1P.json for all the friendly names for each asset
     */
    public static EngineConfiguration generateCarControlConfig() {
        CarControlConfiguration config = CarControlConfiguration.create();
        // clang-format off

        //--------------------------------------------------------------------------
        // Important note: See the car control module README at
        // platforms/android/modules/car-control/README.md for the suggested
        // modeling of additional endpoints not shown in this example. The
        // sample configuration in the README document is the source of truth
        // for how endpoints should be configured.
        //--------------------------------------------------------------------------

        //---------------------------------------------------------------------
        // Define the zones of the vehicle.
        // Ensure the endpoint IDs added to the zones are also defined.
        //---------------------------------------------------------------------
        config.createZone("zone.default")
            .addAssetId(CarControlAssets.Location.ALL)
            .addMembers(new String[] {
                "default.fan",
                "default.heater",
                "default.ac",
                "default.vent",
                "default.reading.light"});
        config.createZone("zone.rear")
            .addAssetId(CarControlAssets.Location.REAR)
            .addMembers(new String[] {"rear.windshield"});
        config.createZone("zone.front")
            .addAssetId(CarControlAssets.Location.FRONT)
            .addMembers(new String[] {"front.light"});
        config.createZone("zone.driver")
            .addAssetId(CarControlAssets.Location.DRIVER)
            .addAssetId(CarControlAssets.Location.FRONT_LEFT)
            .addMembers(new String[] {
                "driver.fan",
                "driver.heater",
                "driver.seat",
                "driver.light",
                "driver.window"});
        config.createZone("zone.passenger")
            .addAssetId(CarControlAssets.Location.PASSENGER)
            .addAssetId(CarControlAssets.Location.FRONT_RIGHT)
            .addMembers(new String[] {
                "passenger.fan",
                "passenger.heater",
                "passenger.seat",
                "passenger.light"});
        config.createZone("zone.secondRow")
            .addAssetId(CarControlAssets.Location.SECOND_ROW)
            .addMembers(new String[] {"secondRow.heater", "secondRow.light"});

        // Since "zone.default" is set to default, utterances matching endpoints
        // in this zone take precedence
        config.setDefaultZone("zone.all");

        //---------------------------------------------------------------------
        // Create "fan" endpoints for various zones.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the fan"
        //    "Alexa, turn [on|off] the [driver|passenger] fan"
        //    "Alexa, set the fan speed to [low|minimum|medium|high|max]"
        //    "Alexa, set the fan to <value between 1 and 10>"
        //    "Alexa, turn up the blower"
        //    "Alexa, [increase|decrease] the fan speed by 3"
        //---------------------------------------------------------------------
        config.createEndpoint("default.fan")
            .addAssetId(CarControlAssets.Device.FAN)
            .addPowerController(false)
            .addRangeController("speed", false, 1, 10, 1, "")
                .addAssetId(CarControlAssets.Setting.FAN_SPEED)
                .addAssetId(CarControlAssets.Setting.SPEED)
                .addPreset(1)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(5)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(10)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("default.fan"), new BoolController());
        m_rangeControllers.put(genKey("default.fan", "speed"), new RangeController(1, 10));

        config.createEndpoint("driver.fan")
            .addAssetId(CarControlAssets.Device.FAN)
            .addPowerController(false)
            .addRangeController("speed", false, 1, 10, 1, "")
                .addAssetId(CarControlAssets.Setting.FAN_SPEED)
                .addAssetId(CarControlAssets.Setting.SPEED)
                .addPreset(1)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(5)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(10)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("driver.fan"), new BoolController());
        m_rangeControllers.put(genKey("driver.fan", "speed"), new RangeController(1, 10));

        config.createEndpoint("passenger.fan")
            .addAssetId(CarControlAssets.Device.FAN)
            .addPowerController(false)
            .addRangeController("speed", false, 1, 10, 1, "")
                .addAssetId(CarControlAssets.Setting.FAN_SPEED)
                .addAssetId(CarControlAssets.Setting.SPEED)
                .addPreset(1)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(5)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(10)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("passenger.fan"), new BoolController());
        m_rangeControllers.put(genKey("passenger.fan", "speed"), new RangeController(1, 10));

        //---------------------------------------------------------------------
        // Create a "heater" endpoint for various zones.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the heater"
        //    "Alexa, turn [on|off] the [driver|passenger|second row] heater"
        //    "Alexa, set the temperature to [low|minimum|medium|high|max]"
        //    "Alexa, set the temperature to <value between 60 and 90>"
        //    "Alexa, [increase/decrease] the temperature"
        //    "Alexa, increase the temperature by 4"
        //---------------------------------------------------------------------
        config.createEndpoint("default.heater")
            .addAssetId(CarControlAssets.Device.HEATER)
            .addAssetId(CarControlAssets.Device.COOLER)
            .addPowerController(false)
            .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
                .addAssetId(CarControlAssets.Setting.TEMPERATURE)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addPreset(60)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(75)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(90)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("default.heater"), new BoolController());
        m_rangeControllers.put(genKey("default.heater", "temperature"), new RangeController(60, 90));

        config.createEndpoint("driver.heater")
            .addAssetId(CarControlAssets.Device.HEATER)
            .addAssetId(CarControlAssets.Device.COOLER)
            .addPowerController(false)
            .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
                .addAssetId(CarControlAssets.Setting.TEMPERATURE)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addPreset(60)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(75)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(90)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("driver.heater"), new BoolController());
        m_rangeControllers.put(genKey("driver.heater", "temperature"), new RangeController(60, 90));

        config.createEndpoint("passenger.heater")
            .addAssetId(CarControlAssets.Device.HEATER)
            .addAssetId(CarControlAssets.Device.COOLER)
            .addPowerController(false)
            .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
                .addAssetId(CarControlAssets.Setting.TEMPERATURE)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addPreset(60)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(75)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(90)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("passenger.heater"), new BoolController());
        m_rangeControllers.put(genKey("passenger.heater", "temperature"), new RangeController(60, 90));

        config.createEndpoint("secondRow.heater")
            .addAssetId(CarControlAssets.Device.HEATER)
            .addAssetId(CarControlAssets.Device.COOLER)
            .addPowerController(false)
            .addRangeController("temperature", false, 60, 90, 1, CarControlAssets.Unit.FAHRENHEIT)
                .addAssetId(CarControlAssets.Setting.TEMPERATURE)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addPreset(60)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(75)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(90)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("secondRow.heater"), new BoolController());
        m_rangeControllers.put(genKey("secondRow.heater", "temperature"), new RangeController(60, 90));

        //---------------------------------------------------------------------
        // Create a "window" endpoint for the driver zone.
        // This endpoint includes a semantic action mapping for more natural
        // voice targeting.
        //
        // Things to try:
        //    Without semantic action mappings...
        //      "Alexa, set the driver window height to full"
        //    With semantic action mappings...
        //      "Alexa, [close|raise] the driver window"
        //---------------------------------------------------------------------
        config.createEndpoint("driver.window")
            .addAssetId(CarControlAssets.Device.WINDOW)
            .addRangeController("height", false, 0, 10, 1, "")
                .addAssetId(CarControlAssets.Setting.HEIGHT)
                .addPreset(10)
                    .addAssetId(CarControlAssets.Value.FULL)
                .addPreset(5)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                    .addAssetId(CarControlAssets.Value.HALF)
                .addActionSetRange(new String[] {
                    CarControlConfiguration.Action.CLOSE, CarControlConfiguration.Action.RAISE}, 10)
                .addActionSetRange(new String[] {
                    CarControlConfiguration.Action.OPEN, CarControlConfiguration.Action.LOWER}, 0);

        // Update internal data structure (not relevant to CarControlConfiguration)
        m_rangeControllers.put(genKey("driver.window", "height"), new RangeController(0, 10));

        //---------------------------------------------------------------------
        // Create generic "light" endpoints for various zones.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the [driver|passenger|front|second row] light"
        //---------------------------------------------------------------------
        config.createEndpoint("driver.light")
            .addAssetId(CarControlAssets.Device.LIGHT)
            .addAssetId(CarControlAssets.Device.DOME_LIGHT)
            .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
            .addPowerController(false);
        config.createEndpoint("passenger.light")
            .addAssetId(CarControlAssets.Device.LIGHT)
            .addAssetId(CarControlAssets.Device.DOME_LIGHT)
            .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
            .addPowerController(false);
        config.createEndpoint("front.light")
            .addAssetId(CarControlAssets.Device.LIGHT)
            .addAssetId(CarControlAssets.Device.DOME_LIGHT)
            .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
            .addPowerController(false);
        config.createEndpoint("secondRow.light")
            .addAssetId(CarControlAssets.Device.LIGHT)
            .addAssetId(CarControlAssets.Device.DOME_LIGHT)
            .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
            .addPowerController(false);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("driver.light"), new BoolController());
        m_boolControllers.put(genKey("passenger.light"), new BoolController());
        m_boolControllers.put(genKey("front.light"), new BoolController());
        m_boolControllers.put(genKey("secondRow.light"), new BoolController());

        //---------------------------------------------------------------------
        // Create various additional endpoints for specialized lights
        //
        // Things to try:
        //    "Alexa, turn [on|off] the [dome|cabin|reading] light"
        //    "Alexa, set the ambient light to blue"
        //---------------------------------------------------------------------
        config.createEndpoint("default.reading.light")
            .addAssetId(CarControlAssets.Device.READING_LIGHT)
            .addPowerController(false);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("default.reading.light"), new BoolController());

        config.createEndpoint("ambient.light")
            .addAssetId(CarControlAssets.Device.AMBIENT_LIGHT)
            .addPowerController(false)
            .addModeController("color", false, false)
                .addAssetId(CarControlAssets.Setting.COLOR)
                .addAssetId(CarControlAssets.Setting.MODE)
                .addValue("RED")
                    .addAssetId(CarControlAssets.Color.RED)
                .addValue("BLUE")
                    .addAssetId(CarControlAssets.Color.BLUE)
                .addValue("GREEN")
                    .addAssetId(CarControlAssets.Color.GREEN)
                .addValue("WHITE")
                    .addAssetId(CarControlAssets.Color.WHITE)
                .addValue("ORANGE")
                    .addAssetId(CarControlAssets.Color.ORANGE)
                .addValue("YELLOW")
                    .addAssetId(CarControlAssets.Color.YELLOW)
                .addValue("INDIGO")
                    .addAssetId(CarControlAssets.Color.INDIGO)
                .addValue("VIOLET")
                    .addAssetId(CarControlAssets.Color.VIOLET);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("ambient.light"), new BoolController());
        ModeController ambientLight = new ModeController();
            ambientLight.addMode("RED");
            ambientLight.addMode("BLUE");
            ambientLight.addMode("GREEN");
            ambientLight.addMode("WHITE");
            ambientLight.addMode("ORANGE");
            ambientLight.addMode("YELLOW");
            ambientLight.addMode("INDIGO");
            ambientLight.addMode("VIOLET");
        m_modeControllers.put(genKey("ambient.light", "color"), ambientLight);

        //---------------------------------------------------------------------
        // Create an "air conditioner" endpoint.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the [air conditioner|AC]"
        //    "Alexa, set the AC mode to [economy|auto|manual]"
        //    "Alexa, set the AC intensity to [min|low|medium|high|max]"
        //    "Alexa, [increase|decrease] the AC"
        //---------------------------------------------------------------------
        config.createEndpoint("default.ac")
            .addAssetId(CarControlAssets.Device.AIR_CONDITIONER)
            .addPowerController(false)
            .addModeController("mode", false, false)
                .addAssetId(CarControlAssets.Setting.MODE)
                .addValue("ECONOMY")
                    .addAssetId(CarControlAssets.Setting.ECONOMY)
                .addValue("AUTOMATIC")
                    .addAssetId(CarControlAssets.Setting.AUTO)
                .addValue("MANUAL")
                    .addAssetId(CarControlAssets.Setting.MANUAL)
            .addModeController("intensity", false, true)
                .addAssetId(CarControlAssets.Setting.INTENSITY)
                .addValue("LOW")
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addValue("MEDIUM")
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addValue("HIGH")
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("default.ac"), new BoolController());
        ModeController acMode = new ModeController();
            acMode.addMode("ECONOMY");
            acMode.addMode("AUTOMATIC");
            acMode.addMode("MANUAL");
        m_modeControllers.put(genKey("default.ac", "mode"), acMode);
        ModeController acIntensity = new ModeController();
            acIntensity.addMode("LOW");
            acIntensity.addMode("MEDIUM");
            acIntensity.addMode("HIGH");
        m_modeControllers.put(genKey("default.ac", "intensity"), acIntensity);

        //---------------------------------------------------------------------
        // Create a "windshield" endpoint for the rear zone.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the rear windshield defroster"
        //---------------------------------------------------------------------
        config.createEndpoint("rear.windshield")
            .addAssetId(CarControlAssets.Device.WINDOW)
            .addAssetId(CarControlAssets.Device.WINDSHIELD)
            .addToggleController("defroster", false)
                .addAssetId(CarControlAssets.Setting.DEFROST)
                .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("rear.windshield", "defroster"), new BoolController());

        //---------------------------------------------------------------------
        // Create a "vent" endpoint.
        // This endpoint includes a semantic action mapping for more natural
        // voice targeting.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the vent"
        //    "Alexa, set the vent position to [floor|body|mix]"
        //---------------------------------------------------------------------
        config.createEndpoint("default.vent")
            .addAssetId(CarControlAssets.Device.VENT)
            .addPowerController(false)
            .addModeController("position", false, false)
                .addAssetId(CarControlAssets.Setting.POSITION)
                .addValue("BODY")
                    .addAssetId(CarControlAssets.Setting.BODY_VENTS)
                .addValue("FLOOR")
                    .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
                .addValue("WINDSHIELD")
                    .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
                .addValue("MIX")
                    .addAssetId(CarControlAssets.Setting.MIX_VENTS);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("default.vent"), new BoolController());
        ModeController ventPosition = new ModeController();
            ventPosition.addMode("BODY");
            ventPosition.addMode("FLOOR");
            ventPosition.addMode("WINDSHIELD");
            ventPosition.addMode("MIX");
        m_modeControllers.put(genKey("default.vent", "position"), ventPosition);
        m_boolControllers.put(genKey("default.vent", "height"), new BoolController());

        //---------------------------------------------------------------------
        // Create "seat" endpoints for driver and passenger zones.
        //
        // Things to try:
        //    "Alexa, turn [on|off] the [driver|passenger] seat heater"
        //    "Alexa, set the [driver|passenger] seat heater intensity to
        //          [low|minimum|medium|high|max]"
        //    "Alexa, set the [driver|passenger] seat heater to
        //          <value between 1 and 3>"
        //    "Alexa, [turn up|increase|decrease] the [driver|passenger] seat
        //          heater"
        //    "Alexa, set the [driver|passenger] seat position to [1|2|3]"
        //---------------------------------------------------------------------
        config.createEndpoint("driver.seat")
            .addAssetId(CarControlAssets.Device.SEAT)
            .addToggleController("heater", false)
                .addAssetId(CarControlAssets.Device.HEATER)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addAssetId(CarControlAssets.Device.SEAT_HEATER)
            .addRangeController("heaterintensity", false, 1, 3, 1, "")
                .addAssetId(CarControlAssets.Device.HEATER)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addAssetId(CarControlAssets.Device.SEAT_HEATER)
                .addPreset(1)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(2)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(3)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM)
            .addToggleController("vent", false)
                .addAssetId(CarControlAssets.Device.VENT)
            .addModeController("position", false, true)
                .addAssetId(CarControlAssets.Setting.STORED_POSITION)
                .addValue("ONE")
                    .addAssetId(CarControlAssets.Value.POSITION_ONE)
                .addValue("TWO")
                    .addAssetId(CarControlAssets.Value.POSITION_TWO)
                .addValue("THREE")
                    .addAssetId(CarControlAssets.Value.POSITION_THREE);
        config.createEndpoint("passenger.seat")
            .addAssetId(CarControlAssets.Device.SEAT)
            .addToggleController("heater", false)
                .addAssetId(CarControlAssets.Device.HEATER)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addAssetId(CarControlAssets.Device.SEAT_HEATER)
            .addRangeController("heaterintensity", false, 1, 3, 1, "")
                .addAssetId(CarControlAssets.Device.HEATER)
                .addAssetId(CarControlAssets.Setting.HEAT)
                .addAssetId(CarControlAssets.Device.SEAT_HEATER)
                .addPreset(1)
                    .addAssetId(CarControlAssets.Value.LOW)
                    .addAssetId(CarControlAssets.Value.MINIMUM)
                .addPreset(2)
                    .addAssetId(CarControlAssets.Value.MEDIUM)
                .addPreset(3)
                    .addAssetId(CarControlAssets.Value.HIGH)
                    .addAssetId(CarControlAssets.Value.MAXIMUM)
            .addToggleController("vent", false)
                .addAssetId(CarControlAssets.Device.VENT)
            .addModeController("position", false, true)
                .addAssetId(CarControlAssets.Setting.STORED_POSITION)
                .addValue("ONE")
                    .addAssetId(CarControlAssets.Value.POSITION_ONE)
                .addValue("TWO")
                    .addAssetId(CarControlAssets.Value.POSITION_TWO)
                .addValue("THREE")
                    .addAssetId(CarControlAssets.Value.POSITION_THREE);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("driver.seat", "heater"), new BoolController());
        m_boolControllers.put(genKey("passenger.seat", "heater"), new BoolController());
        m_rangeControllers.put(genKey("driver.seat", "heaterintensity"), new RangeController(1, 3));
        m_rangeControllers.put(genKey("passenger.seat", "heaterintensity"), new RangeController(1, 3));
        m_boolControllers.put(genKey("driver.seat", "vent"), new BoolController());
        m_boolControllers.put(genKey("passenger.seat", "vent"), new BoolController());
        ModeController positionMode = new ModeController();
            positionMode.addMode("ONE");
            positionMode.addMode("TWO");
            positionMode.addMode("THREE");
        m_modeControllers.put(genKey("driver.seat", "position"), positionMode);
        m_modeControllers.put(genKey("passenger.seat", "position"), positionMode);
        //---------------------------------------------------------------------
        // Create a "car" root endpoint for miscellaneous controls not
        // associated with any other endpoint.
        //
        // Things to try:
        //    "Alexa, turn [on|off] air recirculation"
        //    "Alexa, set air recirculation to [inside|outside|auto]"
        //    "Alexa, turn [on|off] climate sync"
        //---------------------------------------------------------------------
        config.createEndpoint("car")
            .addAssetId(CarControlAssets.Device.CAR)
            .addToggleController("recirculate", false)
                .addAssetId(CarControlAssets.Setting.AIR_RECIRCULATION)
            .addModeController("recirculatemode", false, false)
                .addAssetId(CarControlAssets.Setting.AIR_RECIRCULATION)
                .addValue("INSIDE")
                    .addAssetId(CarControlAssets.Value.INSIDE_AIR)
                .addValue("OUTSIDE")
                    .addAssetId(CarControlAssets.Value.OUTSIDE_AIR)
                .addValue("AUTO")
                    .addAssetId(CarControlAssets.Setting.AUTO)
            .addToggleController("climate.sync", false)
                .addAssetId(CarControlAssets.Setting.CLIMATE_SYNC);
        // Update internal data structure (not relevant to CarControlConfiguration)
        m_boolControllers.put(genKey("car", "recirculate"), new BoolController());
        ModeController recirculateMode = new ModeController();
            recirculateMode.addMode("INSIDE");
            recirculateMode.addMode("OUTSIDE");
            recirculateMode.addMode("AUTO");
        m_modeControllers.put(genKey("car", "recirculatemode"), recirculateMode);
        m_boolControllers.put(genKey("car", "climate.sync"), new BoolController());

        // clang-format on
        return config;
    }

    /**
     * Return Power/Toggle Controller
     */
    public static BoolController getBoolController(String controlId, String controllerId) {
        return m_boolControllers.get(genKey(controlId, controllerId));
    }

    public static BoolController getBoolController(String controlId) {
        return getBoolController(controlId, "");
    }

    /**
     * Return Mode Controller
     */
    public static ModeController getModeController(String controlId, String controllerId) {
        return m_modeControllers.get(genKey(controlId, controllerId));
    }

    /**
     * Return Range Controller
     */
    public static RangeController getRangeController(String controlId, String controllerId) {
        return m_rangeControllers.get(genKey(controlId, controllerId));
    }

    /**
     * Generates a key for map lookup
     */
    private static String genKey(String endpointId) {
        return genKey(endpointId, "");
    }

    /**
     * Generates a key for map lookup
     */
    private static String genKey(String endpointId, String controllerId) {
        return endpointId + "#" + controllerId;
    }

    /**
     * Opens file path and creates JSON object
     */
    private static JSONObject parseFileAsJSONObject(String filePath) {
        JSONObject obj = null;
        try {
            File file = new File(filePath);
            FileInputStream is = new FileInputStream(file);
            byte[] buffer = new byte[is.available()];

            is.read(buffer);
            String json = new String(buffer, "UTF-8");
            obj = new JSONObject(json);
        } catch (Exception e) {
        }

        return obj;
    }

    // Used to maintain state of Power and Toggle controllers
    private static Hashtable<String, BoolController> m_boolControllers = new Hashtable<>();
    // Used to maintain state of Range Controllers
    private static Hashtable<String, ModeController> m_modeControllers = new Hashtable<>();
    // Used to maintain state of Mode Controllers
    private static Hashtable<String, RangeController> m_rangeControllers = new Hashtable<>();
}
