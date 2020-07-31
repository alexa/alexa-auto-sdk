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

#ifndef AACE_JNI_NAVIGATION_NAVIGATION_BINDER_H
#define AACE_JNI_NAVIGATION_NAVIGATION_BINDER_H

#include <AACE/Navigation/Navigation.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace navigation {

//
// NavigationHandler
//

class NavigationHandler : public aace::navigation::Navigation {
public:
    NavigationHandler(jobject obj);

    // aace::navigation::Navigation
    bool cancelNavigation() override;
    std::string getNavigationState() override;
    void startNavigation(const std::string& payload) override;
    void showPreviousWaypoints() override;
    void navigateToPreviousWaypoint() override;
    void showAlternativeRoutes(AlternateRouteType alternateRouteType) override;
    void controlDisplay(ControlDisplay controlDisplay) override;
    void announceManeuver(const std::string& payload) override;
    void announceRoadRegulation(RoadRegulation roadRegulation) override;

private:
    JObject m_obj;
};

//
// NavigationBinder
//

class NavigationBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    NavigationBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_navigationHandler;
    }

    std::shared_ptr<NavigationHandler> getNavigation() {
        return m_navigationHandler;
    }

private:
    std::shared_ptr<NavigationHandler> m_navigationHandler;
};

//
// JAlternateRouteType
//

class JAlternateRouteTypeConfig : public EnumConfiguration<NavigationHandler::AlternateRouteType> {
public:
    using T = NavigationHandler::AlternateRouteType;

    const char* getClassName() override {
        return "com/amazon/aace/navigation/Navigation$AlternateRouteType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::DEFAULT, "DEFAULT"}, {T::SHORTER_TIME, "SHORTER_TIME"}, {T::SHORTER_DISTANCE, "SHORTER_DISTANCE"}};
    }
};

using JAlternateRouteType = JEnum<NavigationHandler::AlternateRouteType, JAlternateRouteTypeConfig>;

//
// JControlDisplay
//

class JControlDisplayConfig : public EnumConfiguration<NavigationHandler::ControlDisplay> {
public:
    using T = NavigationHandler::ControlDisplay;

    const char* getClassName() override {
        return "com/amazon/aace/navigation/Navigation$ControlDisplay";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SHOW_ROUTE_OVERVIEW, "SHOW_ROUTE_OVERVIEW"},
                {T::SHOW_DIRECTIONS_LIST, "SHOW_DIRECTIONS_LIST"},
                {T::ZOOM_IN, "ZOOM_IN"},
                {T::ZOOM_OUT, "ZOOM_OUT"},
                {T::CENTER_MAP_ON_CURRENT_LOCATION, "CENTER_MAP_ON_CURRENT_LOCATION"},
                {T::ORIENT_NORTH, "ORIENT_NORTH"},
                {T::SCROLL_NORTH, "SCROLL_NORTH"},
                {T::SCROLL_UP, "SCROLL_UP"},
                {T::SCROLL_EAST, "SCROLL_EAST"},
                {T::SCROLL_RIGHT, "SCROLL_RIGHT"},
                {T::SCROLL_SOUTH, "SCROLL_SOUTH"},
                {T::SCROLL_DOWN, "SCROLL_DOWN"},
                {T::SCROLL_WEST, "SCROLL_WEST"},
                {T::SCROLL_LEFT, "SCROLL_LEFT"},
                {T::MUTE_ROUTE_GUIDANCE, "MUTE_ROUTE_GUIDANCE"},
                {T::UNMUTE_ROUTE_GUIDANCE, "UNMUTE_ROUTE_GUIDANCE"}};
    }
};

using JControlDisplay = JEnum<NavigationHandler::ControlDisplay, JControlDisplayConfig>;

//
// JRoadRegulation
//

class JRoadRegulationConfig : public EnumConfiguration<NavigationHandler::RoadRegulation> {
public:
    using T = NavigationHandler::RoadRegulation;

    const char* getClassName() override {
        return "com/amazon/aace/navigation/Navigation$RoadRegulation";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SPEED_LIMIT, "SPEED_LIMIT"}, {T::CARPOOL_RULES, "CARPOOL_RULES"}};
    }
};

