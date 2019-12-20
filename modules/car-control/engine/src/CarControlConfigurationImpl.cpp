/*
 * Auto Core Engine SDK
 * Copyright 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved. Amazon.com Confidential Information -- Do Not
 * Distribute.
 *
 * These materials are licensed as "Alexa Materials" under the Alexa Voice Service Agreement (the "License") of the
 * Alexa Voice Service Program, which is available at
 * https://developer.amazon.com/public/solutions/alexa/alexa-voice-service/support/terms-and-agreements. See the License
 * for the specific language governing permissions and limitations under the License. These materials are Confidential
 * Information and may not be distributed. These materials are provided to you on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied.
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

// default Engine constructor
std::shared_ptr<CarControlConfiguration> CarControlConfiguration::create() {
    return std::make_shared<CarControlConfigurationImpl>();
}

CarControlConfigurationImpl::CarControlConfigurationImpl() : m_failed(false) {
    // clang-format off
    m_document = {{
        "aace.carControl", {
            {"endpoints", json::array()},
            {"zones", json::array()}
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

CarControlConfiguration& CarControlConfigurationImpl::createControl(
    const std::string& controlId,
    const std::string& zoneId) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
        auto& endpoints = m_document["aace.carControl"]["endpoints"];
        for (const auto& item : endpoints.items()) {
            const auto& endpoint = item.value();
            ThrowIf(endpoint["endpointId"] == controlId, "duplicateControl " + controlId);
        }

        // clang-format off
        json endpoint = {
            {"endpointId", controlId},
            {"endpointResources", {
                {"friendlyNames", json::array()}
            }},
            {"capabilities", json::array()},
            {"relationships", {{
                "isMemberOf", {{
                    "zoneId", zoneId
                }}
            }}}
        };
        // clang-format on
        createDefaultZone(zoneId);
        endpoints.push_back(endpoint);

        m_allowedOptions = {Option::ENDPOINT};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("controlId", controlId).d("zoneId", zoneId).d("reason", ex.what()));
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

        m_allowedOptions = {Option::ENDPOINT, Option::CAPABILITY};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("controllerId", controllerId).d("reason", ex.what()));
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

        m_allowedOptions = {Option::ENDPOINT, Option::CAPABILITY, Option::PRESET};
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

        m_allowedOptions = {Option::ENDPOINT, Option::PRESET};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("value", value).d("reason", ex.what()));
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

        m_allowedOptions = {Option::ENDPOINT, Option::CAPABILITY, Option::MODE};
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

        m_allowedOptions = {Option::ENDPOINT, Option::MODE};
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("value", value).d("reason", ex.what()));
        m_failed = true;
    }

    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::createZone(const std::string& zoneId) {
    try {
        ThrowIf(m_failed, "previouslyFailed");
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

CarControlConfiguration& CarControlConfigurationImpl::addDefaultAssetsPath(const std::string& path) {
    m_defaultAssetsPath = path;
    return *this;
}

CarControlConfiguration& CarControlConfigurationImpl::addCustomAssetsPath(const std::string& path) {
    m_customAssetsPath = path;
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

void CarControlConfigurationImpl::createDefaultZone(const std::string& zoneId) {
    try {
        auto index = std::find(m_defaultZones.begin(), m_defaultZones.end(), zoneId);
        if (index != m_defaultZones.end()) {
            createZone(zoneId);
            for (auto& assetId : m_idToAssetIds[zoneId]) {
                addAssetId(assetId);
            }
            m_defaultZones.erase(index);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool CarControlConfigurationImpl::isOption(Option option) {
    return std::find(m_allowedOptions.begin(), m_allowedOptions.end(), option) != m_allowedOptions.end();
}

}  // namespace config
}  // namespace carControl
}  // namespace aace
