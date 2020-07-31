/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <rapidjson/error/en.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <AVSCommon/Utils/JSON/JSONUtils.h>

#include "AACE/Engine/Navigation/DisplayManagerCapabilityAgent.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace navigation {
namespace displaymanager {

// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.DisplayManagerCapabilityAgent");

/// The namespace for the Display Manager.
static const std::string NAMESPACE{"Navigation.DisplayManager"};

/// The ControlDisplay directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName CONTROL_DISPLAY{NAMESPACE, "ControlDisplay"};

/// The ShowAlternativeRoutes directive signature.
static const alexaClientSDK::avsCommon::avs::NamespaceAndName SHOW_ALTERNATIVE_ROUTES{NAMESPACE,
                                                                                      "ShowAlternativeRoutes"};

/// Display Manager capability constants
/// Display Manager interface type
static const std::string DISPLAY_MANAGER_CAPABILITY_INTERFACE_TYPE = "AlexaInterface";
/// Display Manager  interface name
static const std::string DISPLAY_MANAGER_CAPABILITY_INTERFACE_NAME = "Navigation.DisplayManager";
/// Display Manager  interface version
static const std::string DISPLAY_MANAGER_CAPABILITY_INTERFACE_VERSION = "1.0";

// Navigation Event Strings
static const std::string CONTROL_DISPLAY_SUCCEEDED = "ControlDisplaySucceeded";
static const std::string SHOW_ALTERNATIVE_ROUTES_SUCCEEDED = "ShowAlternativeRoutesSucceeded";
static const std::string CONTROL_DISPLAY_FAILED = "ControlDisplayFailed";
static const std::string SHOW_ALTERNATIVE_ROUTES_FAILED = "ShowAlternativeRoutesFailed";

/// Map ControlDisplay string modes to @c ControlDisplay values.
static const std::unordered_map<std::string, aace::navigation::Navigation::ControlDisplay>
    controlDisplayStringToEnumMap = {
        {"SHOW_ROUTE_OVERVIEW", aace::navigation::Navigation::ControlDisplay::SHOW_ROUTE_OVERVIEW},
        {"SHOW_DIRECTIONS_LIST", aace::navigation::Navigation::ControlDisplay::SHOW_DIRECTIONS_LIST},
        {"ZOOM_IN", aace::navigation::Navigation::ControlDisplay::ZOOM_IN},
        {"ZOOM_OUT", aace::navigation::Navigation::ControlDisplay::ZOOM_OUT},
        {"CENTER_MAP_ON_CURRENT_LOCATION",
         aace::navigation::Navigation::ControlDisplay::CENTER_MAP_ON_CURRENT_LOCATION},
        {"ORIENT_NORTH", aace::navigation::Navigation::ControlDisplay::ORIENT_NORTH},
        {"SCROLL_NORTH", aace::navigation::Navigation::ControlDisplay::SCROLL_NORTH},
        {"SCROLL_UP", aace::navigation::Navigation::ControlDisplay::SCROLL_UP},
        {"SCROLL_EAST", aace::navigation::Navigation::ControlDisplay::SCROLL_EAST},
        {"SCROLL_RIGHT", aace::navigation::Navigation::ControlDisplay::SCROLL_RIGHT},
        {"SCROLL_SOUTH", aace::navigation::Navigation::ControlDisplay::SCROLL_SOUTH},
        {"SCROLL_DOWN", aace::navigation::Navigation::ControlDisplay::SCROLL_DOWN},
        {"SCROLL_WEST", aace::navigation::Navigation::ControlDisplay::SCROLL_WEST},
        {"SCROLL_LEFT", aace::navigation::Navigation::ControlDisplay::SCROLL_LEFT},
        {"MUTE_ROUTE_GUIDANCE", aace::navigation::Navigation::ControlDisplay::MUTE_ROUTE_GUIDANCE},
        {"UNMUTE_ROUTE_GUIDANCE", aace::navigation::Navigation::ControlDisplay::UNMUTE_ROUTE_GUIDANCE}};

/// Map @c AlternateRouteType string to @c AlternateRouteType values.
static const std::unordered_map<std::string, aace::navigation::Navigation::AlternateRouteType>
    alternateRouteTypeStringToEnumMap = {
        {"DEFAULT", aace::navigation::Navigation::AlternateRouteType::DEFAULT},
        {"SHORTER_TIME", aace::navigation::Navigation::AlternateRouteType::SHORTER_TIME},
        {"SHORTER_DISTANCE", aace::navigation::Navigation::AlternateRouteType::SHORTER_DISTANCE}};

/**
 * Creates the Display Manager capability configuration.
 *
 * @return The Display Manager capability configuration.
 */
static std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>
getDisplayManagerCapabilityConfiguration();

std::shared_ptr<DisplayManagerCapabilityAgent> DisplayManagerCapabilityAgent::create(
    std::shared_ptr<NavigationHandlerInterface> navigationHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    try {
        ThrowIfNull(navigationHandler, "nullNavigationHandler");
        ThrowIfNull(exceptionSender, "nullExceptionSender");
        ThrowIfNull(messageSender, "nullMessageSender");
        ThrowIfNull(contextManager, "nullContextManager");

        auto displayManagerCapabilityAgent = std::shared_ptr<DisplayManagerCapabilityAgent>(
            new DisplayManagerCapabilityAgent(navigationHandler, exceptionSender, messageSender, contextManager));

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
    }
}
void DisplayManagerCapabilityAgent::cancelDirective(std::shared_ptr<DirectiveInfo> info) {
    removeDirective(info);
}

alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration displaymanager::DisplayManagerCapabilityAgent::
    getConfiguration() const {
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration configuration;
    auto audioVisualBlockingPolicy = alexaClientSDK::avsCommon::avs::BlockingPolicy(
        alexaClientSDK::avsCommon::avs::BlockingPolicy::MEDIUMS_AUDIO_AND_VISUAL, true);
    configuration[CONTROL_DISPLAY] = audioVisualBlockingPolicy;
    configuration[SHOW_ALTERNATIVE_ROUTES] = audioVisualBlockingPolicy;
    return configuration;
}

DisplayManagerCapabilityAgent::DisplayManagerCapabilityAgent(
    std::shared_ptr<aace::engine::navigation::NavigationHandlerInterface> navigationHandler,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) :
        alexaClientSDK::avsCommon::avs::CapabilityAgent{NAMESPACE, exceptionSender},
        alexaClientSDK::avsCommon::utils::RequiresShutdown{"DisplayManagerCapabilityAgent"},
        m_navigationHandler{navigationHandler},
        m_contextManager{contextManager},
        m_messageSender{messageSender} {
    m_capabilityConfigurations.insert(getDisplayManagerCapabilityConfiguration());
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration> getDisplayManagerCapabilityConfiguration() {
    std::unordered_map<std::string, std::string> configMap;
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_TYPE_KEY, DISPLAY_MANAGER_CAPABILITY_INTERFACE_TYPE});
    configMap.insert(
        {alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_NAME_KEY, DISPLAY_MANAGER_CAPABILITY_INTERFACE_NAME});
    configMap.insert({alexaClientSDK::avsCommon::avs::CAPABILITY_INTERFACE_VERSION_KEY,
                      DISPLAY_MANAGER_CAPABILITY_INTERFACE_VERSION});

