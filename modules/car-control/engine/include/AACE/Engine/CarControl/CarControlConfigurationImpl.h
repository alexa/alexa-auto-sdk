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
    enum class Option { CAPABILITY, ENDPOINT, MODE, PRESET, ZONE };

    bool addUniqueAssetId(json& friendlyNames, const std::string& assetId);
    void createDefaultZone(const std::string& zoneId);
    bool isOption(Option option);

public:
    CarControlConfigurationImpl();

    // CarControlConfiguration
    std::shared_ptr<std::istream> getStream() override;

    // Generic
    CarControlConfiguration& createControl(const std::string& controlId, const std::string& zoneId = zone::ALL)
        override;
    CarControlConfiguration& addAssetId(const std::string& assetId) override;
    CarControlConfiguration& addPowerController(bool retrievable) override;
    CarControlConfiguration& addToggleController(const std::string& controllerId, bool retrievable) override;
    CarControlConfiguration& addRangeController(
        const std::string& controllerId,
        bool retrievable,
        double minimum,
        double maximum,
        double precision,
        const std::string& unit = "") override;
    CarControlConfiguration& addPreset(double value) override;
    CarControlConfiguration& addModeController(const std::string& controllerId, bool retrievable, bool ordered)
        override;
    CarControlConfiguration& addValue(const std::string& value) override;

    CarControlConfiguration& createZone(const std::string& zoneId) override;
    CarControlConfiguration& addDefaultAssetsPath(const std::string& path) override;
    CarControlConfiguration& addCustomAssetsPath(const std::string& path) override;

private:
    bool m_failed;

    json m_document;

    std::vector<Option> m_allowedOptions;

    std::unordered_map<std::string, std::vector<std::string>> m_idToAssetIds = {
        {zone::ALL, {aace::carControl::assets::alexa::location::ALL}},
        {zone::DRIVER, {aace::carControl::assets::alexa::location::DRIVER}},
        {zone::DRIVER_ROW, {aace::carControl::assets::alexa::location::DRIVER_ROW}},
        {zone::FIRST_ROW, {aace::carControl::assets::alexa::location::FIRST_ROW}},
        {zone::FOURTH_ROW, {aace::carControl::assets::alexa::location::FOURTH_ROW}},
        {zone::FRONT, {aace::carControl::assets::alexa::location::FRONT}},
        {zone::PASSENGER, {aace::carControl::assets::alexa::location::PASSENGER}},
        {zone::PASSENGER_ROW, {aace::carControl::assets::alexa::location::PASSENGER_ROW}},
        {zone::REAR, {aace::carControl::assets::alexa::location::REAR}},
        {zone::REAR_DRIVER, {aace::carControl::assets::alexa::location::REAR_DRIVER}},
        {zone::REAR_PASSENGER, {aace::carControl::assets::alexa::location::REAR_PASSENGER}},
        {zone::SECOND_ROW, {aace::carControl::assets::alexa::location::SECOND_ROW}},
        {zone::THIRD_ROW, {aace::carControl::assets::alexa::location::THIRD_ROW}},
    };

    std::vector<std::string> m_defaultZones = {zone::ALL,
                                               zone::DRIVER,
                                               zone::DRIVER_ROW,
                                               zone::FIRST_ROW,
                                               zone::FOURTH_ROW,
                                               zone::FRONT,
                                               zone::PASSENGER,
                                               zone::PASSENGER_ROW,
                                               zone::REAR,
                                               zone::REAR_DRIVER,
                                               zone::REAR_PASSENGER,
                                               zone::SECOND_ROW,
                                               zone::THIRD_ROW};
};

}  // namespace config
}  // namespace carControl
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H
