/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Engine/Utils/JSON/JSON.h>

#include "AACE/Engine/APL/APLRuntimePropertyGenerator.h"

namespace aace {
namespace engine {
namespace apl {

// json namespace alias
namespace json = aace::engine::utils::json;

// Properties to control the APL runtime
static const std::string APL_DRIVING_STATE("drivingState");
static const std::string APL_THEME("theme");
static const std::string APL_VIDEO("video");

// Platform properties
static const std::string DRIVING_STATE("drivingState");
static const std::string UI_MODE("uiMode");
static const std::string THEME_ID("themeId");

// Property values
static const std::string ENABLED("enabled");
static const std::string DISABLED("disabled");
static const std::string DAY("day");
static const std::string NIGHT("night");
static const std::string MOVING("moving");
static const std::string PARKED("parked");
static const std::string LIGHT("light");
static const std::string DARK("dark");

// String to identify log entries originating from this file.
static const std::string TAG("aace.apl.APLRuntimePropertyGenerator");

APLRuntimePropertyGenerator::APLRuntimePropertyGenerator() {
    m_platformProperties[DRIVING_STATE] = MOVING;
    m_platformProperties[UI_MODE] = DAY;
    m_platformProperties[THEME_ID] = "";
}

void APLRuntimePropertyGenerator::handleProperty(const std::string& name, const std::string& value) {
    m_platformProperties[name] = value;
}

std::string APLRuntimePropertyGenerator::getAPLRuntimeProperties() {
    json::Value properties;

    std::string drivingState = m_platformProperties.at(DRIVING_STATE);
    std::string themeId = m_platformProperties.at(THEME_ID);
    std::string theme = m_platformProperties.at(UI_MODE) == DAY ? LIGHT : DARK;
    std::string video = drivingState == PARKED ? ENABLED : DISABLED;

    // To keep the API flexible warn on unknown values only
    if (drivingState != PARKED || drivingState != MOVING) {
        AACE_WARN(LX(TAG).d("drivingStateUnknownValue", drivingState));
    }

    // Per contract with Alexa styles, the Automotive theme id is appended to the theme
    if (!themeId.empty()) {
        theme += std::string("-") + themeId;
    }

    // Create properties for the APL runtime
    properties[APL_DRIVING_STATE] = drivingState;
    properties[APL_THEME] = theme;
    properties[APL_VIDEO] = video;

    return properties.dump();
}

}  // namespace apl
}  // namespace engine
}  // namespace aace
