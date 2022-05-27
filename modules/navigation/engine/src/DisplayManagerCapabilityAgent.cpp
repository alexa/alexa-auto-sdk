/*
 * Copyright 2017-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <iostream>

#include <string>
#include <nlohmann/json.hpp>
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <AVSCommon/Utils/JSON/JSONUtils.h>

#include "AACE/Engine/Navigation/DisplayManagerCapabilityAgent.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace navigation {

// String to identify log entries originating from this file.
static const std::string TAG("DisplayManagerCapabilityAgent");

/// The namespace for @c DisplayManager interface.
static const std::string NAMESPACE{"Navigation.DisplayManager"};

/// The @c ControlDisplay directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName CONTROL_DISPLAY{NAMESPACE, "ControlDisplay"};

/// The @c ShowAlternativeRoutes directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName SHOW_ALTERNATIVE_ROUTES{NAMESPACE,
                                                                                      "ShowAlternativeRoutes"};
/// @c AlexaInterface interface name.
static const std::string ALEXA_INTERFACE_TYPE = "AlexaInterface";

/// @c DisplayManager interface name.
static const std::string DISPLAY_MANAGER_CAPABILITY_INTERFACE_NAME = "Navigation.DisplayManager";

/// @c DisplayManager interface version.
static const std::string DISPLAY_MANAGER_CAPABILITY_INTERFACE_VERSION = "1.0";

/// @c ControlDisplaySucceeded event name.
static const std::string CONTROL_DISPLAY_SUCCEEDED = "ControlDisplaySucceeded";

/// @c ShowAlternativeRoutesSucceeded event name.
static const std::string SHOW_ALTERNATIVE_ROUTES_SUCCEEDED = "ShowAlternativeRoutesSucceeded";

/// @c ControlDisplayFailed event name.
static const std::string CONTROL_DISPLAY_FAILED = "ControlDisplayFailed";

/// @c ShowAlternativeRoutesFailed event name.
static const std::string SHOW_ALTERNATIVE_ROUTES_FAILED = "ShowAlternativeRoutesFailed";

/// Maps @c ControlDisplay directive "mode" to @c DisplayMode.
static const std::unordered_map<std::string, DisplayMode> controlDisplayStringToEnumMap = {
    {"SHOW_ROUTE_OVERVIEW", DisplayMode::SHOW_ROUTE_OVERVIEW},
    {"SHOW_DIRECTIONS_LIST", DisplayMode::SHOW_DIRECTIONS_LIST},
    {"ZOOM_IN", DisplayMode::ZOOM_IN},
    {"ZOOM_OUT", DisplayMode::ZOOM_OUT},
    {"CENTER_MAP_ON_CURRENT_LOCATION", DisplayMode::CENTER_MAP_ON_CURRENT_LOCATION},
    {"ORIENT_NORTH", DisplayMode::ORIENT_NORTH},
    {"SCROLL_NORTH", DisplayMode::SCROLL_NORTH},
    {"SCROLL_UP", DisplayMode::SCROLL_UP},
    {"SCROLL_EAST", DisplayMode::SCROLL_EAST},
    {"SCROLL_RIGHT", DisplayMode::SCROLL_RIGHT},
    {"SCROLL_SOUTH", DisplayMode::SCROLL_SOUTH},
    {"SCROLL_DOWN", DisplayMode::SCROLL_DOWN},
    {"SCROLL_WEST", DisplayMode::SCROLL_WEST},
    {"SCROLL_LEFT", DisplayMode::SCROLL_LEFT},
    {"MUTE_ROUTE_GUIDANCE", DisplayMode::MUTE_ROUTE_GUIDANCE},
    {"UNMUTE_ROUTE_GUIDANCE", DisplayMode::UNMUTE_ROUTE_GUIDANCE}};

/// Maps @c ShowAlternativeRoutes directive "inquiryType" to @c AlternativeRoutesQueryType.
static const std::unordered_map<std::string, AlternativeRoutesQueryType> alternateRouteTypeStringToEnumMap = {
    {"DEFAULT", AlternativeRoutesQueryType::DEFAULT},
    {"SHORTER_TIME", AlternativeRoutesQueryType::SHORTER_TIME},
    {"SHORTER_DISTANCE", AlternativeRoutesQueryType::SHORTER_DISTANCE}};

/**
 * Creates the Display Manager capability configuration.
 *
 * @return The Display Manager capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getDisplayManagerCapabilityConfiguration();

std::shared_ptr<DisplayManagerCapabilityAgent> DisplayManagerCapabilityAgent::create(
    std::shared_ptr<DisplayHandlerInterface> displayHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    try {
        ThrowIfNull(displayHandler, "nullDisplayHandler");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(contextManager, "nullContextManager");

        auto displayManagerCapabilityAgent = std::shared_ptr<DisplayManagerCapabilityAgent>(
            new DisplayManagerCapabilityAgent(displayHandler, exceptionSender, messageSender, contextManager));

        ThrowIfNull(displayManagerCapabilityAgent, "nullDisplayManagerCapabilityAgent");

        return displayManagerCapabilityAgent;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void DisplayManagerCapabilityAgent::handleDirectiveImmediately(
    std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) {
    handleDirective(std::make_shared<DirectiveInfo>(directive, nullptr));
}

void DisplayManagerCapabilityAgent::preHandleDirective(std::shared_ptr<DirectiveInfo> info) {
    // Intentional no-op (removeDirective() can only be called from handleDirective() and cancelDirective() functions).
}

void DisplayManagerCapabilityAgent::handleDirective(std::shared_ptr<DirectiveInfo> info) {
    try {
        ThrowIfNot(info && info->directive, "nullDirectiveInfo");

        if (info->directive->getName() == CONTROL_DISPLAY.name) {
            handleControlDisplayDirective(info);
        } else if (info->directive->getName() == SHOW_ALTERNATIVE_ROUTES.name) {
            handleShowAlternativeRoutesDirective(info);
        } else {
            handleUnknownDirective(info);
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "handleDirective").d("reason", ex.what()));
        m_executor.submit([this, info] {
            sendExceptionEncounteredAndReportFailed(
                info,
                "null directive info",
                alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        });
    }
}
void DisplayManagerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration DisplayManagerCapabilityAgent::getConfiguration() const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioVisualBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy(
        alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[CONTROL_DISPLAY] = audioVisualBlockingPolicy;
    configuration[SHOW_ALTERNATIVE_ROUTES] = audioVisualBlockingPolicy;
    return configuration;
}

DisplayManagerCapabilityAgent::DisplayManagerCapabilityAgent(
    std::shared_ptr<DisplayHandlerInterface> displayHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
        alexaClientSDK::avsCommon::utils::RequiresShutdown{"DisplayManagerCapabilityAgent"},
        m_displayHandler{displayHandler},
        m_contextManager{contextManager},
        m_messageSender{messageSender} {
    m_capabilityConfigurations.insert(getDisplayManagerCapabilityConfiguration());
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getDisplayManagerCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, ALEXA_INTERFACE_TYPE});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, DISPLAY_MANAGER_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY,
                      DISPLAY_MANAGER_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void DisplayManagerCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_displayHandler.reset();
    m_messageSender.reset();
    m_contextManager.reset();
}

void DisplayManagerCapabilityAgent::removeDirective(std::shared_ptr<DirectiveInfo> info) {
    /*
     * Check result too, to catch cases where DirectiveInfo was created locally, without a nullptr result.
     * In those cases there is no messageId to remove because no result was expected.
     */
    if (info->directive && info->result) {
        alexaClientSDK::avsCommon::avs::CapabilityAgent::removeDirective(info->directive->getMessageId());
    }
}

