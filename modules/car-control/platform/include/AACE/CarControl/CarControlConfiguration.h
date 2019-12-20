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

#ifndef AACE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H
#define AACE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H

/** @file */

#include <string>
#include "AACE/CarControl/CarControl.h"
#include "AACE/Core/EngineConfiguration.h"

namespace aace {
namespace carControl {
namespace config {

/**
 * Default zones are created automatically when used in both generic and canned controls.
 */
namespace zone {
static const std::string ALL = "zone.all";
static const std::string DRIVER = "zone.driver";
static const std::string DRIVER_ROW = "zone.driver.row";
static const std::string FIRST_ROW = "zone.first.row";
static const std::string FOURTH_ROW = "zone.fourth.row";
static const std::string FRONT = "zone.front";
static const std::string PASSENGER = "zone.passenger";
static const std::string PASSENGER_ROW = "zone.passenger.row";
static const std::string REAR = "zone.rear";
static const std::string REAR_DRIVER = "zone.rear.driver";
static const std::string REAR_PASSENGER = "zone.rear.passenger";
static const std::string SECOND_ROW = "zone.second.row";
static const std::string THIRD_ROW = "zone.third.row";
}  // namespace zone

/**
 * The @c CarControlConfiguration class is a factory interface for creating Car Control configuration objects.
 */
class CarControlConfiguration : public aace::core::config::EngineConfiguration {
public:
    static std::shared_ptr<CarControlConfiguration> create();

    /**
     * Create a control endpoint.
     *
     * @param [in] endpointId Unique identifier of the control.
     * @param [in] zoneId Unique identifier of the zone the control is located in. Default is @c zone::ALL.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& createControl(
        const std::string& controlId,
        const std::string& zoneId = zone::ALL) = 0;

    /**
     * Add an @c assetId to the enclosing element. Allowed for control, toggle controller, range controller,
     * mode controller, preset and mode.
     *
     * @param [in] assetId Identifier of the asset.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addAssetId(const std::string& assetId) = 0;

    /**
     * Add a PowerController to the enclosing control
     *
     * @param [in] retrievable @true if the value is evented (not use currently).
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addPowerController(bool retrievable) = 0;

    /**
     * Add a ToggleController to the enclosing control
     *
     * @param [in] controllerId Identifier of the controller (unique for the control).
     * @param [in] retrievable @true if the value is evented (not use currently).
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addToggleController(const std::string& controllerId, bool retrievable) = 0;

    /**
     * Add a RangeController to the enclosing control
     *
     * @param [in] controllerId Identifier of the controller (unique for the control).
     * @param [in] retrievable @true if the value is evented (not use currently).
     * @param [in] minimum Minimal value of the controlled range.
     * @param [in] minimum Maximal value of the controlled range.
     * @param [in] precision Allowed precision of the value.
     * @param [in] unit Unit of the range.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addRangeController(
        const std::string& controllerId,
        bool retrievable,
        double minimum,
        double maximum,
        double precision,
        const std::string& unit = "") = 0;

    /**
     * Add a Preset to the enclosing RangeController (Only allowed for RangeController).
     *
     * @param [in] value Value of the preset.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addPreset(double value) = 0;

    /**
     * Add a ModeController to the enclosing control
     *
     * @param [in] controllerId Identifier of the controller (unique for the control).
     * @param [in] retrievable @true if the value is evented (not use currently).
     * @param [in] ordered @true if the modes are ordered.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addModeController(
        const std::string& controllerId,
        bool retrievable,
        bool ordered) = 0;

    /**
     * Add a @c value to the enclosing ModeController (Only allowed for ModeController).
     *
     * @param [in] value Value of the mode.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& addValue(const std::string& value) = 0;

    /**
     * Create a zone.
     *
     * @param [in] zoneId Unique identifier of the zone.
     * @return @c CarControlConfiguration to allow chaining.
     */
    virtual CarControlConfiguration& createZone(const std::string& zoneId) = 0;

    /**
     * Specify the path to a file that contains the predefined assets.
     *
     * @param [in] path The path to the file
     */
    virtual CarControlConfiguration& addDefaultAssetsPath(const std::string& path) = 0;

    /**
     * Specify the path to a file that defines additional assets.
     *
     * @param [in] path The path to the file
     */
    virtual CarControlConfiguration& addCustomAssetsPath(const std::string& path) = 0;
};

}  // namespace config
}  // namespace carControl
}  // namespace aace

#endif  // AACE_CAR_CONTROL_CAR_CONTROL_CONFIGURATION_H
