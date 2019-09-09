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

#include <sstream>
#include <fstream>

#include <SampleApp/CarControl/CarControlDataProvider.h>
#include <AACE/CarControl/CarControlAssets.h>

using namespace aace::carControl::config;
using namespace aace::carControl::assets;

namespace sampleApp {
namespace carControl {

// Initialize static data structures
std::unordered_map<std::string, BoolController> CarControlDataProvider::m_boolControllers;
std::unordered_map<std::string, ModeController> CarControlDataProvider::m_modeControllers;
std::unordered_map<std::string, RangeController> CarControlDataProvider::m_rangeControllers;

BoolController& CarControlDataProvider::getBoolController(const std::string& controlId, const std::string& controllerId) {
    return m_boolControllers[genKey(controlId, controllerId)];
} 

ModeController& CarControlDataProvider::getModeController(const std::string& controlId, const std::string& controllerId) {
    return m_modeControllers[genKey(controlId, controllerId)];

}
    
RangeController& CarControlDataProvider::getRangeController(const std::string& controlId, const std::string& controllerId) {
    return m_rangeControllers[genKey(controlId, controllerId)];
}

void CarControlDataProvider::initialize(std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configs) {

    json carControl;

    // Search car control configuration
    for(auto config: configs) {
        auto configStream = config->getStream();
        try {
            json j = json::parse(*configStream);

            if (j.find("aace.carControl") != j.end()) {
                carControl = j["aace.carControl"];
            }
        } catch (json::exception &e) {
        }

        // Always reset the config otherwise Auto SDK engine will not find it 
        configStream->clear();
        configStream->seekg(0);

        // If found break out of loop
        if (!carControl.empty()) {
            break;
        }
    }
    
    // Generate initial values from file configuration
    try {

        if (carControl.empty()) {
            return;
        }

        json endpoints = carControl["endpoints"];

        for (auto& endpoint: endpoints) {
            if (!endpoint.empty()) {
                std::string endpointId = endpoint["endpointId"];
                json capabilities = endpoint["capabilities"];
                for (auto& capability : capabilities) {
                    if (!capability.empty()) {
                        std::string controller = capability["interface"];
                        if (controller == "Alexa.ModeController") {
                            std::string controllerId = capability["instance"];
                            json configuration = capability["configuration"];
                            if (!configuration.empty()) {
                                json supportedModes = configuration["supportedModes"];
                                if (!supportedModes.empty()) {
                                    ModeController modeController;
                                    for (auto& mode: supportedModes) {
                                        modeController.addMode(mode["value"]);
                                    }
                                    m_modeControllers[genKey(endpointId, controllerId)] = modeController;
                                }
                            }
                        } else
                        if (controller == "Alexa.RangeController") {
                            std::string controllerId = capability["instance"];
                            json configuration = capability["configuration"];
                            if (!configuration.empty()) {
                                json supportedRange = configuration["supportedRange"];
                                if (!supportedRange.empty()) {
                                    double min = supportedRange["minimumValue"];
                                    double max = supportedRange["maximumValue"];
                                    m_rangeControllers[genKey(endpointId, controllerId)] = RangeController(min, max);
                                }
                            }
                        } else
                        if (controller == "Alexa.ToggleController") {
                            std::string controllerId = capability["instance"];
                            m_boolControllers[genKey(endpointId, controllerId)] = BoolController();
                        } else
                        if (controller == "Alexa.PowerController") {
                            m_boolControllers[genKey(endpointId)] = BoolController();
                        }
                    }
                }
            }
        }
    } catch (json::exception& e) {
    }
}

std::string CarControlDataProvider::genKey(std::string endpointId, std::string controllerId) {
    return  endpointId + "#" + controllerId;
}

std::shared_ptr<aace::core::config::EngineConfiguration> CarControlDataProvider::generateCarControlConfig() {

    // Define common values for speed range controllers
    int SPEED_MIN = 1, SPEED_MEDIUM=5, SPEED_MAX = 10, SPEED_PRECISION = 1;
    // Define common values for temperature range controllers
    int TEMPERATURE_MIN = 60, TEMPERATURE_MEDIUM=76, TEMPERATURE_MAX = 90, TEMPERATURE_PRECISION = 2;

    auto config = CarControlConfiguration::create();
    config
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
        ->createControl("driver.fan", zone::DRIVER)
            .addAssetId(alexa::device::FAN)
            .addPowerController(true)
            .addRangeController("speed", true, SPEED_MIN, SPEED_MAX, SPEED_PRECISION)
                .addAssetId(alexa::setting::FAN_SPEED)
                .addPreset(SPEED_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(SPEED_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(SPEED_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("passenger.fan", zone::PASSENGER)
            .addAssetId(alexa::device::FAN)
            .addPowerController(true)
            .addRangeController("speed", true, SPEED_MIN, SPEED_MAX, SPEED_PRECISION)
                .addAssetId(alexa::setting::FAN_SPEED)
                .addPreset(SPEED_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(SPEED_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(SPEED_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("rear.fan", zone::REAR)
            .addAssetId(alexa::device::FAN)
            .addPowerController(true)
            .addRangeController("speed", true, SPEED_MIN, SPEED_MAX, SPEED_PRECISION)
                .addAssetId(alexa::setting::FAN_SPEED)
                .addPreset(SPEED_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(SPEED_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(SPEED_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("front.fan", zone::FRONT)
            .addAssetId(alexa::device::FAN)
            .addPowerController(true)
            .addRangeController("speed", true, SPEED_MIN, SPEED_MAX, SPEED_PRECISION)
                .addAssetId(alexa::setting::FAN_SPEED)
                .addPreset(SPEED_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(SPEED_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(SPEED_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("all.fan", zone::ALL)
            .addAssetId(alexa::device::FAN)
            .addPowerController(true)
            .addRangeController("speed", true, SPEED_MIN, SPEED_MAX, SPEED_PRECISION)
                .addAssetId(alexa::setting::FAN_SPEED)
                .addPreset(SPEED_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(SPEED_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(SPEED_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
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
        .createControl("driver.heater", zone::DRIVER)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::device::COOLER)
            .addPowerController(true)
            .addRangeController("temperature", true, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, alexa::unit::FAHRENHEIT)
                .addAssetId(alexa::setting::TEMPERATURE)
                .addAssetId(alexa::setting::HEAT)
                .addPreset(TEMPERATURE_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(TEMPERATURE_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(TEMPERATURE_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("passenger.heater", zone::PASSENGER)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::device::COOLER)
            .addPowerController(true)
            .addRangeController("temperature", true, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, alexa::unit::FAHRENHEIT)
                .addAssetId(alexa::setting::TEMPERATURE)
                .addAssetId(alexa::setting::HEAT)
                .addPreset(TEMPERATURE_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(TEMPERATURE_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(TEMPERATURE_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("all.heater", zone::ALL)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::device::COOLER)
            .addPowerController(true)
            .addRangeController("temperature", true, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, alexa::unit::FAHRENHEIT)
                .addAssetId(alexa::setting::TEMPERATURE)
                .addAssetId(alexa::setting::HEAT)
                .addPreset(TEMPERATURE_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(TEMPERATURE_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(TEMPERATURE_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("secondRow.heater", zone::SECOND_ROW)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::device::COOLER)
            .addPowerController(true)
            .addRangeController("temperature", true, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, alexa::unit::FAHRENHEIT)
                .addAssetId(alexa::setting::TEMPERATURE)
                .addAssetId(alexa::setting::HEAT)
                .addPreset(TEMPERATURE_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(TEMPERATURE_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(TEMPERATURE_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        .createControl("thirdRow.heater", zone::THIRD_ROW)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::device::COOLER)
            .addPowerController(true)
            .addRangeController("temperature", true, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, alexa::unit::FAHRENHEIT)
                .addAssetId(alexa::setting::TEMPERATURE)
                .addAssetId(alexa::setting::HEAT)
                .addPreset(TEMPERATURE_MIN)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addPreset(TEMPERATURE_MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addPreset(TEMPERATURE_MAX)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
        //---------------------------------------------------------------------
        // Create light controls for specified zones. Add a power controller. 
        // Alexa utterances:
        //    Alexa turn on the <zone> light 
        //    Alexa turn off the <zone> light 
        //---------------------------------------------------------------------
        .createControl("driver.light", zone::DRIVER)
            .addAssetId(alexa::device::LIGHT)
            .addPowerController(true)
        .createControl("passenger.light", zone::PASSENGER)
            .addAssetId(alexa::device::LIGHT)
            .addPowerController(true)
        .createControl("front.light", zone::FRONT)
            .addAssetId(alexa::device::LIGHT)
            .addPowerController(true)
        .createControl("secondRow.light", zone::SECOND_ROW)
            .addAssetId(alexa::device::LIGHT)
            .addPowerController(true)
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
        .createControl("ac", zone::ALL)
            .addAssetId(alexa::device::AIR_CONDITIONER)
            .addModeController("mode", true, false)
                .addAssetId(alexa::setting::MODE)
                .addValue(mode::ECONOMY)
                    .addAssetId(alexa::setting::ECONOMY)
                .addValue(mode::AUTO)
                    .addAssetId(alexa::setting::AUTO)
                .addValue(mode::MANUAL)
                    .addAssetId(alexa::setting::MANUAL)
            .addModeController("intensity", true, true)
                .addAssetId(alexa::setting::INTENSITY)
                .addValue(intensity::LOW)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addValue(intensity::MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addValue(intensity::HIGH)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
            .addPowerController(true)
        //---------------------------------------------------------------------
        // Create window controller for specified zone. Add a toggle 
        // controller 
        // Alexa utterances:
        //    Alexa turn on the window defroster 
        //    Alexa turn off the window defroster 
        //---------------------------------------------------------------------
        .createControl("rear.windshield", zone::REAR)
            .addAssetId(alexa::device::WINDSHIELD)
            .addAssetId(alexa::device::WINDOW)
            .addToggleController("defroster", true)
                .addAssetId(alexa::setting::DEFROST)
                .addAssetId(alexa::setting::DEFOG)
        //---------------------------------------------------------------------
        // Create car lights. Add a power controller and mode controller to
        // change the color for ambient light.
        // Alexa utterances:
        //    Alexa turn on the <zone> light
        //    Alexa turn off the <zone> light
        //    Alexa set ambient light to red 
        //---------------------------------------------------------------------
        .createControl("dome.light", zone::ALL)
            .addAssetId(alexa::device::DOME_LIGHT)
            .addAssetId(alexa::device::CABIN_LIGHT)
            .addPowerController(true)
        .createControl("reading.light", zone::ALL)
            .addAssetId(alexa::device::READING_LIGHT)
            .addPowerController(true)
        .createControl("ambient.light", zone::ALL)
            .addAssetId(alexa::device::AMBIENT_LIGHT)
            .addPowerController(true)
            .addModeController("color", true, true)
                .addAssetId(alexa::setting::COLOR)
                .addValue(color::RED)
                    .addAssetId(alexa::color::RED)
                .addValue(color::BLUE)
                    .addAssetId(alexa::color::BLUE)
                .addValue(color::GREEN)
                    .addAssetId(alexa::color::GREEN)
                .addValue(color::WHITE)
                    .addAssetId(alexa::color::WHITE)
                .addValue(color::ORANGE)
                    .addAssetId(alexa::color::ORANGE)
                .addValue(color::YELLOW)
                    .addAssetId(alexa::color::YELLOW)
                .addValue(color::INDIGO)
                    .addAssetId(alexa::color::INDIGO)
                .addValue(color::VIOLET)
                    .addAssetId(alexa::color::VIOLET)
        //---------------------------------------------------------------------
        // Create vent controller. Add a power controller and mode controller to
        // change the vent positions. 
        // Alexa utterances:
        //    Alexa turn on the <zone> vent
        //    Alexa turn off the <zone> vent
        //    Alexa set the <zone> vent to floor 
        //---------------------------------------------------------------------
        .createControl("vent", zone::ALL)
            .addAssetId(alexa::device::VENT)
            .addPowerController(true)
            .addModeController("position", true, true)
                .addAssetId(alexa::setting::POSITION)
                .addValue(ventPosition::BODY)
                    .addAssetId(alexa::setting::BODY_VENTS)
                .addValue(ventPosition::FLOOR)
                    .addAssetId(alexa::setting::FLOOR_VENTS)
                .addValue(ventPosition::WINDSHIELD)
                    .addAssetId(alexa::setting::WINDSHIELD_VENTS)
                .addValue(ventPosition::MIX)
                    .addAssetId(alexa::setting::MIX_VENTS)
        //---------------------------------------------------------------------
        // Create a car controller. Add toggle controllers to turn on/off air
        // recirculation, climary sync, and generic stabilitrak device.
        // Alexa utterances:
        //    Alexa turn on/off the air recirculation 
        //    Alexa turn on/off the air sync 
        //    Alexa turn on/off the air stabilitrak 
        //---------------------------------------------------------------------
        .createControl("car", zone::ALL)
            .addAssetId(alexa::device::CAR)
            .addToggleController("recirculate", true)
                .addAssetId(alexa::setting::AIR_RECIRCULATION)
            .addToggleController("climate.sync", true)
                .addAssetId(alexa::setting::CLIMATE_SYNC)
            .addToggleController("stabilitrak", true)
                .addAssetId("My.Stabilitrak");

    return config;
}

} // sampleApp::carControl 
} // sampleApp