    return std::make_shared<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>(configMap);
}

void DisplayManagerCapabilityAgent::doShutdown() {
    m_executor.shutdown();
    m_navigationHandler.reset();
    m_messageSender.reset();
    m_contextManager.reset();
}

void DisplayManagerCapabilityAgent::sendExceptionEncounteredAndReportFailed(
    std::shared_ptr<DirectiveInfo> info,
    const std::string& message,
    alexaClientSDK::avsCommon::avs::ExceptionErrorType type) {
    m_exceptionEncounteredSender->sendExceptionEncountered(info->directive->getUnparsedDirective(), type, message);

    if (info && info->result) {
        info->result->setFailed(message);
    }

    removeDirective(info);
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
    if (controlDisplayStringToEnumMap.find(mode) == controlDisplayStringToEnumMap.end()) {
        AACE_ERROR(LX(TAG, "handleControlDisplayDirective").m("invalidModeValue"));
        sendExceptionEncounteredAndReportFailed(
            info,
            "invalid mode value",
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED);
        return;
    }
    aace::navigation::Navigation::ControlDisplay controlDisplay = controlDisplayStringToEnumMap.find(mode)->second;
    m_executor.submit([this, info, controlDisplay]() {
        m_navigationHandler->controlDisplay(controlDisplay);
        setHandlingCompleted(info);
    });
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
    aace::navigation::Navigation::AlternateRouteType alternateRouteType =
        alternateRouteTypeStringToEnumMap.find(mode)->second;

    m_executor.submit([this, info, alternateRouteType]() {
        m_navigationHandler->showAlternativeRoutes(alternateRouteType);
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

void DisplayManagerCapabilityAgent::navigationEvent(aace::navigation::NavigationEngineInterface::EventName event) {
    m_executor.submit([this, event] { executeNavigationEvent(event); });
}

void DisplayManagerCapabilityAgent::navigationError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    m_executor.submit([this, type, code, description] { executeNavigationError(type, code, description); });
}

void DisplayManagerCapabilityAgent::showAlternativeRoutesSucceeded(const std::string& payload) {
    m_executor.submit([this, payload] { executeShowAlternativeRoutesSucceeded(payload); });
}

void DisplayManagerCapabilityAgent::executeNavigationEvent(
    aace::navigation::NavigationEngineInterface::EventName event) {
    rapidjson::Document payload;
    std::string mode = getMode(event);
    controlDisplaySucceeded(mode);
}

void DisplayManagerCapabilityAgent::executeNavigationError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    rapidjson::Document payload;
    std::string errorCode = getErrorCode(code);
    std::string mode = getMode(type);
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::ROUTE_OVERVIEW_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::DIRECTIONS_LIST_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::ZOOM_IN_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::ZOOM_OUT_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::CENTER_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::ORIENT_NORTH_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_NORTH_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_UP_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_EAST_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_RIGHT_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_SOUTH_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_DOWN_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_WEST_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_LEFT_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::MUTED_ROUTE_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED:
            controlDisplayFailed(errorCode, description, mode);
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_TIME_ROUTES_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_DISTANCE_ROUTES_FAILED:
            showAlternativeRoutesFailed(errorCode, description, mode);
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidErrorType"));
            break;
    }
}

