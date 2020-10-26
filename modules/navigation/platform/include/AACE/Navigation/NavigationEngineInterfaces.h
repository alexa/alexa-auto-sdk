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

#ifndef AAC_NAVIGATION_NAVIGATION_ENGINE_INTERFACES_H
#define AAC_NAVIGATION_NAVIGATION_ENGINE_INTERFACES_H

namespace aace {
namespace navigation {

class NavigationEngineInterface {
public:
    enum class EventName {
        /*
         * Navigation was started. Send in response to startNavigation() directive.
         */
        NAVIGATION_STARTED,

        /*
         * List of previous waypoints was displayed. Send in response to showPreviousWaypoints() directive.
         */
        PREVIOUS_WAYPOINTS_SHOWN,

        /*
         * The previous navigation route was started. Send in response to navigateToPreviousWaypoint() directive.
         */
        PREVIOUS_NAVIGATION_STARTED,

        /*
         *  Overview of route was displayed. Send in response to controlDisplay() directive.
         */
        ROUTE_OVERVIEW_SHOWN,

        /*
         * List of directions was displayed. Send in response to controlDisplay() directive.
         */
        DIRECTIONS_LIST_SHOWN,

        /*
         * Map successfully zoomed in. Send in response to controlDisplay() directive.
         */
        ZOOMED_IN,

        /*
         * Map successfully zoomed out. Send in response to controlDisplay() directive.
         */
        ZOOMED_OUT,

        /*
         * Map successfully centered. Send in response to controlDisplay() directive.
         */
        MAP_CENTERED,

        /*
         * Map successfully aligned with north up. Send in response to controlDisplay() directive.
         */
        ORIENTED_NORTH,

        /*
         * Map successfully moved in North direction. Send in response to controlDisplay() directive.
         */
        SCROLLED_NORTH,

        /*
         * Map successfully moved upwards. Send in response to controlDisplay() directive.
         */
        SCROLLED_UP,

        /*
         * Map successfully moved in East direction. Send in response to controlDisplay() directive.
         */
        SCROLLED_EAST,

        /*
         * Map successfully moved rightwards. Send in response to controlDisplay() directive.
         */
        SCROLLED_RIGHT,

        /*
         * Map successfully moved in South direction. Send in response to controlDisplay() directive.
         */
        SCROLLED_SOUTH,

        /*
         * Map successfully moved downwards. Send in response to controlDisplay() directive.
         */
        SCROLLED_DOWN,

        /*
         * Map successfully moved in West direction. Send in response to controlDisplay() directive.
         */
        SCROLLED_WEST,

        /*
         * Map successfully moved leftwards. Send in response to controlDisplay() directive.
         */
        SCROLLED_LEFT,

        /*
         * Map sounds were muted. Send in response to controlDisplay() directive.
         */
        ROUTE_GUIDANCE_MUTED,

        /*
         * Map sounds were unmuted. Send in response to controlDisplay() directive.
         */
        ROUTE_GUIDANCE_UNMUTED,

        /*
         * Default alternate routes were successfully found and displayed. Send in response to showAlternativeRoutes() directive.
         */
        DEFAULT_ALTERNATE_ROUTES_SHOWN,

        /*
         * Alternate routes with shorter times were successfully found and displayed. Send in response to showAlternativeRoutes() directive.
         */
        SHORTER_TIME_ROUTES_SHOWN,

        /*
         * Alternate routes with shorter distances were successfully found and displayed. Send in response to showAlternativeRoutes() directive.
         */
        SHORTER_DISTANCE_ROUTES_SHOWN,

        /*
         * Next turn was successfully announced. Send in response to announceManeuver() directive.
         */
        TURN_GUIDANCE_ANNOUNCED,

        /*
         * Next exit was successfully announced. Send in response to announceManeuver() directive.
         */
        EXIT_GUIDANCE_ANNOUNCED,

        /*
         * Directions for entering successfully announced. Send in response to announceManeuver() directive.
         */
        ENTER_GUIDANCE_ANNOUNCED,

        /*
         * Directions for merging successfully announced. Send in response to announceManeuver() directive.
         */
        MERGE_GUIDANCE_ANNOUNCED,

        /*
         * Lane guidance was successfully announced. Send in response to announceManeuver() directive.
         */
        LANE_GUIDANCE_ANNOUNCED,

        /*
         * Current speed limit successfully announced. Send in response to announceRoadRegulation() directive.
         */
        SPEED_LIMIT_REGULATION_ANNOUNCED,

        /*
         * Carpool status successfully announced. Send in response to announceRoadRegulation() directive.
         */
        CARPOOL_RULES_REGULATION_ANNOUNCED
    };

    enum class ErrorType {
        /**
         * Navigation failed to start. Send in response to startNavigation() directive.
         */
        NAVIGATION_START_FAILED,