void DisplayManagerCapabilityAgent::setHandlingCompleted(std::shared_ptr<DirectiveInfo> info) {
    if (info && info->result) {
        info->result->setCompleted();
    }

    removeDirective(info);
}

void DisplayManagerCapabilityAgent::handleControlDisplayDirective(std::shared_ptr<DirectiveInfo> info) {
    std::string payload = info->directive->getPayload();
    rapidjson::Document json;
    rapidjson::ParseResult result = json.Parse(&payload[0]);
    if (!result) {
        AACE_ERROR(LX(TAG, "handleControlDisplayDirective")
                       .d("reason", rapidjson::GetParseError_En(result.Code()))
                       .d("messageId", info->directive->getMessageId()));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Unable to parse payload",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    if (!json.HasMember("mode")) {
        AACE_ERROR(LX(TAG, "handleControlDisplayDirective").d("reason", "missing mode value"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Missing mode value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    std::string mode = json["mode"].GetString();
    const auto& iter = controlDisplayStringToEnumMap.find(mode);
    if (iter != controlDisplayStringToEnumMap.end()) {
        aace::engine::navigation::DisplayMode control = iter->second;
        m_executor.submit([this, info, control]() {
            m_displayHandler->controlDisplay(control);
            setHandlingCompleted(info);
        });
    } else {
        AACE_ERROR(LX(TAG).d("invalidModeValue", mode));
        sendExceptionEncounteredAndReportFailed(
            info,
            "invalid mode value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
}

void DisplayManagerCapabilityAgent::handleShowAlternativeRoutesDirective(std::shared_ptr<DirectiveInfo> info) {
    std::string payload = info->directive->getPayload();
    rapidjson::Document json;
    rapidjson::ParseResult result = json.Parse(&payload[0]);
    if (!result) {
        AACE_ERROR(LX(TAG, "handleShowAlternativeRoutesDirective")
                       .d("reason", rapidjson::GetParseError_En(result.Code()))
                       .d("messageId", info->directive->getMessageId()));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Unable to parse payload",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    if (!json.HasMember("inquiryType")) {
        AACE_ERROR(LX(TAG, "handleControlDisplayDirective").d("reason", "missing inquiryType value"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "Missing inquiryType value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    std::string mode = json["inquiryType"].GetString();
    if (alternateRouteTypeStringToEnumMap.find(mode) == alternateRouteTypeStringToEnumMap.end()) {
        AACE_ERROR(LX(TAG, "handleShowAlternativeRoutesDirective").m("invalidInquiryTypeValue"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "invalid inquiryType value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    AlternativeRoutesQueryType alternateRouteType = alternateRouteTypeStringToEnumMap.find(mode)->second;

    m_executor.submit([this, info, alternateRouteType]() {
        m_displayHandler->showAlternativeRoutes(alternateRouteType);
        setHandlingCompleted(info);
    });
}

void DisplayManagerCapabilityAgent::handleUnknownDirective(std::shared_ptr<DirectiveInfo> info) {
    AACE_ERROR(LX(TAG, "handleDirectiveFailed")
                   .d("reason", "unknownDirective")
                   .d("namespace", info->directive->getNamespace())
                   .d("name", info->directive->getName()));

    m_executor.submit([this, info] {
        const std::string exceptionMessage =
            "unexpected directive " + info->directive->getNamespace() + ":" + info->directive->getName();

        sendExceptionEncounteredAndReportFailed(
            info,
            exceptionMessage,
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
    });
}

void DisplayManagerCapabilityAgent::controlDisplaySucceeded(aace::engine::navigation::DisplayMode displayMode) {
    AACE_INFO(LX(TAG).d("displayMode", displayMode));

    m_executor.submit([this, displayMode]() {
        try {
            // clang-format off
            nlohmann::json payload = {
                {"mode", displayModeToString(displayMode)}
            };
            // clang-format on
            auto event = buildJsonEventString(CONTROL_DISPLAY_SUCCEEDED, "", payload.dump());
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
            m_messageSender->sendMessage(request);
        } catch (nlohmann::json::exception& ex) {
            AACE_ERROR(LX(TAG).m("Error constructing event").d("error", ex.what()));
        }
    });
}

void DisplayManagerCapabilityAgent::controlDisplayFailed(
    aace::engine::navigation::DisplayMode displayMode,
    aace::engine::navigation::DisplayControlError error,
    const std::string& description) {
    AACE_INFO(LX(TAG).d("displayMode", displayMode).d("error", error).sensitive("description", description));

    m_executor.submit([this, displayMode, error, description]() {
        try {
            // clang-format off
            nlohmann::json payload = {
                {"mode", displayModeToString(displayMode)},
                {"code", displayControlErrorToString(error)}
            };
            // clang-format on
            if (!description.empty()) {
                payload["description"] = description;
            }
            auto event = buildJsonEventString(CONTROL_DISPLAY_FAILED, "", payload.dump());
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
            m_messageSender->sendMessage(request);
        } catch (nlohmann::json::exception& ex) {
            AACE_ERROR(LX(TAG).m("Error constructing event").d("error", ex.what()));
        }
    });
}

void DisplayManagerCapabilityAgent::showAlternativeRoutesSucceeded(
    aace::engine::navigation::AlternativeRoutesQueryType queryType,
    const aace::engine::navigation::AlternateRoute& route) {
    AACE_INFO(LX(TAG).d("queryType", queryType));
    m_executor.submit([this, queryType, route]() {
        try {
            // clang-format off
            nlohmann::json payload = {
                {"inquiryType", altRoutesQueryTypeToString(queryType)},
                {"alternateRoute", route.toJson()}
            };
            // clang-format on
            auto event = buildJsonEventString(SHOW_ALTERNATIVE_ROUTES_SUCCEEDED, "", payload.dump());
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
            m_messageSender->sendMessage(request);
        } catch (nlohmann::json::exception& ex) {
            AACE_ERROR(LX(TAG).m("Error constructing event").d("error", ex.what()));
        }
    });
}

void DisplayManagerCapabilityAgent::showAlternativeRoutesFailed(
    aace::engine::navigation::AlternativeRoutesQueryType queryType,
    aace::engine::navigation::AlternativeRoutesQueryError error,
    const std::string& description) {
    AACE_INFO(LX(TAG).d("queryType", queryType).d("error", error).sensitive("description", description));

    m_executor.submit([this, queryType, error, description]() {
        try {
            // clang-format off
            nlohmann::json payload = {
                {"inquiryType", altRoutesQueryTypeToString(queryType)},
                {"code", altRoutesErrorToString(error)}
            };
            // clang-format on
            if (!description.empty()) {
                payload["description"] = description;
            }
            auto event = buildJsonEventString(SHOW_ALTERNATIVE_ROUTES_FAILED, "", payload.dump());
            auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(event.second);
            m_messageSender->sendMessage(request);
        } catch (nlohmann::json::exception& ex) {
            AACE_ERROR(LX(TAG).m("Error constructing event").d("error", ex.what()));
        }
    });
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
DisplayManagerCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

}  // namespace navigation
}  // namespace engine
}  // namespace aace
