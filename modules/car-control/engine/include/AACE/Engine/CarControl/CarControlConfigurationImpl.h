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

#ifndef AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H
#define AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H

#include "AACE/CarControl/CarControlAssets.h"
#include "AACE/CarControl/CarControlConfiguration.h"

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

/** @file */

namespace aace {
namespace carControl {
namespace config {

class CarControlConfigurationImpl : public CarControlConfiguration {
private:
    enum class Option { CAPABILITY, ENDPOINT, MODE, PRESET, ZONE, ACTION };

    bool addUniqueAssetId(json& friendlyNames, const std::string& assetId);
    bool isOption(Option option);

public:
    CarControlConfigurationImpl();

    // CarControlConfiguration
    std::shared_ptr<std::istream> getStream() override;

    CarControlConfiguration& createEndpoint(const std::string& endpointId) override;
    CarControlConfiguration& addAssetId(const std::string& assetId) override;
    CarControlConfiguration& addPowerController(bool retrievable) override;
    CarControlConfiguration& addToggleController(const std::string& controllerId, bool retrievable) override;
    CarControlConfiguration& addActionTurnOn(const std::vector<std::string>& actions) override;
    CarControlConfiguration& addActionTurnOff(const std::vector<std::string>& actions) override;
    CarControlConfiguration& addRangeController(
        const std::string& controllerId,
        bool retrievable,
        double minimum,
        double maximum,
        double precision,
        const std::string& unit = "") override;
    CarControlConfiguration& addPreset(double value) override;
    CarControlConfiguration& addActionSetRange(const std::vector<std::string>& actions, double value) override;
    CarControlConfiguration& addActionAdjustRange(const std::vector<std::string>& actions, double delta) override;
    CarControlConfiguration& addModeController(const std::string& controllerId, bool retrievable, bool ordered)
        override;
    CarControlConfiguration& addValue(const std::string& value) override;
    CarControlConfiguration& addActionSetMode(const std::vector<std::string>& actions, const std::string& value)
        override;
    CarControlConfiguration& addActionAdjustMode(const std::vector<std::string>& actions, int delta) override;

    CarControlConfiguration& createZone(const std::string& zoneId) override;
    CarControlConfiguration& addMembers(const std::vector<std::string>& endpointIds) override;
    CarControlConfiguration& setDefaultZone(const std::string& zoneId) override;
    CarControlConfiguration& addDefaultAssetsPath(const std::string& path) override;
    CarControlConfiguration& addCustomAssetsPath(const std::string& path) override;

private:
    bool m_failed;

    json m_document;

    std::vector<Option> m_allowedOptions;
};

}  // namespace config
}  // namespace carControl
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H
