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

#include "AACE/Engine/Navigation/NavigationEngineImpl.h"
#include "AACE/Engine/Core/EngineMacros.h"
#include <AACE/Engine/Utils/Metrics/Metrics.h>

#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace aace {
namespace engine {
namespace navigation {

using namespace aace::engine::utils::metrics;

// String to identify log entries originating from this file.
static const std::string TAG("aace.navigation.NavigationEngineImpl");

/// Program Name for Metrics
static const std::string METRIC_PROGRAM_NAME_SUFFIX = "NavigationEngineImpl";

/// Counter Metrics for Navigation Platform APIs
static const std::string METRIC_NAVIGATION_SHOW_PREVIOUS_WAYPOINTS = "ShowPreviousWaypoints";
static const std::string METRIC_NAVIGATION_NAVIGATE_TO_PREVIOUS_WAYPOINT = "NavigateToPreviousWaypoint";
static const std::string METRIC_NAVIGATION_SHOW_ALTERNATIVE_ROUTES = "ShowAlternativeRoutes";
static const std::string METRIC_NAVIGATION_CONTROL_DISPLAY = "ControlDisplay";
static const std::string METRIC_NAVIGATION_CANCEL_NAVIGATION = "CancelNavigation";
static const std::string METRIC_NAVIGATION_GET_NAVIGATION_STATE = "GetNavigationState";
static const std::string METRIC_NAVIGATION_START_NAVIGATION = "StartNavigation";
static const std::string METRIC_NAVIGATION_ANNOUNCE_MANEUVER = "AnnounceManeuver";
static const std::string METRIC_NAVIGATION_ANNOUNCE_ROADREGULATION = "AnnounceRoadRegulation";
static const std::string METRIC_NAVIGATION_NAVIGATION_EVENT = "NavigationEvent";
static const std::string METRIC_NAVIGATION_NAVIGATION_ERROR = "NavigationError";
static const std::string METRIC_NAVIGATION_SHOW_ALTERNATIVE_ROUTES_SUCCEEDED = "ShowAlternativeRoutesSucceeded";

static const std::string ALT_ROUTE_INQUERY_TYPE_DEFAULT = "DEFAULT";
static const std::string ALT_ROUTE_INQUERY_TYPE_SHORTER_TIME = "SHORTER_TIME";
static const std::string ALT_ROUTE_INQUERY_TYPE_SHORTER_DISTANCE = "SHORTER_DISTANCE";
static const std::string ALT_ROUTE_SAVINGS_TYPE_DISTANCE = "DISTANCE";
static const std::string ALT_ROUTE_SAVINGS_TYPE_TIME = "TIME";
static const std::string ALT_ROUTE_SAVINGS_UNIT_MINUTE = "MINUTE";
static const std::string ALT_ROUTE_SAVINGS_UNIT_HOUR = "HOUR";
static const std::string ALT_ROUTE_SAVINGS_UNIT_YARD = "YARD";
static const std::string ALT_ROUTE_SAVINGS_UNIT_FOOT = "FOOT";
static const std::string ALT_ROUTE_SAVINGS_UNIT_MILE = "MILE";
static const std::string ALT_ROUTE_SAVINGS_UNIT_METER = "METER";
static const std::string ALT_ROUTE_SAVINGS_UNIT_KILOMETER = "KILOMETER";

NavigationEngineImpl::NavigationEngineImpl(
    std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
    const std::string& navigationProviderName) :
        alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG),
        m_navigationPlatformInterface(navigationPlatformInterface),
        m_navigationProviderName{navigationProviderName} {
}

bool NavigationEngineImpl::initialize(
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager) {
    try {
        m_navigationCapabilityAgent = NavigationCapabilityAgent::create(
            shared_from_this(), exceptionSender, messageSender, contextManager, m_navigationProviderName);
        ThrowIfNull(m_navigationCapabilityAgent, "couldNotCreateNavigationCapabilityAgent");

        m_navigationAssistanceCapabilityAgent = navigationassistance::NavigationAssistanceCapabilityAgent::create(
            shared_from_this(), exceptionSender, messageSender, contextManager);
        ThrowIfNull(m_navigationAssistanceCapabilityAgent, "couldNotCreateNavigationAssistanceCapabilityAgent");

        m_displayManagerCapabilityAgent =
            DisplayManagerCapabilityAgent::create(shared_from_this(), exceptionSender, messageSender, contextManager);
        ThrowIfNull(m_displayManagerCapabilityAgent, "couldNotCreateDisplayManagerCapabilityAgent");

        // register capability with the default endpoint
        capabilitiesRegistrar->withCapability(m_navigationCapabilityAgent, m_navigationCapabilityAgent);
        capabilitiesRegistrar->withCapability(m_displayManagerCapabilityAgent, m_displayManagerCapabilityAgent);
        capabilitiesRegistrar->withCapability(
            m_navigationAssistanceCapabilityAgent, m_navigationAssistanceCapabilityAgent);

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "initialize").d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<NavigationEngineImpl> NavigationEngineImpl::create(
    std::shared_ptr<aace::navigation::Navigation> navigationPlatformInterface,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
        capabilitiesRegistrar,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
    const std::string& navigationProviderName) {
    try {
        ThrowIfNull(navigationPlatformInterface, "nullNavigationPlatformInterface");
        ThrowIfNull(capabilitiesRegistrar, "nullCapabilitiesRegistrar");
        ThrowIfNull(exceptionSender, "nullPlatformInterface");
        ThrowIfNull(contextManager, "nullNavigationContextManager");

        std::shared_ptr<NavigationEngineImpl> navigationEngineImpl = std::shared_ptr<NavigationEngineImpl>(
            new NavigationEngineImpl(navigationPlatformInterface, navigationProviderName));

        ThrowIfNot(
            navigationEngineImpl->initialize(capabilitiesRegistrar, exceptionSender, messageSender, contextManager),
            "initializeNavigationEngineImplFailed");

        // set the platform engine interface reference
        navigationPlatformInterface->setEngineInterface(navigationEngineImpl);

        return navigationEngineImpl;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void NavigationEngineImpl::doShutdown() {
    if (m_navigationCapabilityAgent != nullptr) {
        m_navigationCapabilityAgent->shutdown();
        m_navigationCapabilityAgent.reset();
    }

    if (m_displayManagerCapabilityAgent != nullptr) {
        m_displayManagerCapabilityAgent->shutdown();
        m_displayManagerCapabilityAgent.reset();
    }

    if (m_navigationAssistanceCapabilityAgent != nullptr) {
        m_navigationAssistanceCapabilityAgent->shutdown();
        m_navigationAssistanceCapabilityAgent.reset();
    }

    if (m_navigationPlatformInterface != nullptr) {
        m_navigationPlatformInterface->setEngineInterface(nullptr);
    }
}

void NavigationEngineImpl::showPreviousWaypoints() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "showPreviousWaypoints", {METRIC_NAVIGATION_SHOW_PREVIOUS_WAYPOINTS});
    m_navigationPlatformInterface->showPreviousWaypoints();
}

void NavigationEngineImpl::navigateToPreviousWaypoint() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "navigateToPreviousWaypoint", {METRIC_NAVIGATION_NAVIGATE_TO_PREVIOUS_WAYPOINT});
    m_navigationPlatformInterface->navigateToPreviousWaypoint();
}