        /**
         * List of previous waypoints failed to display. Send in response to showPreviousWaypoints() directive.
         */
        SHOW_PREVIOUS_WAYPOINTS_FAILED,

        /**
         * The previous navigation route failed to start. Send in response to navigateToPreviousWaypoint() directive.
         */
        PREVIOUS_NAVIGATION_START_FAILED,

        /*
         *  Overview of route was failed to display. Send in response to controlDisplay() directive.
         */
        ROUTE_OVERVIEW_FAILED,

        /*
         * List of directions was failed to display. Send in response to controlDisplay() directive.
         */
        DIRECTIONS_LIST_FAILED,

        /*
         * Map zoom-in unsuccessful. Send in response to controlDisplay() directive.
         */
        ZOOM_IN_FAILED,

        /*
         * Map zoom-out unsuccessful. Send in response to controlDisplay() directive.
         */
        ZOOM_OUT_FAILED,

        /*
         * Map centering unsuccessful. Send in response to controlDisplay() directive.
         */
        CENTER_FAILED,

        /*
         * Map alignment to north unsuccessful. Send in response to controlDisplay() directive.
         */
        ORIENT_NORTH_FAILED,

        /*
         * Moving map North was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_NORTH_FAILED,

        /*
         * Moving map upwards was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_UP_FAILED,

        /*
         * Moving map East was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_EAST_FAILED,

        /*
         * Moving map rightwards was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_RIGHT_FAILED,

        /*
         * Moving map South was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_SOUTH_FAILED,

        /*
         * Moving map downwards was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_DOWN_FAILED,

        /*
         * Moving map west was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_WEST_FAILED,

        /*
         * Moving map leftwards was unsuccessful. Send in response to controlDisplay() directive.
         */
        SCROLL_LEFT_FAILED,

        /*
         * Map sounds failed to be muted. Send in response to controlDisplay() directive.
         */
        MUTED_ROUTE_GUIDANCE_FAILED,

        /*
         * Map sounds failed to be unmuted. Send in response to controlDisplay() directive.
         */
        UNMUTED_ROUTE_GUIDANCE_FAILED,

        /*
         * Displaying default alternate routes was unsuccessful. Send in response to showAlternativeRoutes() directive.
         */
        DEFAULT_ALTERNATE_ROUTES_FAILED,

        /*
         * Displaying alternate routes with shorter times was unsuccessful. Send in response to showAlternativeRoutes() directive.
         */
        SHORTER_TIME_ROUTES_FAILED,

        /*
         * Displaying alternate routes with shorter distances was unsuccessful. Send in response to showAlternativeRoutes() directive.
         */
        SHORTER_DISTANCE_ROUTES_FAILED,

        /*
         * Next turn announcement was unsuccessful. Send in response to announceManeuver() directive.
         */
        TURN_GUIDANCE_FAILED,

        /*
         * Next exit announcement was unsuccessful. Send in response to announceManeuver() directive.
         */
        EXIT_GUIDANCE_FAILED,

        /*
         * Announcement for entering directions was unsuccessful. Send in response to announceManeuver() directive.
         */
        ENTER_GUIDANCE_FAILED,

        /*
         * Announcement for merging directions was unsuccessful. Send in response to announceManeuver() directive.
         */
        MERGE_GUIDANCE_FAILED,

        /*
         * Lane guidance announcement was unsuccessful. Send in response to announceManeuver() directive.
         */
        LANE_GUIDANCE_FAILED,

        /*
        * Current speed limit announcement was unsuccessful. Send in response to announceRoadRegulation() directive.
        */
        SPEED_LIMIT_REGULATION_FAILED,

        /*
         * Carpool status announcement was unsuccessful. Send in response to announceRoadRegulation() directive.
         */
        CARPOOL_RULES_REGULATION_FAILED
    };

    enum class ErrorCode {
        /**
         * Failure caused by an unexpected service or client implementation error
         */
        INTERNAL_SERVICE_ERROR,

        /**
         * Failed because the route could not be found
         */
        ROUTE_NOT_FOUND,

        /**
         * Failed because there are no previous waypoints available
         */
        NO_PREVIOUS_WAYPOINTS,

        /**
         * The operation requested is not supported or implemented
         */
        NOT_SUPPORTED,

        /**
         * The operation requested can't be performed now
         */
        NOT_ALLOWED
    };

    // user requested alternate route
    enum class AlternateRouteType {
        DEFAULT,

        SHORTER_TIME,

        SHORTER_DISTANCE
    };

    virtual void onNavigationEvent(EventName event) = 0;
    virtual void onNavigationError(ErrorType type, ErrorCode code, const std::string& description) = 0;
    virtual void onShowAlternativeRoutesSucceeded(const std::string& payload) = 0;
};

}  // namespace navigation
}  // namespace aace

#endif
