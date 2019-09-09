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

#ifndef SAMPLEAPP_CARCONTROL_CARCONTROL_DATA_PROVIDER_H
#define SAMPLEAPP_CARCONTROL_CARCONTROL_DATA_PROVIDER_H

#include <AACE/CarControl/CarControlConfiguration.h>
#include <AACE/Core/EngineConfiguration.h>

#include <SampleApp/CarControl/BoolController.h>
#include <SampleApp/CarControl/ModeController.h>
#include <SampleApp/CarControl/RangeController.h>

#include <vector>
#include <unordered_map>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace carControl {

namespace mode {
    static const std::string AUTO                 = "AUTOMATIC";
    static const std::string ECONOMY              = "ECONOMY";
    static const std::string MANUAL               = "MANUAL";
    static const std::string MAXIMUM              = "MAXIMUM";
}

namespace ventPosition {
    static const std::string BODY                 = "BODY";
    static const std::string MIX                  = "MIX";
    static const std::string FLOOR                = "FLOOR";
    static const std::string WINDSHIELD           = "WINDSHIELD";
}

namespace color {
    static const std::string WHITE                = "WHITE";
    static const std::string RED                  = "RED";
    static const std::string ORANGE               = "ORANGE";
    static const std::string YELLOW               = "YELLOW";
    static const std::string GREEN                = "GREEN";
    static const std::string BLUE                 = "BLUE";
    static const std::string INDIGO               = "INDIGO";
    static const std::string VIOLET               = "VIOLET";
}

namespace intensity {
    static const std::string LOW                  = "LOW";
    static const std::string MEDIUM               = "MEDIUM";
    static const std::string HIGH                 = "HIGH";
}

class CarControlDataProvider {

public:
    
    /**
     * Initialize data from car control configuration.
     */
    static auto initialize(std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationFiles) -> void;

    /**
     * Generate car control configuration.
     */
    static auto generateCarControlConfig() -> std::shared_ptr<aace::core::config::EngineConfiguration>;
    
    /**
     * Return Power/Toggle Controller 
     */
    static auto getBoolController(const std::string& controlId, const std::string& controllerId = "") -> BoolController&;

    /**
     * Return Mode Controller
     */
    static auto getModeController(const std::string& controlId, const std::string& controllerId) ->  ModeController&;
    
    /**
     * Return Range Controller
     */
    static auto getRangeController(const std::string& controlId, const std::string& controllerId) ->  RangeController&;

private:

    // Generates a key for map lookup
    static auto genKey(std::string endpointId, std::string controllerId = "") -> std::string;
    // Used to maintain state of Power and Toggle controllers
    static std::unordered_map<std::string, BoolController> m_boolControllers;
    // Used to maintain state of Range Controllers
    static std::unordered_map<std::string, ModeController> m_modeControllers;
    // Used to maintain state of Mode Controllers
    static std::unordered_map<std::string, RangeController> m_rangeControllers;
};

} // sampleApp::carControl 
} // sampleApp

#endif // SAMPLEAPP_CARCONTROL_CARCONTROL_DATA_PROVIDER_H