void NavigationEngineImpl::showAlternativeRoutes(aace::engine::navigation::AlternativeRoutesQueryType queryType) {
    AACE_DEBUG(LX(TAG).d("queryType", queryType));

    std::stringstream ss;
    ss << queryType;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "showAlternativeRoutes", {METRIC_NAVIGATION_SHOW_ALTERNATIVE_ROUTES, ss.str()});

    aace::navigation::NavigationEngineInterface::AlternateRouteType type;
    switch (queryType) {
        case aace::engine::navigation::AlternativeRoutesQueryType::DEFAULT:
            type = aace::navigation::NavigationEngineInterface::AlternateRouteType::DEFAULT;
            break;
        case aace::engine::navigation::AlternativeRoutesQueryType::SHORTER_TIME:
            type = aace::navigation::NavigationEngineInterface::AlternateRouteType::SHORTER_TIME;
            break;
        case aace::engine::navigation::AlternativeRoutesQueryType::SHORTER_DISTANCE:
            type = aace::navigation::NavigationEngineInterface::AlternateRouteType::SHORTER_DISTANCE;
            break;
        default:
            AACE_ERROR(LX(TAG).d("Unrecognized AlternativeRoutesQueryType", queryType));
            return;
    }
    m_navigationPlatformInterface->showAlternativeRoutes(type);
}

