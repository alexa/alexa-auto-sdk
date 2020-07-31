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

package com.amazon.aace.navigation;

import com.amazon.aace.core.PlatformInterface;

/**
 * Navigation should be extended to handle navigation directives from the Engine.
 */
abstract public class Navigation extends PlatformInterface {
    public Navigation() {}

    public enum EventName {
        /**
         * Navigation was started
         */
        NAVIGATION_STARTED("NAVIGATION_STARTED"),
        /**
         * List of previous waypoints was displayed
         */
        PREVIOUS_WAYPOINTS_SHOWN("PREVIOUS_WAYPOINTS_SHOWN"),
        /**
         * The previous navigation route was started
         */
        PREVIOUS_NAVIGATION_STARTED("PREVIOUS_NAVIGATION_STARTED"),

        /**
         * The following event names should be sent in response to the corresponding Map Control handling
         */
        ROUTE_OVERVIEW_SHOWN("ROUTE_OVERVIEW_SHOWN"),

        DIRECTIONS_LIST_SHOWN("DIRECTIONS_LIST_SHOWN"),

        ZOOMED_IN("ZOOMED_IN"),

        ZOOMED_OUT("ZOOMED_OUT"),

        MAP_CENTERED("MAP_CENTERED"),

        ORIENTED_NORTH("ORIENTED_NORTH"),

        SCROLLED_NORTH("SCROLLED_NORTH"),

        SCROLLED_UP("SCROLLED_UP"),

        SCROLLED_EAST("SCROLLED_EAST"),

        SCROLLED_RIGHT("SCROLLED_RIGHT"),

        SCROLLED_SOUTH("SCROLLED_SOUTH"),

        SCROLLED_DOWN("SCROLLED_DOWN"),

        SCROLLED_WEST("SCROLLED_WEST"),

        SCROLLED_LEFT("SCROLLED_LEFT"),

        ROUTE_GUIDANCE_MUTED("ROUTE_GUIDANCE_MUTED"), // navigation sounds off

        ROUTE_GUIDANCE_UNMUTED("ROUTE_GUIDANCE_UNMUTED"), // navigation sounds on

        /**
         * The following event names should be sent in response to the corresponding Show Alternate Routes handling
         */
        DEFAULT_ALTERNATE_ROUTES_SHOWN("DEFAULT_ALTERNATE_ROUTES_SHOWN"),

        SHORTER_TIME_ROUTES_SHOWN("SHORTER_TIME_ROUTES_SHOWN"),

        SHORTER_DISTANCE_ROUTES_SHOWN("SHORTER_DISTANCE_ROUTES_SHOWN"),

        /*
         * The following event names should be sent in response to the corresponding Turn and Lane Guidance handling
         */
        TURN_GUIDANCE_ANNOUNCED("TURN_GUIDANCE_ANNOUNCED"),

        EXIT_GUIDANCE_ANNOUNCED("EXIT_GUIDANCE_ANNOUNCED"),

        ENTER_GUIDANCE_ANNOUNCED("ENTER_GUIDANCE_ANNOUNCED"),

        MERGE_GUIDANCE_ANNOUNCED("MERGE_GUIDANCE_ANNOUNCED"),

        LANE_GUIDANCE_ANNOUNCED("LANE_GUIDANCE_ANNOUNCED"),

        SPEED_LIMIT_REGULATION_ANNOUNCED("SPEED_LIMIT_REGULATION_ANNOUNCED"),

