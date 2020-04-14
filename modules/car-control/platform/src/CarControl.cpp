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

#include <AACE/CarControl/CarControl.h>

namespace aace {
namespace carControl {

CarControl::~CarControl() = default;

/**
 * PowerController
 */
bool CarControl::turnPowerControllerOn(const std::string& controlId) {
    return false;
}

bool CarControl::turnPowerControllerOff(const std::string& controlId) {
    return false;
}

bool CarControl::isPowerControllerOn(const std::string& controlId, bool& isOn) {
    return false;
}

/**
 * ToggleController
 */
bool CarControl::turnToggleControllerOn(const std::string& controlId, const std::string& controllerId) {
    return false;
}

bool CarControl::turnToggleControllerOff(const std::string& controlId, const std::string& controllerId) {
    return false;
}

bool CarControl::isToggleControllerOn(const std::string& controlId, const std::string& controllerId, bool& isOn) {
    return false;
}

/**
 * RangeController
 */
bool CarControl::setRangeControllerValue(const std::string& controlId, const std::string& controllerId, double value) {
    return false;
}

bool CarControl::adjustRangeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    double delta) {
    return false;
}

bool CarControl::getRangeControllerValue(const std::string& controlId, const std::string& controllerId, double& value) {
    return false;
}

/**
 * ModeController
 */
bool CarControl::setModeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    const std::string& value) {
    return false;
}

bool CarControl::adjustModeControllerValue(const std::string& controlId, const std::string& controllerId, int delta) {
    return false;
}

bool CarControl::getModeControllerValue(
    const std::string& controlId,
    const std::string& controllerId,
    std::string& value) {
    return false;
}

}  // namespace carControl
}  // namespace aace