void NavigationEngineImpl::controlDisplay(aace::engine::navigation::DisplayMode mode) {
    AACE_DEBUG(LX(TAG).d("mode", mode));

    std::stringstream ss;
    ss << mode;
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "controlDisplay", {METRIC_NAVIGATION_CONTROL_DISPLAY, ss.str()});

    aace::navigation::Navigation::ControlDisplay type;
    switch (mode) {
        case aace::engine::navigation::DisplayMode::SHOW_ROUTE_OVERVIEW:
            type = aace::navigation::Navigation::ControlDisplay::SHOW_ROUTE_OVERVIEW;
            break;
        case aace::engine::navigation::DisplayMode::SHOW_DIRECTIONS_LIST:
            type = aace::navigation::Navigation::ControlDisplay::SHOW_DIRECTIONS_LIST;
            break;
        case aace::engine::navigation::DisplayMode::ZOOM_IN:
            type = aace::navigation::Navigation::ControlDisplay::ZOOM_IN;
            break;
        case aace::engine::navigation::DisplayMode::ZOOM_OUT:
            type = aace::navigation::Navigation::ControlDisplay::ZOOM_OUT;
            break;
        case aace::engine::navigation::DisplayMode::CENTER_MAP_ON_CURRENT_LOCATION:
            type = aace::navigation::Navigation::ControlDisplay::CENTER_MAP_ON_CURRENT_LOCATION;
            break;
        case aace::engine::navigation::DisplayMode::ORIENT_NORTH:
            type = aace::navigation::Navigation::ControlDisplay::ORIENT_NORTH;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_NORTH:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_NORTH;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_UP:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_UP;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_EAST:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_EAST;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_RIGHT:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_RIGHT;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_SOUTH:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_SOUTH;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_DOWN:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_DOWN;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_WEST:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_WEST;
            break;
        case aace::engine::navigation::DisplayMode::SCROLL_LEFT:
            type = aace::navigation::Navigation::ControlDisplay::SCROLL_LEFT;
            break;
        case aace::engine::navigation::DisplayMode::MUTE_ROUTE_GUIDANCE:
            type = aace::navigation::Navigation::ControlDisplay::MUTE_ROUTE_GUIDANCE;
            break;
        case aace::engine::navigation::DisplayMode::UNMUTE_ROUTE_GUIDANCE:
            type = aace::navigation::Navigation::ControlDisplay::UNMUTE_ROUTE_GUIDANCE;
            break;
        default:
            AACE_ERROR(LX(TAG).d("Unrecognized DisplayMode", mode));
            return;
    }
    m_navigationPlatformInterface->controlDisplay(type);
}

void NavigationEngineImpl::startNavigation(const std::string& payload) {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "startNavigation", {METRIC_NAVIGATION_START_NAVIGATION});
    m_navigationPlatformInterface->startNavigation(payload);
}

void NavigationEngineImpl::announceManeuver(const std::string& payload) {
    AACE_DEBUG(LX(TAG).sensitive("payload", payload));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "announceManeuver", {METRIC_NAVIGATION_ANNOUNCE_MANEUVER});
    m_navigationPlatformInterface->announceManeuver(payload);
}

