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

#ifndef SAMPLEAPP_CARCONTROL_MODECONTROLLER_H
#define SAMPLEAPP_CARCONTROL_MODECONTROLLER_H

#include <algorithm>
#include <string>
#include <vector>
#include <exception>

namespace sampleApp {
namespace carControl {

/**
 * Application needs to keep track of currently selected mode of an
 * "Alexa.ModeController". It also needs to be able to adjust the mode
 * based on a delta. This class models a mode controller to be able to
 * query, set, and adjust mode.
 */
class ModeController {
private:
    int m_index = 0;
    std::vector<std::string> m_modes;

public:
    ModeController() {
    }

    void addMode(std::string mode) {
        m_modes.push_back(mode);
    }

    std::string getMode() {
        return m_modes[m_index];
    }

    /**
     * The mode is saved by updating the index to referenced mode.
     */
    bool setMode(std::string mode) {
        auto it = std::find(m_modes.begin(), m_modes.end(), mode);
        if (it != m_modes.end()) {
            m_index = std::distance(m_modes.begin(), it);
            return true;
        }

        return false;
    }

    /**
     * Increase index to the next valid mode.
     */
    void adjustMode(int delta) {
        m_index += delta;
        if (m_index < 0) {
            m_index = 0;
        } else if (m_index >= m_modes.size()) {
            m_index = m_modes.size() - 1;
        }
    }

    static std::string genKey(std::string endpointId, std::string controllerId) {
        return "ModeController#" + endpointId + "#" + controllerId;
    }
};

}  // namespace carControl
}  // namespace sampleApp

#endif  // SAMPLEAPP_CARCONTROL_MODECONTROLLER_H