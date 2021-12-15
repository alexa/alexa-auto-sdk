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

#ifndef SAMPLEAPP_CARCONTROL_BOOLCONTROLLER_H
#define SAMPLEAPP_CARCONTROL_BOOLCONTROLLER_H

#include <algorithm>
#include <string>
#include <vector>
#include <exception>

namespace sampleApp {
namespace carControl {

/**
 * Application needs to maintain power or toggle state for
 * "Alexa.PowerController" and "Alexa.ToggleController". This class
 * provides a way to save and retrieve that state.
 */
class BoolController {
private:
    bool m_value = false;

public:
    BoolController() {
    }

    void setValue(bool value) {
        m_value = value;
    }

    bool getValue() {
        return m_value;
    }
};

}  // namespace carControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_CARCONTROL_BOOLCONTROLLER_H