void NavigationEngineImpl::announceRoadRegulation(aace::navigation::Navigation::RoadRegulation roadRegulation) {
    AACE_DEBUG(LX(TAG).d("roadRegulation", roadRegulation));

    std::stringstream ss;
    ss << roadRegulation;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX, "announceRoadRegulation", {METRIC_NAVIGATION_ANNOUNCE_ROADREGULATION, ss.str()});
    m_navigationPlatformInterface->announceRoadRegulation(roadRegulation);
}

void NavigationEngineImpl::cancelNavigation() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "cancelNavigation", {METRIC_NAVIGATION_CANCEL_NAVIGATION});
    m_navigationPlatformInterface->cancelNavigation();
}

std::string NavigationEngineImpl::getNavigationState() {
    AACE_DEBUG(LX(TAG));
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "getNavigationState", {METRIC_NAVIGATION_GET_NAVIGATION_STATE});
    return m_navigationPlatformInterface->getNavigationState();
}

void NavigationEngineImpl::onNavigationEvent(EventName event) {
    AACE_DEBUG(LX(TAG));
    std::stringstream ss;
    ss << event;
    emitCounterMetrics(METRIC_PROGRAM_NAME_SUFFIX, "onNavigationEvent", {METRIC_NAVIGATION_NAVIGATION_EVENT, ss.str()});
    switch (event) {
        case aace::navigation::NavigationEngineInterface::EventName::NAVIGATION_STARTED:
        case aace::navigation::NavigationEngineInterface::EventName::PREVIOUS_WAYPOINTS_SHOWN:
        case aace::navigation::NavigationEngineInterface::EventName::PREVIOUS_NAVIGATION_STARTED:
            m_navigationCapabilityAgent->navigationEvent(event);
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_OVERVIEW_SHOWN:
        case aace::navigation::NavigationEngineInterface::EventName::DIRECTIONS_LIST_SHOWN:
        case aace::navigation::NavigationEngineInterface::EventName::ZOOMED_IN:
        case aace::navigation::NavigationEngineInterface::EventName::ZOOMED_OUT:
        case aace::navigation::NavigationEngineInterface::EventName::MAP_CENTERED:
        case aace::navigation::NavigationEngineInterface::EventName::ORIENTED_NORTH:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_NORTH:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_UP:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_EAST:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_RIGHT:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_SOUTH:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_DOWN:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_WEST:
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_LEFT:
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_MUTED:
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_UNMUTED:
            handleControlDisplaySuccess(event);
            break;
        case aace::navigation::NavigationEngineInterface::EventName::TURN_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::EXIT_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::ENTER_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::MERGE_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::LANE_GUIDANCE_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::SPEED_LIMIT_REGULATION_ANNOUNCED:
        case aace::navigation::NavigationEngineInterface::EventName::CARPOOL_RULES_REGULATION_ANNOUNCED:
            m_navigationAssistanceCapabilityAgent->navigationEvent(event);
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "Invalid EventName"));
            break;
    }
}

void NavigationEngineImpl::onNavigationError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    AACE_DEBUG(LX(TAG));
    std::stringstream errorType;
    std::stringstream errorCode;
    errorType << type;
    errorCode << code;
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onNavigationError",
        {METRIC_NAVIGATION_NAVIGATION_ERROR, errorType.str(), errorCode.str()});
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::NAVIGATION_START_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::PREVIOUS_NAVIGATION_START_FAILED:
            m_navigationCapabilityAgent->navigationError(type, code, description);
            break;
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
            handleControlDisplayError(type, code, description);
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_TIME_ROUTES_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_DISTANCE_ROUTES_FAILED:
            handleShowAlternativeRoutesError(type, code, description);
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::TURN_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::EXIT_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::ENTER_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::MERGE_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::LANE_GUIDANCE_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::SPEED_LIMIT_REGULATION_FAILED:
        case aace::navigation::NavigationEngineInterface::ErrorType::CARPOOL_RULES_REGULATION_FAILED:
            m_navigationAssistanceCapabilityAgent->navigationError(type, code, description);
            break;
        default:
            AACE_ERROR(LX(TAG).d("reason", "Invalid Navigation ErrorType"));
            break;
    }
}