using JRoadRegulation = JEnum<NavigationHandler::RoadRegulation, JRoadRegulationConfig>;

//
// JEventName
//

class JEventNameConfig : public EnumConfiguration<NavigationHandler::EventName> {
public:
    using T = NavigationHandler::EventName;

    const char* getClassName() override {
        return "com/amazon/aace/navigation/Navigation$EventName";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::NAVIGATION_STARTED, "NAVIGATION_STARTED"},
                {T::PREVIOUS_WAYPOINTS_SHOWN, "PREVIOUS_WAYPOINTS_SHOWN"},
                {T::PREVIOUS_NAVIGATION_STARTED, "PREVIOUS_NAVIGATION_STARTED"},
                {T::ROUTE_OVERVIEW_SHOWN, "ROUTE_OVERVIEW_SHOWN"},
                {T::DIRECTIONS_LIST_SHOWN, "DIRECTIONS_LIST_SHOWN"},
                {T::ZOOMED_IN, "ZOOMED_IN"},
                {T::ZOOMED_OUT, "ZOOMED_OUT"},
                {T::MAP_CENTERED, "MAP_CENTERED"},
                {T::ORIENTED_NORTH, "ORIENTED_NORTH"},
                {T::SCROLLED_NORTH, "SCROLLED_NORTH"},
                {T::SCROLLED_UP, "SCROLLED_UP"},
                {T::SCROLLED_EAST, "SCROLLED_EAST"},
                {T::SCROLLED_RIGHT, "SCROLLED_RIGHT"},
                {T::SCROLLED_SOUTH, "SCROLLED_SOUTH"},
                {T::SCROLLED_DOWN, "SCROLLED_DOWN"},
                {T::SCROLLED_WEST, "SCROLLED_WEST"},
                {T::SCROLLED_LEFT, "SCROLLED_LEFT"},
                {T::ROUTE_GUIDANCE_MUTED, "ROUTE_GUIDANCE_MUTED"},
                {T::ROUTE_GUIDANCE_UNMUTED, "ROUTE_GUIDANCE_UNMUTED"},
                {T::DEFAULT_ALTERNATE_ROUTES_SHOWN, "DEFAULT_ALTERNATE_ROUTES_SHOWN"},
                {T::SHORTER_TIME_ROUTES_SHOWN, "SHORTER_TIME_ROUTES_SHOWN"},
                {T::SHORTER_DISTANCE_ROUTES_SHOWN, "SHORTER_DISTANCE_ROUTES_SHOWN"},
                {T::TURN_GUIDANCE_ANNOUNCED, "TURN_GUIDANCE_ANNOUNCED"},
                {T::EXIT_GUIDANCE_ANNOUNCED, "EXIT_GUIDANCE_ANNOUNCED"},
                {T::ENTER_GUIDANCE_ANNOUNCED, "ENTER_GUIDANCE_ANNOUNCED"},
                {T::MERGE_GUIDANCE_ANNOUNCED, "MERGE_GUIDANCE_ANNOUNCED"},
                {T::LANE_GUIDANCE_ANNOUNCED, "LANE_GUIDANCE_ANNOUNCED"},
                {T::SPEED_LIMIT_REGULATION_ANNOUNCED, "SPEED_LIMIT_REGULATION_ANNOUNCED"},
                {T::CARPOOL_RULES_REGULATION_ANNOUNCED, "CARPOOL_RULES_REGULATION_ANNOUNCED"}};
    }
};

using JEventName = JEnum<NavigationHandler::EventName, JEventNameConfig>;

//
// JErrorType
//

class JErrorTypeConfig : public EnumConfiguration<NavigationHandler::ErrorType> {
public:
    using T = NavigationHandler::ErrorType;

