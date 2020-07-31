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

#ifndef SAMPLEAPP_CARCONTROL_RANGECONTROLLER_H
#define SAMPLEAPP_CARCONTROL_RANGECONTROLLER_H

#include <string>

namespace sampleApp {
namespace carControl {

/**
 * Application needs to keep track of the current range value, as well 
 * as the min and max values for an "Alexa.RangeController". It also needs
 * to be able to adjust the range based on a delta. This class models a 
 * range controller to be able to query, set, and adjust range values.
 */
class RangeController {
private:
    double m_min;
    double m_max;
    double m_value;

public:
    RangeController() {
    }
    RangeController(double min, double max) : m_min{min}, m_max{max}, m_value{min} {
    }

    double getValue() {
        return m_value;
    }

    void setValue(double value) {
        clamp(value);
    }

    void adjustValue(double delta) {
        clamp(m_value + delta);
    }

private:
    void clamp(double value) {
        m_value = std::min(m_max, std::max(value, m_min));
    }
};

}  // namespace carControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_CARCONTROL_RANGECONTROLLER_H