void NavigationEngineImpl::onShowAlternativeRoutesSucceeded(const std::string& payload) {
    emitCounterMetrics(
        METRIC_PROGRAM_NAME_SUFFIX,
        "onShowAlternativeRoutesSucceeded",
        {METRIC_NAVIGATION_SHOW_ALTERNATIVE_ROUTES_SUCCEEDED});

    aace::engine::navigation::AlternativeRoutesQueryType queryType;
    std::vector<std::string> labels;
    std::vector<aace::engine::navigation::RouteSavings> savingsList;
    try {
        nlohmann::json payloadJson = nlohmann::json::parse(payload);

        std::string inquiryType = payloadJson.at("inquiryType");
        if (inquiryType == ALT_ROUTE_INQUERY_TYPE_DEFAULT) {
            queryType = aace::engine::navigation::AlternativeRoutesQueryType::DEFAULT;
        } else if (inquiryType == ALT_ROUTE_INQUERY_TYPE_SHORTER_TIME) {
            queryType = aace::engine::navigation::AlternativeRoutesQueryType::SHORTER_TIME;
        } else if (inquiryType == ALT_ROUTE_INQUERY_TYPE_SHORTER_DISTANCE) {
            queryType = aace::engine::navigation::AlternativeRoutesQueryType::SHORTER_DISTANCE;
        } else {
            AACE_ERROR(LX(TAG).d("Invalid inquiryType", inquiryType));
            return;
        }

        nlohmann::json alternateRouteJson = payloadJson.at("alternateRoute");
        labels = alternateRouteJson.at("labels").get<std::vector<std::string>>();
        if (labels.empty()) {
            AACE_ERROR(LX(TAG).m("alternateRoute.labels must be nonempty"));
            return;
        }

        if (alternateRouteJson.contains("savings")) {
            nlohmann::json savingsJson = alternateRouteJson.at("savings");
            for (auto& saving : savingsJson.items()) {
                double amount = saving.value().at("amount");

                aace::engine::navigation::RouteSavingsType savingsType;
                const std::string& savingsTypeStr = saving.value().at("type");
                if (savingsTypeStr == ALT_ROUTE_SAVINGS_TYPE_DISTANCE) {
                    savingsType = aace::engine::navigation::RouteSavingsType::DISTANCE;
                } else if (savingsTypeStr == ALT_ROUTE_SAVINGS_TYPE_TIME) {
                    savingsType = aace::engine::navigation::RouteSavingsType::TIME;
                } else {
                    AACE_ERROR(LX(TAG).d("Invalid savings.type", savingsTypeStr));
                    return;
                }

                aace::engine::navigation::SavingsUnit savingsUnit;
                const std::string& savingsUnitStr = saving.value().at("unit");
                if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_MINUTE) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::MINUTE;
                } else if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_HOUR) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::HOUR;
                } else if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_FOOT) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::FOOT;
                } else if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_YARD) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::YARD;
                } else if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_MILE) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::MILE;
                } else if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_METER) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::METER;
                } else if (savingsUnitStr == ALT_ROUTE_SAVINGS_UNIT_KILOMETER) {
                    savingsUnit = aace::engine::navigation::SavingsUnit::KILOMETER;
                } else {
                    AACE_ERROR(LX(TAG).d("Invalid savings.unit", savingsUnitStr));
                    return;
                }
                savingsList.push_back({savingsType, amount, savingsUnit});
            }
            if (savingsList.empty()) {
                // non-fatal error since the savings list is optional
                AACE_WARN(LX(TAG).m("alternateRoute.savings must have at least one value if present"));
            }
        }
        m_displayManagerCapabilityAgent->showAlternativeRoutesSucceeded(queryType, {labels, savingsList});
    } catch (nlohmann::json::exception& ex) {
        AACE_ERROR(LX(TAG).d("exception", ex.what()));
        return;
    }
}

