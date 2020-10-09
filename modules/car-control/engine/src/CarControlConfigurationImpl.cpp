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

#include "AACE/Engine/CarControl/CarControlConfigurationImpl.h"

#include "AACE/CarControl/CarControlAssets.h"
#include "AACE/CarControl/CarControlConfiguration.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace carControl {
namespace config {

using namespace aace::carControl::assets;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.carControl.config.CarControlConfigurationImpl");

/// "ActionsToDirective" action mapping type
static const std::string ACTIONS_TO_DIRECTIVE = "ActionsToDirective";

// default Engine constructor
std::shared_ptr<CarControlConfiguration> CarControlConfiguration::create() {
    return std::make_shared<CarControlConfigurationImpl>();
}

CarControlConfigurationImpl::CarControlConfigurationImpl() : m_failed(false) {
    // clang-format off
    m_document = {{
        "aace.carControl", {
            {"endpoints", json::array()}
        }
    }};
    // clang-format on
}

std::shared_ptr<std::istream> CarControlConfigurationImpl::getStream() {
    auto stream = std::make_shared<std::stringstream>();
    try {
        ThrowIf(m_failed, "previouslyFailed");
        AACE_DEBUG(LX(TAG).sensitive("json", m_document));
        *stream << m_document;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        *stream << "{}";
    }
    return stream;
}

CarControlConfiguration& CarControlConfigurationImpl::createEndpoint(const std::string& endpointId) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIf(endpointId == "", "emptyEndpointId");
        auto& endpoints = m_document["aace.carControl"]["endpoints"];
        for (const auto& item : endpoints.items()) {
            const auto& endpoint = item.value();
            ThrowIf(endpoint["endpointId"] == endpointId, "duplicateEndpoint " + endpointId);
        }

        // clang-format off
        json endpoint = {
            {"endpointId", endpointId},
            {"endpointResources", {
                {"friendlyNames", json::array()}
            }},
            {"capabilities", json::array()}
        };
        // clang-format on

