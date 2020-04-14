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

    // See if a car control configuration is specified that will overrride 
    // the generated one
    for(auto config: configs) {
        auto configStream = config->getStream();
        try {
            json j = json::parse(*configStream);

            if (j.find("aace.carControl") != j.end()) {
                carControl = j["aace.carControl"];
            }
        } catch (json::exception &e) {
        }

        // Always reset the config otherwise the Auto SDK Engine will not find it 
        configStream->clear();
        configStream->seekg(0);

        // If found break out of loop
        if (!carControl.empty()) {
            break;
        }
    }
    
    // Build client configuration from configuration file
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

// Build an example car control configuration for the Auto SDK Engine
std::shared_ptr<aace::core::config::EngineConfiguration> CarControlDataProvider::generateCarControlConfig() {

    // Define common values for speed RangeControllers
    int SPEED_MIN = 1, SPEED_MEDIUM=5, SPEED_MAX = 10, SPEED_PRECISION = 1;
    // Define common values for temperature RangeControllers
    int TEMPERATURE_MIN = 60, TEMPERATURE_MEDIUM=76, TEMPERATURE_MAX = 90, TEMPERATURE_PRECISION = 2;

    auto config = CarControlConfiguration::create();

    // Create the example endpoint configuration without using zones, which work 
    // only in hybrid mode with Local Voice Control.  Use generic assets for 
    // friendly names and synonyms.
    // See modules/car-control/assets/assets-1P.json for reference

    config
        //---------------------------------------------------------------------
        // Create a "fan" endpoint, and add a "speed" RangeController using
        // SPEED_MIN, SPEED_MAX, and SPEED_PRECISION constants. Add some named
        // presets to set the fan speed to preset values. Note that preset
        // values must be exact increments of the SPEED_PRECISION, otherwise 
        // it will not work. Lastly, add a PowerController for the fan.
        //
        // Things to try:
        //    Alexa, turn on the fan 
        //    Alexa, turn the fan off
        //    Alexa, set the fan to low|minimum|medium|high|max
        //    Alexa, set the fan to < value between SPEED_MIN and SPEED_MAX > 
        //    Alexa, turn up the blower
        //    Alexa, increase|decrease fan speed by three
        //---------------------------------------------------------------------
        ->createControl("all.fan", zone::ALL)
            .addAssetId(alexa::device::FAN)
            .addPowerController(false)
            .addRangeController("speed", false, SPEED_MIN, SPEED_MAX, SPEED_PRECISION)
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
        // Create a "heater" endpoint. Add a "temperature" RangeController using
        // the TEMPERATURE_MIN, TEMPERATURE_MAX and TEMPERATURE_PRECISION constants. 
        // Add presets to set the temperature to preset values. Note that preset
        // values must be exact increments of the TEMPERATURE_PRECISION, otherwise
        // it will not work. Lastly, add a PowerController.
        //
        // Things to try:
        //    Alexa, turn on the heater 
        //    Alexa, turn the heater off
        //    Alexa, set the temperature to low|minimum|medium|high|max
        //    Alexa, set the temperature to < value between TEMPERATURE_MIN and TEMPERATURE_MAX > 
        //    Alexa, increase/decrease the temperature 
        //    Alexa, increase the temperature by four
        //---------------------------------------------------------------------
        .createControl("all.heater", zone::ALL)
            .addAssetId(alexa::device::HEATER)
            .addAssetId(alexa::device::COOLER)
            .addPowerController(false)
            .addRangeController("temperature", false, TEMPERATURE_MIN, TEMPERATURE_MAX, TEMPERATURE_PRECISION, alexa::unit::FAHRENHEIT)
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
        // Create a generic "light" endpoint. Add a PowerController for it. 
        //
        // Things to try:
        //    Alexa, turn on the light 
        //    Alexa, turn off the light 
        //---------------------------------------------------------------------
        .createControl("all.light", zone::ALL)
            .addAssetId(alexa::device::LIGHT)
            .addPowerController(false)
        //---------------------------------------------------------------------
        // Create an "air conditioner" endpoint. Add a PowerController 
        // and a ModeController, and define mode values.  Make the "intensity"
        // ModeController ordered to enable AdjustMode utterances like "Turn up
        // the A/C".
        //
        // Things to try:
        //    Alexa, turn on the air conditioner 
        //    Alexa, turn the air conditioner off
        //    Alexa, turn off the AC 
        //    Alexa, set the air conditioner to economy/auto/manual
        //    Alexa, set the air conditioner to low/medium/high
        //    Alexa, increase/decrease the air conditioner 
        //---------------------------------------------------------------------
        .createControl("ac", zone::ALL)
            .addAssetId(alexa::device::AIR_CONDITIONER)
            .addModeController("mode", false, false)
                .addAssetId(alexa::setting::MODE)
                .addValue(mode::ECONOMY)
                    .addAssetId(alexa::setting::ECONOMY)
                .addValue(mode::AUTO)
                    .addAssetId(alexa::setting::AUTO)
                .addValue(mode::MANUAL)
                    .addAssetId(alexa::setting::MANUAL)
            .addModeController("intensity", false, true)
                .addAssetId(alexa::setting::INTENSITY)
                .addValue(intensity::LOW)
                    .addAssetId(alexa::value::LOW)
                    .addAssetId(alexa::value::MINIMUM)
                .addValue(intensity::MEDIUM)
                    .addAssetId(alexa::value::MEDIUM)
                .addValue(intensity::HIGH)
                    .addAssetId(alexa::value::HIGH)
                    .addAssetId(alexa::value::MAXIMUM)
            .addPowerController(false)
        //---------------------------------------------------------------------
        // Create a "windshield" endpoint. Add a controller for a defroster. A
        // PowerController would be global to the windshield itself,
        // which could have other controllers (wipers, for example), so a 
        // ToggleController is best here.
        //
        // Things to try:
        //    Alexa, turn on the defroster 
        //    Alexa, turn the defroster off
        //---------------------------------------------------------------------
        .createControl("all.windshield", zone::ALL)
            .addAssetId(alexa::device::WINDSHIELD)
            .addAssetId(alexa::device::WINDOW)
            .addToggleController("defroster", false)
                .addAssetId(alexa::setting::DEFROST)
                .addAssetId(alexa::setting::DEFOG)
        //---------------------------------------------------------------------
        // Model the interior lighting by adding endpoints for the various 
        // controllable lights in the car. Add PowerControllers to each, and a
        // ModeController to change the color of the ambient lighting.
        //
        // Things to try:
        //    Alexa, turn on the light
        //    Alexa, turn off the light
        //    Alexa, set ambient light to red 
        //---------------------------------------------------------------------
        .createControl("dome.light", zone::ALL)
            .addAssetId(alexa::device::DOME_LIGHT)
            .addAssetId(alexa::device::CABIN_LIGHT)
            .addPowerController(false)
        .createControl("reading.light", zone::ALL)
            .addAssetId(alexa::device::READING_LIGHT)
            .addPowerController(false)
        .createControl("ambient.light", zone::ALL)
            .addAssetId(alexa::device::AMBIENT_LIGHT)
            .addPowerController(false)
            .addModeController("color", false, true)
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
        // Create a "vent" endpoint. Add a PowerController. Add a ModeController 
        // to change the vent positions and values for the modes. 
        //
        // Things to try:
        //    Alexa, turn on the vent
        //    Alexa, turn off the vent
        //    Alexa, set the vent to floor|body|mix
        //---------------------------------------------------------------------
        .createControl("vent", zone::ALL)
            .addAssetId(alexa::device::VENT)
            .addPowerController(false)
            .addModeController("position", false, true)
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
        // Create a generic "car" endpoint for miscellaneous controls not 
        // associated with any other endpoint. For example, add ToggleControllers
        // for air recirculation and climate control sync.
        //
        // Things to try:
        //    Alexa, turn on|off recirculation 
        //    Alexa, turn on|off climate control sync 
        //---------------------------------------------------------------------
        .createControl("car", zone::ALL)
            .addAssetId(alexa::device::CAR)
            .addToggleController("recirculate", false)
                .addAssetId(alexa::setting::AIR_RECIRCULATION)
            .addToggleController("climate.sync", false)
                .addAssetId(alexa::setting::CLIMATE_SYNC);

    return config;
}

} // sampleApp::carControl 
} // sampleApp