void NavigationEngineImpl::handleControlDisplaySuccess(aace::navigation::NavigationEngineInterface::EventName event) {
    aace::engine::navigation::DisplayMode mode;
    switch (event) {
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_OVERVIEW_SHOWN:
            mode = aace::engine::navigation::DisplayMode::SHOW_ROUTE_OVERVIEW;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::DIRECTIONS_LIST_SHOWN:
            mode = aace::engine::navigation::DisplayMode::SHOW_DIRECTIONS_LIST;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ZOOMED_IN:
            mode = aace::engine::navigation::DisplayMode::ZOOM_IN;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ZOOMED_OUT:
            mode = aace::engine::navigation::DisplayMode::ZOOM_OUT;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::MAP_CENTERED:
            mode = aace::engine::navigation::DisplayMode::CENTER_MAP_ON_CURRENT_LOCATION;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ORIENTED_NORTH:
            mode = aace::engine::navigation::DisplayMode::ORIENT_NORTH;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_NORTH:
            mode = aace::engine::navigation::DisplayMode::SCROLL_NORTH;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_UP:
            mode = aace::engine::navigation::DisplayMode::SCROLL_UP;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_EAST:
            mode = aace::engine::navigation::DisplayMode::SCROLL_EAST;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_RIGHT:
            mode = aace::engine::navigation::DisplayMode::SCROLL_RIGHT;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_SOUTH:
            mode = aace::engine::navigation::DisplayMode::SCROLL_SOUTH;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_DOWN:
            mode = aace::engine::navigation::DisplayMode::SCROLL_DOWN;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_WEST:
            mode = aace::engine::navigation::DisplayMode::SCROLL_WEST;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::SCROLLED_LEFT:
            mode = aace::engine::navigation::DisplayMode::SCROLL_LEFT;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_MUTED:
            mode = aace::engine::navigation::DisplayMode::MUTE_ROUTE_GUIDANCE;
            break;
        case aace::navigation::NavigationEngineInterface::EventName::ROUTE_GUIDANCE_UNMUTED:
            mode = aace::engine::navigation::DisplayMode::UNMUTE_ROUTE_GUIDANCE;
            break;
        default:
            AACE_ERROR(LX(TAG).m("Received invalid type. Ignoring.").d("type", event));
            return;
    }
    m_displayManagerCapabilityAgent->controlDisplaySucceeded(mode);
}

void NavigationEngineImpl::handleControlDisplayError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    aace::engine::navigation::DisplayMode mode;
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::ROUTE_OVERVIEW_FAILED:
            mode = aace::engine::navigation::DisplayMode::SHOW_ROUTE_OVERVIEW;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::DIRECTIONS_LIST_FAILED:
            mode = aace::engine::navigation::DisplayMode::SHOW_DIRECTIONS_LIST;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ZOOM_IN_FAILED:
            mode = aace::engine::navigation::DisplayMode::ZOOM_IN;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ZOOM_OUT_FAILED:
            mode = aace::engine::navigation::DisplayMode::ZOOM_OUT;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::CENTER_FAILED:
            mode = aace::engine::navigation::DisplayMode::CENTER_MAP_ON_CURRENT_LOCATION;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::ORIENT_NORTH_FAILED:
            mode = aace::engine::navigation::DisplayMode::ORIENT_NORTH;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_NORTH_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_NORTH;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_UP_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_UP;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_EAST_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_EAST;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_RIGHT_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_RIGHT;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_SOUTH_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_SOUTH;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_DOWN_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_DOWN;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_WEST_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_WEST;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SCROLL_LEFT_FAILED:
            mode = aace::engine::navigation::DisplayMode::SCROLL_LEFT;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::MUTED_ROUTE_GUIDANCE_FAILED:
            mode = aace::engine::navigation::DisplayMode::MUTE_ROUTE_GUIDANCE;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED:
            mode = aace::engine::navigation::DisplayMode::UNMUTE_ROUTE_GUIDANCE;
            break;
        default:
            AACE_ERROR(LX(TAG).m("Received invalid type. Ignoring.").d("type", type));
            return;
    }
    aace::engine::navigation::DisplayControlError displayControlError;
    switch (code) {
        case aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR:
            displayControlError = aace::engine::navigation::DisplayControlError::INTERNAL_ERROR;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_SUPPORTED:
            displayControlError = aace::engine::navigation::DisplayControlError::NOT_SUPPORTED;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_ALLOWED:
            displayControlError = aace::engine::navigation::DisplayControlError::NOT_ALLOWED;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::ROUTE_NOT_FOUND:
            // Fall through to NOT_NAVIGATING.
            // "ROUTE_NOT_FOUND" is not a valid code for display control error, but "NOT_NAVIGATING" is an ok
            // replacement since the meaning in this context is the closest match.
            AACE_WARN(LX(TAG).m("ROUTE_NOT_FOUND is not a valid error code for ControlDisplay. Use NOT_NAVIGATING."));
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_NAVIGATING:
            displayControlError = aace::engine::navigation::DisplayControlError::NOT_NAVIGATING;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NO_PREVIOUS_WAYPOINTS:
            // fall through since this error code does not make sense as a display control error
        default:
            AACE_ERROR(LX(TAG).m("Received invalid error code. Ignoring.").d("code", code));
            return;
    }
    m_displayManagerCapabilityAgent->controlDisplayFailed(mode, displayControlError, description);
}

