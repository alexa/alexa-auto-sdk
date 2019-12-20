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

    //-------------------------------------------------------------------------
    // The string values defined here are arbitrary and used when constructing a
    // car control configuration. When the platform interface is called, these
    // values will be returned. So the application can define any suitable values.
    //-------------------------------------------------------------------------
    private class Mode {
        public static final String AUTO                 = "AUTOMATIC";
        public static final String ECONOMY              = "ECONOMY";
        public static final String MANUAL               = "MANUAL";
        public static final String MAXIMUM              = "MAXIMUM";
    }

    private class VentPosition {
        public static final String BODY                 = "BODY";
        public static final String MIX                  = "MIX";
        public static final String FLOOR                = "FLOOR";
        public static final String WINDSHIELD           = "WINDSHIELD";
    }

    private class Color {
        public static final String WHITE                = "WHITE";
        public static final String RED                  = "RED";
        public static final String ORANGE               = "ORANGE";
        public static final String YELLOW               = "YELLOW";
        public static final String GREEN                = "GREEN";
        public static final String BLUE                 = "BLUE";
        public static final String INDIGO               = "INDIGO";
        public static final String VIOLET               = "VIOLET";
    }

    private class Intensity {
        public static final String LOW                  = "LOW";
        public static final String MEDIUM               = "MEDIUM";
        public static final String HIGH                 = "HIGH";
    }

    /**
     * Initialize data from car control configuration.
     */
    public static void initialize(String json) throws Exception {

        try {
            JSONObject config = parseFileAsJSONObject(json);
            JSONObject carConfig = config.getJSONObject("aace.carControl");
            JSONArray endpoints = carConfig.getJSONArray("endpoints");

            for (int i=0; i<endpoints.length(); i++) {
                JSONObject endpoint = endpoints.getJSONObject(i);
                if (endpoint != null) {
                    String endpointId = endpoint.getString("endpointId");
                    JSONArray capabilities = endpoint.getJSONArray("capabilities");
                    for (int c=0; c<capabilities.length(); c++) {
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
                                        for (int m=0; m<supportedModes.length(); m++) {
                                            modeController.addMode(supportedModes.getJSONObject(m).getString("value"));
                                        }
                                        m_modeControllers.put(genKey(endpointId, controllerId), modeController);
                                    }
                                }
                            } else
                            if (controller.equals("Alexa.RangeController")) {
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
                            } else
                            if (controller.equals("Alexa.ToggleController")) {
                                String controllerId = capability.getString("instance");
                                m_boolControllers.put(genKey(endpointId, controllerId), new BoolController());
                            } else
                            if (controller.equals("Alexa.PowerController")) {
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
     * Generate car control configuration.
     */
    public static EngineConfiguration generateCarControlConfig() {
        // Define common values for speed range controllers
        int SPEED_MIN = 1, SPEED_MEDIUM=5, SPEED_MAX = 10, SPEED_PRECISION = 1;
        // Define common values for temperature range controllers
        int TEMPERATURE_MIN = 60, TEMPERATURE_MEDIUM=76, TEMPERATURE_MAX = 90, TEMPERATURE_PRECISION = 2;

        CarControlConfiguration config = CarControlConfiguration.create();
        //---------------------------------------------------------------------
        // Create a Fan controls for the specified zones. Also add a fan speed
        // range controller defined SPEED_MIN and SPEED_MAX. Add some
        // presets to set the fan speed to preset values. Note that preset
        // values must be exact increments of the SPEED_PRECISION, otherwise
        // it will not work. Lastly, add a power control.
        // Alexa utterances:
        //    Alexa turn on the <zone> fan
        //    Alexa turn off the <zone> fan
        //    Alexa set the <zone> fan low/minimum/medium/high/max
        //    Alexa set the <zone> fan to <value between SPEED_MIN and SPEED_MAX >
        //    Alexa increase/decrease <zone> fan speed
        //---------------------------------------------------------------------
        String[] controlIds = { "all.fan" };
        String[] zones = {  CarControlConfiguration.Zone.ALL };

        for (int i=0; i < controlIds.length; i++) {
            config.createControl(controlIds[i], zones[i])
                .addAssetId(CarControlAssets.Device.FAN)
                .addPowerController(true)
                .addRangeController("speed", true, SPEED_MIN, SPEED_MAX, SPEED_PRECISION, "")
                    .addAssetId(CarControlAssets.Setting.FAN_SPEED)
                    .addPreset(SPEED_MIN)
                        .addAssetId(CarControlAssets.Value.LOW)
                        .addAssetId(CarControlAssets.Value.MINIMUM)
                    .addPreset(SPEED_MEDIUM)
                        .addAssetId(CarControlAssets.Value.MEDIUM)
                    .addPreset(SPEED_MAX)
                        .addAssetId(CarControlAssets.Value.HIGH)
                        .addAssetId(CarControlAssets.Value.MAXIMUM);
            // Update internal data structure
            m_boolControllers.put(genKey(controlIds[i]), new BoolController());
            m_rangeControllers.put(genKey(controlIds[i], "speed"), new RangeController(SPEED_MIN, SPEED_MAX));
        }

        //---------------------------------------------------------------------
        // Create a Heater controls for the specified zones. Add a temperature
        // range controller defined TEMPERATURE_MIN and TEMPERATURE_MAX. Add some
        // presets to set the temperature to preset values. Note that preset
        // values must be exact increments of the TEMPERATURE_PRECISION, otherwise
        // it will not work. Lastly, add a power control.
        // Alexa utterances:
        //    Alexa turn on the <zone> heater
        //    Alexa turn off the <zone> heater
        //    Alexa set the <zone> heater low/minimum/medium/high/max
        //    Alexa set the <zone> heater to <value between SPEED_MIN and SPEED_MAX >
        //    Alexa increase/decrease <zone> heater temperature
        //---------------------------------------------------------------------
        controlIds = new String[] { "all.heater" };
        zones = new String[] { CarControlConfiguration.Zone.ALL };
        for (int i=0; i < controlIds.length; i++) {
            config.createControl(controlIds[i], zones[i])
                .addAssetId(CarControlAssets.Device.HEATER)
                .addAssetId(CarControlAssets.Device.COOLER)
                .addPowerController(true)
                .addRangeController("temperature", true, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, CarControlAssets.Unit.FAHRENHEIT)
                    .addAssetId(CarControlAssets.Setting.TEMPERATURE)
                    .addAssetId(CarControlAssets.Setting.HEAT)
                    .addPreset(TEMPERATURE_MIN)
                        .addAssetId(CarControlAssets.Value.LOW)
                        .addAssetId(CarControlAssets.Value.MINIMUM)
                    .addPreset(TEMPERATURE_MEDIUM)
                        .addAssetId(CarControlAssets.Value.MEDIUM)
                    .addPreset(TEMPERATURE_MAX)
                        .addAssetId(CarControlAssets.Value.HIGH)
                        .addAssetId(CarControlAssets.Value.MAXIMUM);
            // Update internal data structure
            m_boolControllers.put(genKey(controlIds[i]), new BoolController());
            m_rangeControllers.put(genKey(controlIds[i], "temperature"), new RangeController(TEMPERATURE_MIN, TEMPERATURE_MAX));
        }

        //---------------------------------------------------------------------
        // Create light controls for specified zones. Add a power controller.
        // Alexa utterances:
        //    Alexa turn on the <zone> light
        //    Alexa turn off the <zone> light
        //---------------------------------------------------------------------
        controlIds = new String[] { "driver.light" };
        zones = new String[] { CarControlConfiguration.Zone.ALL };
        for (int i=0; i < controlIds.length; i++) {
            config.createControl(controlIds[i], zones[i])
                .addAssetId(CarControlAssets.Device.LIGHT)
                .addPowerController(true);
            // Update internal data structure
            m_boolControllers.put(genKey(controlIds[i]), new BoolController());
        }

        //---------------------------------------------------------------------
        // Create air conditioner controller for specified zones. Add a power
        // controller and a mode controller with user defined mode values.
        // Alexa utterances:
        //    Alexa turn on the air conditioner
        //    Alexa turn on the <zone> air conditioner
        //    Alexa turn off the <zone> air conditioner
        //    Alexa set the <zone> air conditioner to economy/auto/manual
        //    Alexa set the <zone> air conditioner intensity to low/medium/high
        //    Alexa increase/decrease the <zone> air conditioner
        //---------------------------------------------------------------------
        config.createControl("ac", CarControlConfiguration.Zone.ALL)
                .addAssetId(CarControlAssets.Device.AIR_CONDITIONER)
                .addModeController("mode", true, false)
                    .addAssetId(CarControlAssets.Setting.MODE)
                    .addValue(Mode.ECONOMY)
                        .addAssetId(CarControlAssets.Setting.ECONOMY)
                    .addValue(Mode.AUTO)
                        .addAssetId(CarControlAssets.Setting.AUTO)
                    .addValue(Mode.MANUAL)
                        .addAssetId(CarControlAssets.Setting.MANUAL)
                .addModeController("intensity", true, true)
                    .addAssetId(CarControlAssets.Setting.INTENSITY)
                    .addValue(Intensity.LOW)
                        .addAssetId(CarControlAssets.Value.LOW)
                        .addAssetId(CarControlAssets.Value.MINIMUM)
                    .addValue(Intensity.MEDIUM)
                        .addAssetId(CarControlAssets.Value.MEDIUM)
                    .addValue(Intensity.HIGH)
                        .addAssetId(CarControlAssets.Value.HIGH)
                        .addAssetId(CarControlAssets.Value.MAXIMUM)
                .addPowerController(true);
        m_boolControllers.put(genKey("ac"), new BoolController());
        // Add AC mode controller
        ModeController acMode = new ModeController();
        acMode.addMode(Mode.ECONOMY);
        acMode.addMode(Mode.AUTO);
        acMode.addMode(Mode.MANUAL);
        m_modeControllers.put(genKey("ac", "mode"), acMode);
        // Add AC intensity controller
        ModeController acIntensity = new ModeController();
        acIntensity.addMode(Intensity.LOW);
        acIntensity.addMode(Intensity.MEDIUM);
        acIntensity.addMode(Intensity.HIGH);
        m_modeControllers.put(genKey("ac", "intensity"), acIntensity);

        //---------------------------------------------------------------------
        // Create window controller for specified zone. Add a toggle
        // controller
        // Alexa utterances:
        //    Alexa turn on the window defroster
        //    Alexa turn off the window defroster
        //---------------------------------------------------------------------
        config.createControl("all.windshield", CarControlConfiguration.Zone.ALL)
            .addAssetId(CarControlAssets.Device.WINDSHIELD)
            .addAssetId(CarControlAssets.Device.WINDOW)
            .addToggleController("defroster", true)
                .addAssetId(CarControlAssets.Setting.DEFROST)
                .addAssetId(CarControlAssets.Setting.DEFOG);
        m_boolControllers.put(genKey("all.windshield", "defroster"), new BoolController());

        //---------------------------------------------------------------------
        // Create car lights. Add a power controller and mode controller to
        // change the color for ambient light.
        // Alexa utterances:
        //    Alexa turn on the <zone> light
        //    Alexa turn off the <zone> light
        //    Alexa set ambient light to red
        //---------------------------------------------------------------------
        config.createControl("dome.light", CarControlConfiguration.Zone.ALL)
            .addAssetId(CarControlAssets.Device.DOME_LIGHT)
            .addAssetId(CarControlAssets.Device.CABIN_LIGHT)
            .addPowerController(true);
        m_boolControllers.put(genKey("dome.light"), new BoolController());

        config.createControl("reading.light", CarControlConfiguration.Zone.ALL)
            .addAssetId(CarControlAssets.Device.READING_LIGHT)
            .addPowerController(true);
        m_boolControllers.put(genKey("reading.light"), new BoolController());

        config.createControl("ambient.light", CarControlConfiguration.Zone.ALL)
            .addAssetId(CarControlAssets.Device.AMBIENT_LIGHT)
            .addPowerController(true)
            .addModeController("color", true, true)
                .addAssetId(CarControlAssets.Setting.COLOR)
                .addValue(Color.RED)
                    .addAssetId(CarControlAssets.Color.RED)
                .addValue(Color.BLUE)
                    .addAssetId(CarControlAssets.Color.BLUE)
                .addValue(Color.GREEN)
                    .addAssetId(CarControlAssets.Color.GREEN)
                .addValue(Color.WHITE)
                    .addAssetId(CarControlAssets.Color.WHITE)
                .addValue(Color.ORANGE)
                    .addAssetId(CarControlAssets.Color.ORANGE)
                .addValue(Color.YELLOW)
                    .addAssetId(CarControlAssets.Color.YELLOW)
                .addValue(Color.INDIGO)
                    .addAssetId(CarControlAssets.Color.INDIGO)
                .addValue(Color.VIOLET)
                    .addAssetId(CarControlAssets.Color.VIOLET);
        m_boolControllers.put(genKey("ambient.light"), new BoolController());
        ModeController ambientLight = new ModeController();
        ambientLight.addMode(Color.RED);
        ambientLight.addMode(Color.BLUE);
        ambientLight.addMode(Color.GREEN);
        ambientLight.addMode(Color.WHITE);
        ambientLight.addMode(Color.ORANGE);
        ambientLight.addMode(Color.YELLOW);
        ambientLight.addMode(Color.INDIGO);
        ambientLight.addMode(Color.VIOLET);
        m_modeControllers.put(genKey("ambient.light", "color"), ambientLight);
        //---------------------------------------------------------------------
        // Create vent controller. Add a power controller and mode controller to
        // change the vent positions.
        // Alexa utterances:
        //    Alexa turn on the <zone> vent
        //    Alexa turn off the <zone> vent
        //    Alexa set the <zone> vent to floor
        //---------------------------------------------------------------------
        config.createControl("vent", CarControlConfiguration.Zone.ALL)
            .addAssetId(CarControlAssets.Device.VENT)
            .addPowerController(true)
            .addModeController("position", true, true)
                .addAssetId(CarControlAssets.Setting.POSITION)
                .addValue(VentPosition.BODY)
                    .addAssetId(CarControlAssets.Setting.BODY_VENTS)
                .addValue(VentPosition.FLOOR)
                    .addAssetId(CarControlAssets.Setting.FLOOR_VENTS)
                .addValue(VentPosition.WINDSHIELD)
                    .addAssetId(CarControlAssets.Setting.WINDSHIELD_VENTS)
                .addValue(VentPosition.MIX)
                    .addAssetId(CarControlAssets.Setting.MIX_VENTS);
        m_boolControllers.put(genKey("vent"), new BoolController());
        ModeController ventPosition = new ModeController();
        ventPosition.addMode(VentPosition.BODY);
        ventPosition.addMode(VentPosition.FLOOR);
        ventPosition.addMode(VentPosition.WINDSHIELD);
        ventPosition.addMode(VentPosition.MIX);
        m_modeControllers.put(genKey("vent", "position"), ventPosition);
        //---------------------------------------------------------------------
        // Create a car controller. Add toggle controllers to turn on/off air
        // recirculation, climate sync, and camera.
        // Alexa utterances:
        //    Alexa turn on/off the air recirculation
        //    Alexa turn on/off the air sync
        //    Alexa turn on/off the camera
        //---------------------------------------------------------------------
        config.createControl("car", CarControlConfiguration.Zone.ALL)
                .addAssetId(CarControlAssets.Device.CAR)
                .addToggleController("recirculate", true)
                    .addAssetId(CarControlAssets.Setting.AIR_RECIRCULATION)
                .addToggleController("climate.sync", true)
                    .addAssetId(CarControlAssets.Setting.CLIMATE_SYNC);
        m_boolControllers.put(genKey("car", "recirculate"), new BoolController());
        m_boolControllers.put(genKey("car", "climate.sync"), new BoolController());

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
        return  genKey(endpointId, "");
    }

    /**
     * Generates a key for map lookup
     */
    private static String genKey(String endpointId, String controllerId) {
        return  endpointId + "#" + controllerId;
    }

    /**
     * Opens file path and creates JSON object
     */
    private static JSONObject parseFileAsJSONObject(String filePath) {
        JSONObject obj = null;
        try {
            File file = new File( filePath );
            FileInputStream is = new FileInputStream(file);
            byte[] buffer = new byte[ is.available() ];

            is.read( buffer );
            String json = new String( buffer, "UTF-8" );
            obj = new JSONObject( json );
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
