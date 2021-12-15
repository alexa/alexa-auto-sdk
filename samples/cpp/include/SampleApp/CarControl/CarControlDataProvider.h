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

class CarControlDataProvider {
public:
    /**
     * Initialize data from car control configuration.
     */
    static auto initialize(std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationFiles)
        -> void;

    /**
     * Generate car control configuration.
     */
    static auto generateCarControlConfig() -> std::shared_ptr<aace::core::config::EngineConfiguration>;

    /**
     * Return Power/Toggle Controller 
     */
    static auto getBoolController(const std::string& controlId, const std::string& controllerId = "")
        -> BoolController&;

    /**
     * Return Mode Controller
     */
    static auto getModeController(const std::string& controlId, const std::string& controllerId) -> ModeController&;

    /**
     * Return Range Controller
     */
    static auto getRangeController(const std::string& controlId, const std::string& controllerId) -> RangeController&;

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

}  // namespace carControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_CARCONTROL_CARCONTROL_DATA_PROVIDER_H