        endpoints.push_back(endpoint);
        m_allowedOptions = {Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("endpointId", endpointId).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addAssetId(const std::string& assetId) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        if (isOption(Option::ZONE)) {
            auto& zone = m_document["aace.carControl"]["zones"].back();
            auto& friendlyNames = zone["zoneResources"]["friendlyNames"];
            ThrowIfNot(addUniqueAssetId(friendlyNames, assetId), "duplicateAssetId");
        } else {
            auto& endpoints = m_document["aace.carControl"]["endpoints"];
            ThrowIf(endpoints.size() == 0, "noEndpoint");
            auto& endpoint = endpoints.back();
            if (isOption(Option::CAPABILITY)) {
                auto& capability = endpoint["capabilities"].back();
                ThrowIfNot(capability.contains("capabilityResources"), "cannotAddAsset");
                auto& friendlyNames = capability["capabilityResources"]["friendlyNames"];
                ThrowIfNot(addUniqueAssetId(friendlyNames, assetId), "duplicateAssetId");
            } else if (isOption(Option::MODE)) {
                auto& capability = endpoint["capabilities"].back();
                auto& mode = capability["configuration"]["supportedModes"].back();
                auto& friendlyNames = mode["modeResources"]["friendlyNames"];
                ThrowIfNot(addUniqueAssetId(friendlyNames, assetId), "duplicateAssetId");
            } else if (isOption(Option::PRESET)) {
                auto& capability = endpoint["capabilities"].back();
                auto& preset = capability["configuration"]["presets"].back();
                auto& friendlyNames = preset["presetResources"]["friendlyNames"];
                ThrowIfNot(addUniqueAssetId(friendlyNames, assetId), "duplicateAssetId");
            } else if (isOption(Option::ENDPOINT)) {
                auto& friendlyNames = endpoint["endpointResources"]["friendlyNames"];
                ThrowIfNot(addUniqueAssetId(friendlyNames, assetId), "duplicateAssetId");
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("assetId", assetId).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addPowerController(bool retrievable) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ENDPOINT), "powerControllerNotAllowed");

        // clang-format off
        json capability = {
            {"type", "AlexaInterface"},
            {"interface", "Alexa.PowerController"},
            {"version", "3"},
            {"properties", {
                {"supported", {
                    {{"name", "powerState"}}
                }},
                {"proactivelyReported", false},
                {"retrievable", retrievable}
            }}
        };
        // clang-format on

        auto& endpoints = m_document["aace.carControl"]["endpoints"];
        auto& endpoint = endpoints.back();
        auto& capabilities = endpoint["capabilities"];
        capabilities.push_back(capability);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addToggleController(
    const std::string& controllerId,
    bool retrievable) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ENDPOINT), "toggleControllerNotAllowed");

        // clang-format off
        json capability = {
            {"type", "AlexaInterface"},
            {"interface", "Alexa.ToggleController"},
            {"version", "3"},
            {"instance", controllerId},
            {"properties", {
                {"supported", {
                    {{"name", "toggleState"}}
                }},
                {"proactivelyReported", false},
                {"retrievable", retrievable}
            }},
            {"capabilityResources", {
                {"friendlyNames", json::array()}
            }}
        };
        // clang-format on

        auto& endpoints = m_document["aace.carControl"]["endpoints"];
        auto& endpoint = endpoints.back();
        auto& capabilities = endpoint["capabilities"];
        capabilities.push_back(capability);

        m_allowedOptions = {Option::ACTION, Option::ENDPOINT, Option::CAPABILITY};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("controllerId", controllerId).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addActionTurnOn(const std::vector<std::string>& actions) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ACTION), "actionTurnOnNotAllowed");
        ThrowIf(actions.empty(), "noActionsSpecified");

        json actionsArray = json::array();
        for (auto action = actions.begin(); action != actions.end(); ++action) {
            actionsArray.push_back(*action);
        }

        // clang-format off
        json actionMapping = {
            {"@type", ACTIONS_TO_DIRECTIVE},
            {"actions", actionsArray},
            {"directive", {
                {"name", "TurnOn"},
                {"payload", json::object()}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.ToggleController", "notToggleController");
        if (!capability.contains("semantics")) {
            capability["semantics"] = json::object();
        }
        auto& semantics = capability["semantics"];
        if (!semantics.contains("actionMappings")) {
            semantics["actionMappings"] = json::array();
        }
        auto& actionMappings = semantics["actionMappings"];
        actionMappings.push_back(actionMapping);

        m_allowedOptions = {Option::ACTION, Option::CAPABILITY, Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addActionTurnOff(const std::vector<std::string>& actions) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ACTION), "actionTurnOffNotAllowed");
        ThrowIf(actions.empty(), "noActionsSpecified");

        json actionsArray = json::array();
        for (auto action = actions.begin(); action != actions.end(); ++action) {
            actionsArray.push_back(*action);
        }

        // clang-format off
        json actionMapping = {
            {"@type", ACTIONS_TO_DIRECTIVE},
            {"actions", actionsArray},
            {"directive", {
                {"name", "TurnOff"},
                {"payload", json::object()}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.ToggleController", "notToggleController");
        if (!capability.contains("semantics")) {
            capability["semantics"] = json::object();
        }
        auto& semantics = capability["semantics"];
        if (!semantics.contains("actionMappings")) {
            semantics["actionMappings"] = json::array();
        }
        auto& actionMappings = semantics["actionMappings"];
        actionMappings.push_back(actionMapping);

        m_allowedOptions = {Option::ACTION, Option::CAPABILITY, Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addRangeController(
    const std::string& controllerId,
    bool retrievable,
    double minimum,
    double maximum,
    double precision,
    const std::string& unit) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ENDPOINT), "rangeControllerNotAllowed");

        // clang-format off
        json capability = {
            {"type", "AlexaInterface"},
            {"interface", "Alexa.RangeController"},
            {"version", "3"},
            {"instance", controllerId},
            {"properties", {
                {"supported", {
                    {{"name", "rangeValue"}}
                }},
                {"proactivelyReported", false},
                {"retrievable", retrievable}
            }},
            {"capabilityResources", {
                {"friendlyNames", json::array()}
            }},
            {"configuration", {
                {"supportedRange", {
                    {"minimumValue", minimum},
                    {"maximumValue", maximum},
                    {"precision", precision}
                }}
            }}
        };
        // clang-format on
        if (!unit.empty()) {
            capability["configuration"]["unitOfMeasure"] = unit;
        }

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capabilities = endpoint["capabilities"];
        capabilities.push_back(capability);

        m_allowedOptions = {Option::ACTION, Option::ENDPOINT, Option::CAPABILITY, Option::PRESET};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("controllerId", controllerId).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addPreset(double value) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::PRESET), "addPresetNotAllowed");

        // clang-format off
        json preset = {
            {"rangeValue", value},
            {"presetResources", {
                {"friendlyNames", json::array()}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.RangeController", "notRangeController");
        auto& configuration = capability["configuration"];
        if (!configuration.contains("presets")) {
            configuration["presets"] = json::array();
        }
        auto& presets = configuration["presets"];
        presets.push_back(preset);

        m_allowedOptions = {Option::ACTION, Option::ENDPOINT, Option::PRESET};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("value", value).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addActionSetRange(
    const std::vector<std::string>& actions,
    double value) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ACTION), "actionSetRangeNotAllowed");
        ThrowIf(actions.empty(), "noActionsSpecified");

        json actionsArray = json::array();
        for (auto action = actions.begin(); action != actions.end(); ++action) {
            actionsArray.push_back(*action);
        }

        // clang-format off
        json actionMapping = {
            {"@type", ACTIONS_TO_DIRECTIVE},
            {"actions", actionsArray},
            {"directive", {
                {"name", "SetRangeValue"},
                {"payload", {
                    {"rangeValue", value}
                }}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.RangeController", "notRangeController");
        if (!capability.contains("semantics")) {
            capability["semantics"] = json::object();
        }
        auto& semantics = capability["semantics"];
        if (!semantics.contains("actionMappings")) {
            semantics["actionMappings"] = json::array();
        }
        auto& actionMappings = semantics["actionMappings"];
        actionMappings.push_back(actionMapping);

        m_allowedOptions = {Option::ACTION, Option::CAPABILITY, Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addActionAdjustRange(
    const std::vector<std::string>& actions,
    double delta) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ACTION), "actionAdjustRangeNotAllowed");
        ThrowIf(actions.empty(), "noActionsSpecified");

        json actionsArray = json::array();
        for (auto action = actions.begin(); action != actions.end(); ++action) {
            actionsArray.push_back(*action);
        }

        // clang-format off
        json actionMapping = {
            {"@type", ACTIONS_TO_DIRECTIVE},
            {"actions", actionsArray},
            {"directive", {
                {"name", "AdjustRangeValue"},
                {"payload", {
                    {"rangeValueDelta", delta},
                    {"rangeValueDeltaDefault", false}
                }}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.RangeController", "notRangeController");
        if (!capability.contains("semantics")) {
            capability["semantics"] = json::object();
        }
        auto& semantics = capability["semantics"];
        if (!semantics.contains("actionMappings")) {
            semantics["actionMappings"] = json::array();
        }
        auto& actionMappings = semantics["actionMappings"];
        actionMappings.push_back(actionMapping);

        m_allowedOptions = {Option::ACTION, Option::CAPABILITY, Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addModeController(
    const std::string& controllerId,
    bool retrievable,
    bool ordered) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ENDPOINT), "modeControllerNotAllowed");

        // clang-format off
        json capability = {
            {"type", "AlexaInterface"},
            {"interface", "Alexa.ModeController"},
            {"version", "3"},
            {"instance", controllerId},
            {"properties", {
                {"supported", {
                    {{"name", "mode"}}
                }},
                {"proactivelyReported", false},
                {"retrievable", retrievable}
            }},
            {"capabilityResources", {
                {"friendlyNames", json::array()}
            }},
            {"configuration", {
                {"ordered", ordered}
            }}
        };
        // clang-format on

        auto& endpoints = m_document["aace.carControl"]["endpoints"];
        auto& endpoint = endpoints.back();
        auto& capabilities = endpoint["capabilities"];
        capabilities.push_back(capability);

        m_allowedOptions = {Option::ACTION, Option::ENDPOINT, Option::CAPABILITY, Option::MODE};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("controllerId", controllerId).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addValue(const std::string& value) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::MODE), "addValueNotAllowed");

        // clang-format off
        json mode = {
            {"value", value},
            {"modeResources", {
                {"friendlyNames", json::array()}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.ModeController", "notModeController");
        auto& configuration = capability["configuration"];
        if (!configuration.contains("supportedModes")) {
            configuration["supportedModes"] = json::array();
        }
        auto& supportedModes = configuration["supportedModes"];
        supportedModes.push_back(mode);

        m_allowedOptions = {Option::ACTION, Option::ENDPOINT, Option::MODE};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("value", value).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addActionSetMode(
    const std::vector<std::string>& actions,
    const std::string& value) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ACTION), "actionSetModeNotAllowed");
        ThrowIf(actions.empty(), "noActionsSpecified");

        json actionsArray = json::array();
        for (auto action = actions.begin(); action != actions.end(); ++action) {
            actionsArray.push_back(*action);
        }

        // clang-format off
        json actionMapping = {
            {"@type", ACTIONS_TO_DIRECTIVE},
            {"actions", actionsArray},
            {"directive", {
                {"name", "SetMode"},
                {"payload", {
                    {"mode", value}
                }}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.ModeController", "notModeController");
        if (!capability.contains("semantics")) {
            capability["semantics"] = json::object();
        }
        auto& semantics = capability["semantics"];
        if (!semantics.contains("actionMappings")) {
            semantics["actionMappings"] = json::array();
        }
        auto& actionMappings = semantics["actionMappings"];
        actionMappings.push_back(actionMapping);

        m_allowedOptions = {Option::ACTION, Option::CAPABILITY, Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addActionAdjustMode(
    const std::vector<std::string>& actions,
    int delta) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ACTION), "actionAdjustModeNotAllowed");
        ThrowIf(actions.empty(), "noActionsSpecified");

        json actionsArray = json::array();
        for (auto action = actions.begin(); action != actions.end(); ++action) {
            actionsArray.push_back(*action);
        }

        // clang-format off
        json actionMapping = {
            {"@type", ACTIONS_TO_DIRECTIVE},
            {"actions", actionsArray},
            {"directive", {
                {"name", "AdjustMode"},
                {"payload", {
                    {"modeDelta", delta}
                }}
            }}
        };
        // clang-format on

        auto& endpoint = m_document["aace.carControl"]["endpoints"].back();
        auto& capability = endpoint["capabilities"].back();
        ThrowIfNot(capability["interface"] == "Alexa.ModeController", "notModeController");
        if (!capability.contains("semantics")) {
            capability["semantics"] = json::object();
        }
        auto& semantics = capability["semantics"];
        if (!semantics.contains("actionMappings")) {
            semantics["actionMappings"] = json::array();
        }
        auto& actionMappings = semantics["actionMappings"];
        actionMappings.push_back(actionMapping);

        m_allowedOptions = {Option::ACTION, Option::CAPABILITY, Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::createZone(const std::string& zoneId) {
    try {
        ThrowIf(m_failed, "previouslyFailed");

        if (!m_document["aace.carControl"].contains("zones")) {
            m_document["aace.carControl"]["zones"] = json::array();
        }

        auto& zones = m_document["aace.carControl"]["zones"];
        for (const auto& item : zones.items()) {
            const auto& zone = item.value();
            ThrowIf(zone["zoneId"] == zoneId, "duplicateZone " + zoneId);
        }

        // clang-format off
        json zone = {
            {"zoneId", zoneId},
            {"zoneResources", {
                {"friendlyNames", json::array()}
            }}
        };
        // clang-format on
        zones.push_back(zone);

        m_allowedOptions = {Option::ZONE};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("zoneId", zoneId).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addMembers(const std::vector<std::string>& endpointIds) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        ThrowIfNot(isOption(Option::ZONE), "addMembersNotAllowed");

        auto& zone = m_document["aace.carControl"]["zones"].back();
        if (!zone.contains("members")) {
            zone["members"] = json::array();
        }
        auto& members = zone["members"];
        for (auto endpointItr = endpointIds.begin(); endpointItr != endpointIds.end(); ++endpointItr) {
            json endpoint = {{"endpointId", *endpointItr}};
            members.push_back(endpoint);
        }
        m_allowedOptions = {Option::ZONE};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::setDefaultZone(const std::string& zoneId) {
    try {
        ThrowIf(zoneId == "", "zoneIdIsEmpty");
        m_document["aace.carControl"]["defaultZoneId"] = zoneId;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        m_failed = true;
    }
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addDefaultAssetsPath(const std::string& path) {
    m_document["aace.carControl"]["assets"]["defaultAssetsPath"] = path;
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addCustomAssetsPath(const std::string& path) {
    m_document["aace.carControl"]["assets"]["customAssetsPath"] = path;
    return *this;
}

bool CarControlConfigurationImpl::addUniqueAssetId(json& friendlyNames, const std::string& assetId) {
    try {
        for (auto& item : friendlyNames.items()) {
            auto& asset = item.value();
            if (asset["value"]["assetId"] == assetId) {
                return false;
            }
        }
        // clang-format off
        json friendlyName = {
            {"@type", "asset"},
            {"value", {
                {"assetId", assetId}
            }}
        };
        // clang-format on
        friendlyNames.push_back(friendlyName);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("assetId", assetId).d("reason", ex.what()));
        return false;
    }
}

bool CarControlConfigurationImpl::isOption(Option option) {
    return std::find(m_allowedOptions.begin(), m_allowedOptions.end(), option) != m_allowedOptions.end();
}

}  // namespace config
}  // namespace carControl
}  // namespace aace