void displaymanager::DisplayManagerCapabilityAgent::controlDisplaySucceeded(std::string mode) {
    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("mode", rapidjson::Value(mode.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(CONTROL_DISPLAY_SUCCEEDED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(navEvent.second);
    m_messageSender->sendMessage(request);
}

void displaymanager::DisplayManagerCapabilityAgent::controlDisplayFailed(
    std::string code,
    std::string description,
    std::string mode) {
    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("mode", rapidjson::Value(mode.c_str(), allocator), allocator);
    payload.AddMember("code", rapidjson::Value(code.c_str(), allocator), allocator);
    payload.AddMember("description", rapidjson::Value(description.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(CONTROL_DISPLAY_FAILED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(navEvent.second);
    m_messageSender->sendMessage(request);
}

void DisplayManagerCapabilityAgent::showAlternativeRoutesFailed(
    std::string code,
    std::string description,
    std::string mode) {
    rapidjson::Document payload(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType& allocator = payload.GetAllocator();
    payload.AddMember("inquiryType", rapidjson::Value(mode.c_str(), allocator), allocator);
    payload.AddMember("code", rapidjson::Value(code.c_str(), allocator), allocator);
    payload.AddMember("description", rapidjson::Value(description.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    ThrowIfNot(payload.Accept(writer), "failedToWriteJsonDocument");

    auto navEvent = buildJsonEventString(SHOW_ALTERNATIVE_ROUTES_FAILED, "", buffer.GetString());
    auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(navEvent.second);
    m_messageSender->sendMessage(request);
}

void DisplayManagerCapabilityAgent::executeShowAlternativeRoutesSucceeded(const std::string& payload) {
    try {
        rapidjson::Document showAlternativeRoutesSucceededPayload(rapidjson::kObjectType);
        showAlternativeRoutesSucceededPayload.Parse(payload.c_str());

        if (showAlternativeRoutesSucceededPayload.HasParseError()) {
            rapidjson::ParseErrorCode ok = showAlternativeRoutesSucceededPayload.GetParseError();
            AACE_ERROR(LX(TAG).d("HasParseError", GetParseError_En(ok)));
            Throw("parseError");
        }

        ThrowIfNot(showAlternativeRoutesSucceededPayload.HasMember("inquiryType"), "inquiryTypeKeyMissing");

        ThrowIfNot(showAlternativeRoutesSucceededPayload.HasMember("alternateRoute"), "alternateRouteKeyMissing");

        auto alternateRoute = showAlternativeRoutesSucceededPayload["alternateRoute"].GetObject();

        ThrowIfNot(alternateRoute.HasMember("labels"), "labelsKeyMissing");

        if (alternateRoute.HasMember("savings")) {
            auto savings = alternateRoute["savings"].GetArray();
            for (int i = 0; i < savings.Size(); i++) {
                auto saving = savings[i].GetObject();
                ThrowIfNot(saving.HasMember("type"), "savingTypeMissing");
                ThrowIfNot(saving.HasMember("amount"), "savingAmountMissing");
                ThrowIfNot(saving.HasMember("unit"), "savingUnitMissing");
            }
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        ThrowIfNot(showAlternativeRoutesSucceededPayload.Accept(writer), "failedToWriteJsonDocument");

        auto navEvent = buildJsonEventString(SHOW_ALTERNATIVE_ROUTES_SUCCEEDED, "", buffer.GetString());
        auto request = std::make_shared<alexaClientSDK::avsCommon::avs::MessageRequest>(navEvent.second);
        m_messageSender->sendMessage(request);
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::string DisplayManagerCapabilityAgent::getErrorCode(aace::navigation::NavigationEngineInterface::ErrorCode code) {
    std::string errorCodeString;
    switch (code) {
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_ALLOWED:
            errorCodeString = "NOT_ALLOWED";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_SUPPORTED:
            errorCodeString = "NOT_SUPPORTED";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::ROUTE_NOT_FOUND:
            errorCodeString = "NOT_NAVIGATING";
            break;
        default:
            errorCodeString = "INTERNAL_ERROR";
            break;
    }
    return errorCodeString;
}

std::string DisplayManagerCapabilityAgent::getMode(aace::navigation::NavigationEngineInterface::EventName event) {
    std::string mode;
    switch (event) {
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_OVERVIEW_SHOWN:
            mode = "SHOW_ROUTE_OVERVIEW";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::DIRECTIONS_LIST_SHOWN:
            mode = "SHOW_DIRECTIONS_LIST";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ZOOMED_IN:
            mode = "ZOOM_IN";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ZOOMED_OUT:
            mode = "ZOOM_OUT";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::MAP_CENTERED:
            mode = "CENTER_MAP_ON_CURRENT_LOCATION";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ORIENTED_NORTH:
            mode = "ORIENT_NORTH";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_NORTH:
            mode = "SCROLL_NORTH";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_UP:
            mode = "SCROLL_UP";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_EAST:
            mode = "SCROLL_EAST";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_RIGHT:
            mode = "SCROLL_RIGHT";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_SOUTH:
            mode = "SCROLL_SOUTH";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_DOWN:
            mode = "SCROLL_DOWN";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_WEST:
            mode = "SCROLL_WEST";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_LEFT:
            mode = "SCROLL_LEFT";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_MUTED:
            mode = "MUTE_ROUTE_GUIDANCE";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_UNMUTED:
            mode = "UNMUTE_ROUTE_GUIDANCE";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::DEFAULT_ALTERNATE_ROUTES_SHOWN:
            mode = "DEFAULT";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SHORTER_TIME_ROUTES_SHOWN:
            mode = "SHORTER_TIME";
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SHORTER_DISTANCE_ROUTES_SHOWN:
            mode = "SHORTER_DISTANCE";
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidEventName"));
            break;
    }
    return mode;
}

std::string displaymanager::DisplayManagerCapabilityAgent::getMode(
    aace::navigation::NavigationEngineInterface::ErrorType type) {
    std::string mode;
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::ROUTE_OVERVIEW_FAILED:
            mode = "SHOW_ROUTE_OVERVIEW";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::DIRECTIONS_LIST_FAILED:
            mode = "SHOW_DIRECTIONS_LIST";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ZOOM_IN_FAILED:
            mode = "ZOOM_IN";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ZOOM_OUT_FAILED:
            mode = "ZOOM_OUT";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::CENTER_FAILED:
            mode = "CENTER_MAP";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ORIENT_NORTH_FAILED:
            mode = "ORIENT_NORTH";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_NORTH_FAILED:
            mode = "SCROLL_NORTH";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_UP_FAILED:
            mode = "SCROLL_UP";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_EAST_FAILED:
            mode = "SCROLL_EAST";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_RIGHT_FAILED:
            mode = "SCROLL_RIGHT";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_SOUTH_FAILED:
            mode = "SCROLL_SOUTH";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_DOWN_FAILED:
            mode = "SCROLL_DOWN";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_WEST_FAILED:
            mode = "SCROLL_WEST";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_LEFT_FAILED:
            mode = "SCROLL_LEFT";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::MUTED_ROUTE_GUIDANCE_FAILED:
            mode = "MUTE_ROUTE_GUIDANCE";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED:
            mode = "UNMUTE_ROUTE_GUIDANCE";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED:
            mode = "DEFAULT";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_TIME_ROUTES_FAILED:
            mode = "SHORTER_TIME";
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_DISTANCE_ROUTES_FAILED:
            mode = "SHORTER_DISTANCE";
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "invalidEventName"));
            break;
    }
    return mode;
}

std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
DisplayManagerCapabilityAgent::getCapabilityConfigurations() {
    return m_capabilityConfigurations;
}

}  // namespace displaymanager
}  // namespace navigation
}  // namespace engine
}  // namespace aace
