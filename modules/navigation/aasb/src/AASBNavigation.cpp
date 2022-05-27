/*
 * Copyright 2018-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Navigation/AASBNavigation.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Navigation/Navigation/AlternateRouteType.h>
#include <AASB/Message/Navigation/Navigation/AnnounceManeuverMessage.h>
#include <AASB/Message/Navigation/Navigation/AnnounceRoadRegulationMessage.h>
#include <AASB/Message/Navigation/Navigation/CancelNavigationMessage.h>
#include <AASB/Message/Navigation/Navigation/ControlDisplay.h>
#include <AASB/Message/Navigation/Navigation/ControlDisplayMessage.h>
#include <AASB/Message/Navigation/Navigation/ErrorCode.h>
#include <AASB/Message/Navigation/Navigation/ErrorType.h>
#include <AASB/Message/Navigation/Navigation/EventName.h>
#include <AASB/Message/Navigation/Navigation/GetNavigationStateMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigateToPreviousWaypointMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigationErrorMessage.h>
#include <AASB/Message/Navigation/Navigation/NavigationEventMessage.h>
#include <AASB/Message/Navigation/Navigation/RoadRegulation.h>
#include <AASB/Message/Navigation/Navigation/ShowAlternativeRoutesMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowAlternativeRoutesSucceededMessage.h>
#include <AASB/Message/Navigation/Navigation/ShowPreviousWaypointsMessage.h>
#include <AASB/Message/Navigation/Navigation/StartNavigationMessage.h>

#include <unordered_map>

namespace aasb {
namespace engine {
namespace navigation {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.navigation.AASBNavigation");

// aliases
using Message = aace::engine::messageBroker::Message;
using AlternateRouteType = aace::navigation::Navigation::AlternateRouteType;
using EventName = aace::navigation::Navigation::EventName;
using ErrorType = aace::navigation::Navigation::ErrorType;
using ErrorCode = aace::navigation::Navigation::ErrorCode;
using AASBAlternateRouteType = aasb::message::navigation::navigation::AlternateRouteType;
using AASBEventName = aasb::message::navigation::navigation::EventName;
using AASBErrorType = aasb::message::navigation::navigation::ErrorType;
using AASBErrorCode = aasb::message::navigation::navigation::ErrorCode;

// enum classes do not have built-in hash support in C++11
// http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2148
struct EnumHash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

static std::unordered_map<AlternateRouteType, AASBAlternateRouteType, EnumHash> g_altRouteTypeMap = {
    {AlternateRouteType::DEFAULT, AASBAlternateRouteType::DEFAULT},
    {AlternateRouteType::SHORTER_TIME, AASBAlternateRouteType::SHORTER_TIME},
    {AlternateRouteType::SHORTER_DISTANCE, AASBAlternateRouteType::SHORTER_DISTANCE}};

static std::unordered_map<AASBEventName, EventName, EnumHash> g_eventNameMap = {
    {AASBEventName::NAVIGATION_STARTED, EventName::NAVIGATION_STARTED},
    {AASBEventName::PREVIOUS_WAYPOINTS_SHOWN, EventName::PREVIOUS_WAYPOINTS_SHOWN},
    {AASBEventName::PREVIOUS_NAVIGATION_STARTED, EventName::PREVIOUS_NAVIGATION_STARTED},
    {AASBEventName::ROUTE_OVERVIEW_SHOWN, EventName::ROUTE_OVERVIEW_SHOWN},
    {AASBEventName::DIRECTIONS_LIST_SHOWN, EventName::DIRECTIONS_LIST_SHOWN},
    {AASBEventName::ZOOMED_IN, EventName::ZOOMED_IN},
    {AASBEventName::ZOOMED_OUT, EventName::ZOOMED_OUT},
    {AASBEventName::MAP_CENTERED, EventName::MAP_CENTERED},
    {AASBEventName::ORIENTED_NORTH, EventName::ORIENTED_NORTH},
    {AASBEventName::SCROLLED_NORTH, EventName::SCROLLED_NORTH},
    {AASBEventName::SCROLLED_UP, EventName::SCROLLED_UP},
    {AASBEventName::SCROLLED_EAST, EventName::SCROLLED_EAST},
    {AASBEventName::SCROLLED_RIGHT, EventName::SCROLLED_RIGHT},
    {AASBEventName::SCROLLED_SOUTH, EventName::SCROLLED_SOUTH},
    {AASBEventName::SCROLLED_DOWN, EventName::SCROLLED_DOWN},
    {AASBEventName::SCROLLED_WEST, EventName::SCROLLED_WEST},
    {AASBEventName::SCROLLED_LEFT, EventName::SCROLLED_LEFT},
    {AASBEventName::ROUTE_GUIDANCE_MUTED, EventName::ROUTE_GUIDANCE_MUTED},
    {AASBEventName::ROUTE_GUIDANCE_UNMUTED, EventName::ROUTE_GUIDANCE_UNMUTED},
    {AASBEventName::DEFAULT_ALTERNATE_ROUTES_SHOWN, EventName::DEFAULT_ALTERNATE_ROUTES_SHOWN},
    {AASBEventName::SHORTER_TIME_ROUTES_SHOWN, EventName::SHORTER_TIME_ROUTES_SHOWN},
    {AASBEventName::SHORTER_DISTANCE_ROUTES_SHOWN, EventName::SHORTER_DISTANCE_ROUTES_SHOWN},
    {AASBEventName::TURN_GUIDANCE_ANNOUNCED, EventName::TURN_GUIDANCE_ANNOUNCED},
    {AASBEventName::EXIT_GUIDANCE_ANNOUNCED, EventName::EXIT_GUIDANCE_ANNOUNCED},
    {AASBEventName::ENTER_GUIDANCE_ANNOUNCED, EventName::ENTER_GUIDANCE_ANNOUNCED},
    {AASBEventName::MERGE_GUIDANCE_ANNOUNCED, EventName::MERGE_GUIDANCE_ANNOUNCED},
    {AASBEventName::LANE_GUIDANCE_ANNOUNCED, EventName::LANE_GUIDANCE_ANNOUNCED},
    {AASBEventName::SPEED_LIMIT_REGULATION_ANNOUNCED, EventName::SPEED_LIMIT_REGULATION_ANNOUNCED},
    {AASBEventName::CARPOOL_RULES_REGULATION_ANNOUNCED, EventName::CARPOOL_RULES_REGULATION_ANNOUNCED}};

static std::unordered_map<AASBErrorType, ErrorType, EnumHash> g_errorTypeMap = {
    {AASBErrorType::NAVIGATION_START_FAILED, ErrorType::NAVIGATION_START_FAILED},
    {AASBErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED, ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED},
    {AASBErrorType::PREVIOUS_NAVIGATION_START_FAILED, ErrorType::PREVIOUS_NAVIGATION_START_FAILED},
    {AASBErrorType::ROUTE_OVERVIEW_FAILED, ErrorType::ROUTE_OVERVIEW_FAILED},
    {AASBErrorType::DIRECTIONS_LIST_FAILED, ErrorType::DIRECTIONS_LIST_FAILED},
    {AASBErrorType::ZOOM_IN_FAILED, ErrorType::ZOOM_IN_FAILED},
    {AASBErrorType::ZOOM_OUT_FAILED, ErrorType::ZOOM_OUT_FAILED},
    {AASBErrorType::CENTER_FAILED, ErrorType::CENTER_FAILED},
    {AASBErrorType::ORIENT_NORTH_FAILED, ErrorType::ORIENT_NORTH_FAILED},
    {AASBErrorType::SCROLL_NORTH_FAILED, ErrorType::SCROLL_NORTH_FAILED},
    {AASBErrorType::SCROLL_UP_FAILED, ErrorType::SCROLL_UP_FAILED},
    {AASBErrorType::SCROLL_EAST_FAILED, ErrorType::SCROLL_EAST_FAILED},
    {AASBErrorType::SCROLL_RIGHT_FAILED, ErrorType::SCROLL_RIGHT_FAILED},
    {AASBErrorType::SCROLL_SOUTH_FAILED, ErrorType::SCROLL_SOUTH_FAILED},
    {AASBErrorType::SCROLL_DOWN_FAILED, ErrorType::SCROLL_DOWN_FAILED},
    {AASBErrorType::SCROLL_WEST_FAILED, ErrorType::SCROLL_WEST_FAILED},
    {AASBErrorType::SCROLL_LEFT_FAILED, ErrorType::SCROLL_LEFT_FAILED},
    {AASBErrorType::MUTED_ROUTE_GUIDANCE_FAILED, ErrorType::MUTED_ROUTE_GUIDANCE_FAILED},
    {AASBErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED, ErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED},
    {AASBErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED, ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED},
    {AASBErrorType::SHORTER_TIME_ROUTES_FAILED, ErrorType::SHORTER_TIME_ROUTES_FAILED},
    {AASBErrorType::SHORTER_DISTANCE_ROUTES_FAILED, ErrorType::SHORTER_DISTANCE_ROUTES_FAILED},
    {AASBErrorType::TURN_GUIDANCE_FAILED, ErrorType::TURN_GUIDANCE_FAILED},
    {AASBErrorType::EXIT_GUIDANCE_FAILED, ErrorType::EXIT_GUIDANCE_FAILED},
    {AASBErrorType::ENTER_GUIDANCE_FAILED, ErrorType::ENTER_GUIDANCE_FAILED},
    {AASBErrorType::MERGE_GUIDANCE_FAILED, ErrorType::MERGE_GUIDANCE_FAILED},
    {AASBErrorType::LANE_GUIDANCE_FAILED, ErrorType::LANE_GUIDANCE_FAILED},
    {AASBErrorType::SPEED_LIMIT_REGULATION_FAILED, ErrorType::SPEED_LIMIT_REGULATION_FAILED},
    {AASBErrorType::CARPOOL_RULES_REGULATION_FAILED, ErrorType::CARPOOL_RULES_REGULATION_FAILED}};

static std::unordered_map<AASBErrorCode, ErrorCode, EnumHash> g_errorCodeMap = {
    {AASBErrorCode::INTERNAL_SERVICE_ERROR, ErrorCode::INTERNAL_SERVICE_ERROR},
    {AASBErrorCode::ROUTE_NOT_FOUND, ErrorCode::ROUTE_NOT_FOUND},
    {AASBErrorCode::NO_PREVIOUS_WAYPOINTS, ErrorCode::NO_PREVIOUS_WAYPOINTS},
    {AASBErrorCode::NOT_SUPPORTED, ErrorCode::NOT_SUPPORTED},
    {AASBErrorCode::NOT_ALLOWED, ErrorCode::NOT_ALLOWED},
    {AASBErrorCode::NOT_NAVIGATING, ErrorCode::NOT_NAVIGATING}};

std::shared_ptr<AASBNavigation> AASBNavigation::create(
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        auto handler = std::shared_ptr<AASBNavigation>(new AASBNavigation());

        // initialize the handler
        ThrowIfNot(handler->initialize(messageBroker), "initializeAASBNavigationFailed");

        return handler;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBNavigation::initialize(std::shared_ptr<aace::engine::messageBroker::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBNavigation> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::navigation::navigation::NavigationEventMessage::topic(),
            aasb::message::navigation::navigation::NavigationEventMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::navigation::navigation::NavigationEventMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    const auto& it = g_eventNameMap.find(payload.event);
                    ThrowIf(it == g_eventNameMap.end(), "Failed to convert EventName");
                    sp->navigationEvent(it->second);

                    AACE_INFO(LX(TAG, "NavigationEventMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "NavigationEventMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::navigation::navigation::NavigationErrorMessage::topic(),
            aasb::message::navigation::navigation::NavigationErrorMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::navigation::navigation::NavigationErrorMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    const auto& it1 = g_errorTypeMap.find(payload.type);
                    const auto& it2 = g_errorCodeMap.find(payload.code);
                    ThrowIf(it1 == g_errorTypeMap.end(), "Failed to convert ErrorType");
                    ThrowIf(it2 == g_errorCodeMap.end(), "Failed to convert ErrorCode");
                    sp->navigationError(it1->second, it2->second, payload.description);

                    AACE_INFO(LX(TAG, "NavigationErrorMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "NavigationErrorMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::navigation::navigation::ShowAlternativeRoutesSucceededMessage::topic(),
            aasb::message::navigation::navigation::ShowAlternativeRoutesSucceededMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");
                    aasb::message::navigation::navigation::ShowAlternativeRoutesSucceededMessage::Payload payload =
                        nlohmann::json::parse(message.payload());
                    sp->showAlternativeRoutesSucceeded(payload.payload);

                    AACE_INFO(LX(TAG, "ShowAlternativeRoutesSucceededMessage").m("MessageRouted"));
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "ShowAlternativeRoutesSucceededMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::navigation::Navigation
//

void AASBNavigation::showPreviousWaypoints() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::ShowPreviousWaypointsMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::navigateToPreviousWaypoint() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::NavigateToPreviousWaypointMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::showAlternativeRoutes(AlternateRouteType alternateRouteType) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::ShowAlternativeRoutesMessage message;
        const auto& it = g_altRouteTypeMap.find(alternateRouteType);
        ThrowIf(it == g_altRouteTypeMap.end(), "Failed to convert AlternateRouteType");

        message.payload.alternateRouteType = it->second;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::controlDisplay(ControlDisplay controlDisplay) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::ControlDisplayMessage message;

        message.payload.controlDisplay =
            static_cast<aasb::message::navigation::navigation::ControlDisplay>(controlDisplay);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

bool AASBNavigation::cancelNavigation() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::CancelNavigationMessage message;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::string AASBNavigation::getNavigationState() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::GetNavigationStateMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        if (result.valid()) {
            std::string payload = result.payload();
            aasb::message::navigation::navigation::GetNavigationStateMessageReply::Payload replyPayload =
                nlohmann::json::parse(payload);
            m_cachedNavState = replyPayload.navigationState;
        }

        AACE_INFO(LX(TAG, "GetNavigationStateMessage").m("ReplyReceived"));

        return m_cachedNavState;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return "";
    }
}

void AASBNavigation::startNavigation(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::StartNavigationMessage message;

        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::announceManeuver(const std::string& payload) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::AnnounceManeuverMessage message;

        message.payload.payload = payload;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

void AASBNavigation::announceRoadRegulation(RoadRegulation roadRegulation) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::navigation::navigation::AnnounceRoadRegulationMessage message;

        message.payload.roadRegulation =
            static_cast<aasb::message::navigation::navigation::RoadRegulation>(roadRegulation);

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace navigation
}  // namespace engine
}  // namespace aasb