void NavigationEngineImpl::handleShowAlternativeRoutesError(
    aace::navigation::NavigationEngineInterface::ErrorType type,
    aace::navigation::NavigationEngineInterface::ErrorCode code,
    const std::string& description) {
    aace::engine::navigation::AlternativeRoutesQueryType queryType;
    switch (type) {
        case aace::navigation::NavigationEngineInterface::ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED:
            queryType = aace::engine::navigation::AlternativeRoutesQueryType::DEFAULT;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_TIME_ROUTES_FAILED:
            queryType = aace::engine::navigation::AlternativeRoutesQueryType::SHORTER_TIME;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorType::SHORTER_DISTANCE_ROUTES_FAILED:
            queryType = aace::engine::navigation::AlternativeRoutesQueryType::SHORTER_DISTANCE;
            break;
        default:
            AACE_ERROR(LX(TAG).m("Received invalid type. Ignoring.").d("type", type));
            return;
    }
    aace::engine::navigation::AlternativeRoutesQueryError queryError;
    switch (code) {
        case aace::navigation::NavigationEngineInterface::ErrorCode::INTERNAL_SERVICE_ERROR:
            queryError = aace::engine::navigation::AlternativeRoutesQueryError::INTERNAL_ERROR;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_ALLOWED:
            // Fall through to NOT_SUPPORTED.
            // "NOT_ALLOWED" is not a valid error code for alternate routes error, but "NOT_SUPPORTED" is an ok
            // replacement since its meaning in this context is the closest match.
            AACE_WARN(LX(TAG).m("NOT_ALLOWED is not a valid error code for ShowAlternativeRoutes. Use NOT_SUPPORTED."));
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_SUPPORTED:
            queryError = aace::engine::navigation::AlternativeRoutesQueryError::NOT_SUPPORTED;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::NOT_NAVIGATING:
            queryError = aace::engine::navigation::AlternativeRoutesQueryError::NOT_NAVIGATING;
            break;
        case aace::navigation::NavigationEngineInterface::ErrorCode::ROUTE_NOT_FOUND:
            queryError = aace::engine::navigation::AlternativeRoutesQueryError::NO_ROUTES_FOUND;
            break;
        default:
            AACE_ERROR(LX(TAG).m("Received invalid error code. Ignoring.").d("code", code));
            return;
    }
    m_displayManagerCapabilityAgent->showAlternativeRoutesFailed(queryType, queryError, description);
}

}  // namespace navigation
}  // namespace engine
}  // namespace aace