    const char* getClassName() override {
        return "com/amazon/aace/navigation/Navigation$ErrorType";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::NAVIGATION_START_FAILED, "NAVIGATION_START_FAILED"},
                {T::SHOW_PREVIOUS_WAYPOINTS_FAILED, "SHOW_PREVIOUS_WAYPOINTS_FAILED"},
                {T::PREVIOUS_NAVIGATION_START_FAILED, "PREVIOUS_NAVIGATION_START_FAILED"},
                {T::ROUTE_OVERVIEW_FAILED, "ROUTE_OVERVIEW_FAILED"},
                {T::DIRECTIONS_LIST_FAILED, "DIRECTIONS_LIST_FAILED"},
                {T::ZOOM_IN_FAILED, "ZOOM_IN_FAILED"},
                {T::ZOOM_OUT_FAILED, "ZOOM_OUT_FAILED"},
                {T::CENTER_FAILED, "CENTER_FAILED"},
                {T::ORIENT_NORTH_FAILED, "ORIENT_NORTH_FAILED"},
                {T::SCROLL_NORTH_FAILED, "SCROLL_NORTH_FAILED"},
                {T::SCROLL_UP_FAILED, "SCROLL_UP_FAILED"},
                {T::SCROLL_EAST_FAILED, "SCROLL_EAST_FAILED"},
                {T::SCROLL_RIGHT_FAILED, "SCROLL_RIGHT_FAILED"},
                {T::SCROLL_SOUTH_FAILED, "SCROLL_SOUTH_FAILED"},
                {T::SCROLL_DOWN_FAILED, "SCROLL_DOWN_FAILED"},
                {T::SCROLL_WEST_FAILED, "SCROLL_WEST_FAILED"},
                {T::SCROLL_LEFT_FAILED, "SCROLL_LEFT_FAILED"},
                {T::MUTED_ROUTE_GUIDANCE_FAILED, "MUTED_ROUTE_GUIDANCE_FAILED"},
                {T::UNMUTED_ROUTE_GUIDANCE_FAILED, "UNMUTED_ROUTE_GUIDANCE_FAILED"},
                {T::DEFAULT_ALTERNATE_ROUTES_FAILED, "DEFAULT_ALTERNATE_ROUTES_FAILED"},
                {T::SHORTER_TIME_ROUTES_FAILED, "SHORTER_TIME_ROUTES_FAILED"},
                {T::SHORTER_DISTANCE_ROUTES_FAILED, "SHORTER_DISTANCE_ROUTES_FAILED"},
                {T::TURN_GUIDANCE_FAILED, "TURN_GUIDANCE_FAILED"},
                {T::EXIT_GUIDANCE_FAILED, "EXIT_GUIDANCE_FAILED"},
                {T::ENTER_GUIDANCE_FAILED, "ENTER_GUIDANCE_FAILED"},
                {T::MERGE_GUIDANCE_FAILED, "MERGE_GUIDANCE_FAILED"},
                {T::LANE_GUIDANCE_FAILED, "LANE_GUIDANCE_FAILED"},
                {T::SPEED_LIMIT_REGULATION_FAILED, "SPEED_LIMIT_REGULATION_FAILED"},
                {T::CARPOOL_RULES_REGULATION_FAILED, "CARPOOL_RULES_REGULATION_FAILED"}};
    }
};

using JErrorType = JEnum<NavigationHandler::ErrorType, JErrorTypeConfig>;

//
// JErrorCode
//

class JErrorCodeConfig : public EnumConfiguration<NavigationHandler::ErrorCode> {
public:
    using T = NavigationHandler::ErrorCode;

    const char* getClassName() override {
        return "com/amazon/aace/navigation/Navigation$ErrorCode";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::INTERNAL_SERVICE_ERROR, "INTERNAL_SERVICE_ERROR"},
                {T::ROUTE_NOT_FOUND, "ROUTE_NOT_FOUND"},
                {T::NO_PREVIOUS_WAYPOINTS, "NO_PREVIOUS_WAYPOINTS"},
                {T::NOT_SUPPORTED, "NOT_SUPPORTED"},
                {T::NOT_ALLOWED, "NOT_ALLOWED"}};
    }
};

using JErrorCode = JEnum<NavigationHandler::ErrorCode, JErrorCodeConfig>;

}  // namespace navigation
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NAVIGATION_NAVIGATION_BINDER_H