        CARPOOL_RULES_REGULATION_ANNOUNCED("CARPOOL_RULES_REGULATION_ANNOUNCED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private EventName(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum ErrorType {
        /**
         * Navigation failed to start
         */
        NAVIGATION_START_FAILED("NAVIGATION_START_FAILED"),
        /**
         * List of previous waypoints failed to display
         */
        SHOW_PREVIOUS_WAYPOINTS_FAILED("SHOW_PREVIOUS_WAYPOINTS_FAILED"),
        /**
         * The previous navigation route failed to start
         */
        PREVIOUS_NAVIGATION_START_FAILED("PREVIOUS_NAVIGATION_START_FAILED"),

        /**
         * The following error types should be sent in response to the corresponding Map Control handling
         */
        ROUTE_OVERVIEW_FAILED("ROUTE_OVERVIEW_FAILED"),

        DIRECTIONS_LIST_FAILED("DIRECTIONS_LIST_FAILED"),

        ZOOM_IN_FAILED("ZOOM_IN_FAILED"),

        ZOOM_OUT_FAILED("ZOOM_OUT_FAILED"),

        CENTER_FAILED("CENTER_FAILED"),

        ORIENT_NORTH_FAILED("ORIENT_NORTH_FAILED"),

        SCROLL_NORTH_FAILED("SCROLL_NORTH_FAILED"),

        SCROLL_UP_FAILED("SCROLL_UP_FAILED"),

        SCROLL_EAST_FAILED("SCROLL_EAST_FAILED"),

        SCROLL_RIGHT_FAILED("SCROLL_RIGHT_FAILED"),

        SCROLL_SOUTH_FAILED("SCROLL_SOUTH_FAILED"),

        SCROLL_DOWN_FAILED("SCROLL_DOWN_FAILED"),

        SCROLL_WEST_FAILED("SCROLL_WEST_FAILED"),

        SCROLL_LEFT_FAILED("SCROLL_LEFT_FAILED"),

        MUTED_ROUTE_GUIDANCE_FAILED("MUTED_ROUTE_GUIDANCE_FAILED"), // navigation sounds off

        UNMUTED_ROUTE_GUIDANCE_FAILED("UNMUTED_ROUTE_GUIDANCE_FAILED"), // navigation sounds on

        /**
         * The following error types should be sent in response to the corresponding Show Alternate Routes handling
         */
        DEFAULT_ALTERNATE_ROUTES_FAILED("DEFAULT_ALTERNATE_ROUTES_FAILED"),

        SHORTER_TIME_ROUTES_FAILED("SHORTER_TIME_ROUTES_FAILED"),

        SHORTER_DISTANCE_ROUTES_FAILED("SHORTER_DISTANCE_ROUTES_FAILED"),

        /**
         * The following error types should be sent in response to the corresponding Turn and Lane Guidance handling
         */
        TURN_GUIDANCE_FAILED("TURN_GUIDANCE_FAILED"),

        EXIT_GUIDANCE_FAILED("EXIT_GUIDANCE_FAILED"),

        ENTER_GUIDANCE_FAILED("ENTER_GUIDANCE_FAILED"),

        MERGE_GUIDANCE_FAILED("MERGE_GUIDANCE_FAILED"),

        LANE_GUIDANCE_FAILED("LANE_GUIDANCE_FAILED"),

        SPEED_LIMIT_REGULATION_FAILED("SPEED_LIMIT_REGULATION_FAILED"),

        CARPOOL_RULES_REGULATION_FAILED("CARPOOL_RULES_REGULATION_FAILED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ErrorType(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum ErrorCode {
        /**
         * Failure caused by an unexpected service or client implementation error
         */
        INTERNAL_SERVICE_ERROR("INTERNAL_SERVICE_ERROR"),
        /**
         * Failed because the route could not be found
         */
        ROUTE_NOT_FOUND("ROUTE_NOT_FOUND"),
        /**
         * Failed because there are no previous waypoints available
         */
        NO_PREVIOUS_WAYPOINTS("NO_PREVIOUS_WAYPOINTS"),
        /**
         * The operation requested is not supported or implemented
         */
        NOT_SUPPORTED("NOT_SUPPORTED"),
        /**
         * The operation requested can't be performed now
         */
        NOT_ALLOWED("NOT_ALLOWED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ErrorCode(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum AlternateRouteType {
        DEFAULT("DEFAULT"),

        SHORTER_TIME("SHORTER_TIME"),

        SHORTER_DISTANCE("SHORTER_DISTANCE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AlternateRouteType(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum ControlDisplay {
        SHOW_ROUTE_OVERVIEW("SHOW_ROUTE_OVERVIEW"),

        SHOW_DIRECTIONS_LIST("SHOW_DIRECTIONS_LIST"),

        ZOOM_IN("ZOOM_IN"),

        ZOOM_OUT("ZOOM_OUT"),

        CENTER_MAP_ON_CURRENT_LOCATION("CENTER_MAP_ON_CURRENT_LOCATION"),

        ORIENT_NORTH("ORIENT_NORTH"),

        SCROLL_NORTH("SCROLL_NORTH"),

        SCROLL_UP("SCROLL_UP"),

        SCROLL_EAST("SCROLL_EAST"),

        SCROLL_RIGHT("SCROLL_RIGHT"),

        SCROLL_SOUTH("SCROLL_SOUTH"),

        SCROLL_DOWN("SCROLL_DOWN"),

        SCROLL_WEST("SCROLL_WEST"),

        SCROLL_LEFT("SCROLL_LEFT"),

        MUTE_ROUTE_GUIDANCE("MUTE_ROUTE_GUIDANCE"), // navigation sounds off

        UNMUTE_ROUTE_GUIDANCE("UNMUTE_ROUTE_GUIDANCE"); // navigation sounds on

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private ControlDisplay(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum RoadRegulation {
        SPEED_LIMIT("SPEED_LIMIT"),
        CARPOOL_RULES("CARPOOL_RULES");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private RoadRegulation(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }
    ;

    /**
     * Notifies the platform implementation to cancel navigation
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean cancelNavigation() {
        return false;
    }

    /**
     * Retrieve the navigation state from the platform.
     * NOTE: You may return an empty string to default the payload to NOT_NAVIGATING
     *
     * @return the current NavigationState JSON payload
     * @code{.json})
     * "state": "{{STRING}}", //NAVIGATING or NOT_NAVIGATING
     * "waypoints": [
     *   {
     *       "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM
     *       "estimatedTimeOfArrival": {
     *           "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
     *           "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
     *        },
     *       "address": "{{STRING}}",
     *       "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
     *   },
     *   {
     *       "type": "{{STRING}}", //Type of the waypoint - SOURCE, DESTINATION or INTERIM
     *       "estimatedTimeOfArrival": {
     *           "ideal": {{STRING}}, //Expected clock time ETA based on the ideal conditions. ISO 8601 UTC format
     *           "predicted": {{STRING}} //predicted clock time ETA based on traffic conditions. ISO 8601 UTC format
     *        },
     *       "address": "{{STRING}}",
     *       "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}],
     *       "poiOfInterest": {
     *           "id": "{{STRING}}", //POI lookup Id vended from Alexa
     *           "name": "{{STRING}}", // POI Name
     *           "hoursOfOperation": [
     *            {
     *                "dayOfWeek": "{{STRING}}",
     *                "hours": [
     *                 {
     *                    "open": "{{STRING}}", // ISO-8601 time with timezone format
     *                    "close": "{{STRING}}" // ISO-8601 time with timezone format
     *                 }
     *               ],
     *               "type": "{{STRING}}" // Can be: OPEN_DURING_HOURS, OPEN_24_HOURS, etc.
     *           }
     *           ],
     *           "phoneNumber": "{{STRING}}"
     *        }
     *
     *   },
     *   ...
     * ],
     * "shapes": [
     *   [
     *       {{LATITUDE_DOUBLE}},
     *       {{LONGITUDE_DOUBLE}}
     *   ],
     *   ...
     * ]
     * @endcode
     * @li state (required) : current navigation state
     * @li waypoints (required) : list of waypoints, which can be empty
     * @li shapes (required) : list of route shapes, which can be empty or limited to 100 entries
     */
    public String getNavigationState() {
        return "";
    }

    /**
     * Notifies the platform implementation to start the navigation
     *
     * @param payload JSON data containing the destination information
     * @code{.json})
     * "transportationMode":"{{STRING}}",
     * "waypoints":[
     *    {
     *        "type":"{{STRING}}",
     *        "estimatedTimeOfArrival":{
     *            "ideal":"{{STRING}}", //ISO-8601 time format
     *            "predicted":"{{STRING}}" //ISO-8601 time format
     *        },
     *        "address": {
     *            "addressLine1": "{{STRING}}", //Address line 1
     *            "addressLine2": "{{STRING}}", //Address line 2
     *            "addressLine3": "{{STRING}}", //Address line 3
     *            "city": "{{STRING}}", //city
     *            "districtOrCounty": "{{STRING}}", //district or county
     *            "stateOrRegion": "{{STRING}}", // state or region
     *            "countryCode": "{{STRING}}", //3 letter country code
     *            "postalCode": "{{STRING}}", // postal code
     *        },
     *        "coordinate":[
     *            "{{LATITUDE_DOUBLE}}",
     *            "{{LONGITUDE_DOUBLE}}"
     *        ],
     *        "name":"{{STRING}}"
     *    },
     *    {
     *        "type":"{{STRING}}",
     *        "estimatedTimeOfArrival":{
     *            "ideal":"{{STRING}}",
     *            "predicted":"{{STRING}}"
     *        },
     *        "address":"{{STRING}}",
     *        "coordinate":[
     *            "{{LATITUDE_DOUBLE}}",
     *            "{{LONGITUDE_DOUBLE}}"
     *        ],
     *        "name":"{{STRING}}"
     *        "poiOfInterest":{
     *            "id":"{{STRING}}",
     *            "hoursOfOperation":[
     *                {
     *                "dayOfWeek":"{{STRING}}",
     *                "hours":[
     *                    {
     *                        "open":"{{STRING}}",
     *                        "close":"{{STRING}}"
     *                    }
     *                ],
     *                "type":"{{STRING}}"
     *                }
     *            ],
     *            "phoneNumber":"{{STRING}}"
     *        }
     *    }
     * ]
     * @endcode
     * @li transportationMode (optional) : Indicates the means by which to navigate to the destination (Values: BIKING,
     * DRIVING, TRANSIT, WALKING)
     * @li waypoints (required) : list of waypoints, which can be empty
     * @li waypoints.type (required) : String from Enum Type of the waypoint - SOURCE, DESTINATION or INTERIM
     * @li estimatedTimeOfArrival (optional) : Arrival time at the destination
     * @li estimatedTimeOfArrival.ideal (optional) : Expected arrival time to the destination without considering any
     * other signals such as traffic, diversions etc
     * @li estimatedTimeOfArrival.predicted (required) : Predicted arrival time to the destination based on traffic etc.
     * In the case of actual and current matching, only current ETA will be populated
     * @li address (optional) : Waypoint address
     * @li address.addressLine1 (optional) : address line 1
     * @li address.addressLine2 (optional) : address line 2
     * @li address.addressLine3 (optional) : address line 3
     * @li address.city (optional) : city of destination address
     * @li address.stateOrRegion (optional) : city of destination address
     * @li address.countryCode (optional) : 3 letter country code in ISO 3166-1 alpha-3 format
     * @li address.districtOrCounty (optional) : district or county
     * @li address.postalCode (optional) : postal code
     * @li coordinate (required) : Contains the geolocation information of the destination
     * @li coordinate.latitudeInDegrees (required) : Latitude coordinate in degrees
     * @li coordinate.longitudeInDegrees (required) : Longitute coordinate in degrees
     * @li name (optional) : waypoint name
     * @li pointOfInterest (optional) : POI information related to the waypoint
     * @li pointOfInterest.id (optional) : Lookup Id for the POIs vended by Alexa
     * @li metadata.hoursOfOperation (optional) : Hours of operation for the business
     * @li hoursOfOperation.dayOfWeek (required) : Day of the week (Values: MONDAY, TUESDAY, WEDNESDAY, THURSDAY,
     * FRIDAY, SATURDAY, SUNDAY)
     * @li hoursOfOperation.hours (required): List of opening (open) and closing (close) hours for the day. Hours are in
     * ISO-8601 time with timezone format
     * @li hoursOfOperation.type (required) : Indicates whether the location is open on this day or not (Values:
     * OPEN_DURING_HOURS, OPEN_24_HOURS, CLOSED, UNKNOWN, HOLIDAY)
     * @li metadata.phoneNumber (optional) : The phone number of the location in E.164 format
     */
    public void startNavigation(String payload) {
        return;
    }

    /*
     * Notifies the platform implementation to display list of previous waypoints
     */
    public void showPreviousWaypoints() {
        return;
    }

    /*
     * Notifies the platform implementation to start navigation to the previous waypoint
     */
    public void navigateToPreviousWaypoint() {
        return;
    }

    /*
     * Notifies the platform implementation to show alternative routes
     *
     * @param alternateRouteType The type of alternate route requested
     */
    public void showAlternativeRoutes(AlternateRouteType alternateRouteType) {
        return;
    }

    /*
     * Notifies the platform implementation to perform user interaction with the onscreen map application
     * @param [in] controlDisplay the user requested map control
     */
    public void controlDisplay(ControlDisplay controlDisplay) {
        return;
    }

    /**
     * Notifies the platform implementation to give details about a maneuver to next waypoint on the route or a
     completely different waypoint off route.
     *
     * @param payload JSON data containing the manueuver information
     * @code{.json})
     * "maneuverType": "{{STRING}}", // requested maneuver type
     * "queryTarget" : {
     *     "name": "{{STRING}}" // name of the requested location
     *     "address": { // address of requested location
     *         "addressLine1": "{{STRING}}", //Address line 1
     *         "addressLine2": "{{STRING}}", //Address line 2
     *         "addressLine3": "{{STRING}}", //Address line 3
     *         "city": "{{STRING}}", //city
     *         "districtOrCounty": "{{STRING}}", //district or county
     *         "stateOrRegion": "{{STRING}}", // state or region
     *         "countryCode": "{{STRING}}", //3 letter country code
     *         "postalCode": "{{STRING}}", // postal code
     *     },
     *     "coordinate": [{{LATITUDE_DOUBLE}},{{LONGITUDE_DOUBLE}}]
     *  }
     * @endcode
     * @li maneuverType (required) : Can take values : TURN, EXIT, ENTER, MERGE, LANE
     * @li queryTarget (optional) : Contains information about the targeted location on the route. If the location is a
     POI / user place / street address, Alexa will resolve and provide all 3 fields in the directive. If Alexa is not
     able to resolve the target, then at least one of the three fields will be present if a target was specified.
     * @li queryTarget.name (optional) : name of the requested location
     * @li queryTarget.coordinate (optional) : The geographic coordinates (lat, long) of the location as an ordered
     double list. (The order of the lat/long double list → [Latitude double , Longitude double])
     * @li queryTarget.address (optional) : Address of the requested location
     * @li queryTarget.address.addressLine1 (optional) : address line 1
     * @li queryTarget.address.addressLine2 (optional) : address line 2
     * @li queryTarget.address.addressLine3 (optional) : address line 3
     * @li queryTarget.address.city (optional) : city of destination address
     * @li queryTarget.address.stateOrRegion (optional) : city of destination address
     * @li queryTarget.address.countryCode (optional) : 3 letter country code in ISO 3166-1 alpha-3 format
     * @li queryTarget.address.districtOrCounty (optional) : district or county
     * @li queryTarget.address.postalCode (optional) : postal code
     */
    public void announceManeuver(String payload) {
        return;
    }

    /**
     * Notifies the platform implementation to give details about road regulations about the road segments that the user
     * is on
     *
     * @param roadRegulation Type of road regulation requested.(Values: SPEED_LIMIT, CARPOOL_RULES)
     */
    public void announceRoadRegulation(RoadRegulation roadRegulation) {
        return;
    }

    /**
     * Notifies the Engine of error in handling a Navigation directive.
     *
     * @param type ErrorType describing which operation failed.
     * @li ErrorType values based on directive:
     * @li startNavigation(): NAVIGATION_START_FAILED
     * @li showPreviousWaypoints(): SHOW_PREVIOUS_WAYPOINTS_FAILED
     * @li navigateToPreviousWaypoint(): PREVIOUS_NAVIGATION_START_FAILED
     * @li controlDisplay(): ROUTE_OVERVIEW_FAILED, DIRECTIONS_LIST_FAILED, ZOOM_IN_FAILED, ZOOM_OUT_FAILED,
     * CENTER_FAILED, ORIENT_NORTH_FAILED, SCROLL_NORTH_FAILED, SCROLL_UP_FAILED, SCROLL_EAST_FAILED,
     * SCROLL_RIGHT_FAILED, SCROLL_SOUTH_FAILED, SCROLL_DOWN_FAILED, SCROLL_WEST_FAILED, SCROLL_LEFT_FAILED,
     * MUTED_ROUTE_GUIDANCE_FAILED, UNMUTED_ROUTE_GUIDANCE_FAILED
     * @li showAlternativeRoutes(): DEFAULT_ALTERNATE_ROUTES_FAILED, SHORTER_TIME_ROUTES_FAILED,
     * SHORTER_DISTANCE_ROUTES_FAILED,
     * @li announceManeuver(): TURN_GUIDANCE_FAILED, EXIT_GUIDANCE_FAILED, ENTER_GUIDANCE_FAILED, MERGE_GUIDANCE_FAILED,
     * LANE_GUIDANCE_FAILED,
     * @li announceRoadRegulation(): SPEED_LIMIT_REGULATION_FAILED, CARPOOL_RULES_REGULATION_FAILED
     * @param code ErrorCode describing the type of failure. (Values: INTERNAL_SERVICE_ERROR, ROUTE_NOT_FOUND,
     *         NO_PREVIOUS_WAYPOINTS, NOT_SUPPORTED, NOT_ALLOWED)
     * @param description String providing additional information.
     */
    final protected void navigationError(ErrorType type, ErrorCode code, String description) {
        navigationError(getNativeRef(), type, code, description);
    }

    /**
     * Notifies the Engine of successful handling of a Navigation directive.
     *
     * @param [in] event EventName describing which operation was successful.
     * @li EventName values based on directive:
     * @li startNavigation(): NAVIGATION_STARTED
     * @li showPreviousWaypoints(): PREVIOUS_WAYPOINTS_SHOWN
     * @li navigateToPreviousWaypoint(): PREVIOUS_NAVIGATION_STARTED
     * @li controlDisplay(): ROUTE_OVERVIEW_SHOWN, DIRECTIONS_LIST_SHOWN, ZOOMED_IN, ZOOMED_OUT, MAP_CENTERED,
     * ORIENTED_NORTH, SCROLLED_NORTH, SCROLLED_UP, SCROLLED_EAST, SCROLLED_RIGHT, SCROLLED_SOUTH, SCROLLED_DOWN,
     * SCROLLED_WEST, SCROLLED_LEFT, ROUTE_GUIDANCE_MUTED, ROUTE_GUIDANCE_UNMUTED
     * @li showAlternativeRoutes(): DEFAULT_ALTERNATE_ROUTES_SHOWN, SHORTER_TIME_ROUTES_SHOWN,
     * SHORTER_DISTANCE_ROUTES_SHOWN
     * @li announceManeuver(): TURN_GUIDANCE_ANNOUNCED, EXIT_GUIDANCE_ANNOUNCED, ENTER_GUIDANCE_ANNOUNCED,
     * MERGE_GUIDANCE_ANNOUNCED, LANE_GUIDANCE_ANNOUNCED
     * @li announceRoadRegulation(): SPEED_LIMIT_REGULATION_ANNOUNCED, CARPOOL_RULES_REGULATION_ANNOUNCED
     */
    final protected void navigationEvent(EventName event) {
        navigationEvent(getNativeRef(), event);
    }

    /**
     * Notifies AVS of successful showing of alternative routes to the user
     *
     * @param payload JSON data containing the alternative route information
     * @code{.json})
     * "inquiryType": "{{STRING}}" // DEFAULT, SHORTER_TIME, SHORTER_DISTANCE
     * "alternateRoute":
     *   {
     *       "labels": ["{{STRING}}"],
     *       "savings": [
     *           {
     *               "type": "{{STRING}}", // DISTANCE, TIME
     *               "amount": "{{FLOAT}}",
     *               "unit": "{{STRING}}" // MINUTE, HOUR, YARD, FOOT, MILE, METER, KILOMETER
     *           }
     *       ]
     *   }
     * @endcode
     * @li inquiryType (required) : alternate route preference
     * @li alternateRoute (required) : The best route found that matches inquiryType.
     * @li labels (required) : Unique names within a route used to distinguish between alternative routes. The label
     * might contain the direction of the route when passing by the detail.
     * @li savings (optional) : List of savings achieved by the route. Savings can be in: Time and/or Distance
     * @li savings.type (required) : The type of savings
     * @li savings.amount (required) : The amount of savings achieved by the route. Alexa will use prescribed unit to
     * convert the amount of savings to improve the driver's experience, if needed.
     * @li savings.unit (required) : Measurement unit of the savings
     */
    final protected void showAlternativeRoutesSucceeded(String payload) {
        showAlternativeRoutesSucceeded(getNativeRef(), payload);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void navigationError(long nativeRef, ErrorType type, ErrorCode code, String description);
    private native void navigationEvent(long nativeRef, EventName event);
    private native void showAlternativeRoutesSucceeded(long nativeRef, String payload);
}

// END OF FILE
