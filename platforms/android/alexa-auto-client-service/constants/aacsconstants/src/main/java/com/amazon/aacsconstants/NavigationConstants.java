package com.amazon.aacsconstants;

public class NavigationConstants {
    public static class EventName {
        public static final String NAVIGATION_STARTED = "NAVIGATION_STARTED";
        public static final String PREVIOUS_WAYPOINTS_SHOWN = "PREVIOUS_WAYPOINTS_SHOWN";
        public static final String PREVIOUS_NAVIGATION_STARTED = "PREVIOUS_NAVIGATION_STARTED";
        public static final String ROUTE_OVERVIEW_SHOWN = "ROUTE_OVERVIEW_SHOWN";
        public static final String DIRECTIONS_LIST_SHOWN = "DIRECTIONS_LIST_SHOWN";
        public static final String ZOOMED_IN = "ZOOMED_IN";
        public static final String ZOOMED_OUT = "ZOOMED_OUT";
        public static final String MAP_CENTERED = "MAP_CENTERED";
        public static final String ORIENTED_NORTH = "ORIENTED_NORTH";
        public static final String SCROLLED_NORTH = "SCROLLED_NORTH";
        public static final String SCROLLED_UP = "SCROLLED_UP";
        public static final String SCROLLED_EAST = "SCROLLED_EAST";
        public static final String SCROLLED_RIGHT = "SCROLLED_RIGHT";
        public static final String SCROLLED_SOUTH = "SCROLLED_SOUTH";
        public static final String SCROLLED_DOWN = "SCROLLED_DOWN";
        public static final String SCROLLED_WEST = "SCROLLED_WEST";
        public static final String SCROLLED_LEFT = "SCROLLED_LEFT";
        public static final String ROUTE_GUIDANCE_MUTED = "ROUTE_GUIDANCE_MUTED";
        public static final String ROUTE_GUIDANCE_UNMUTED = "ROUTE_GUIDANCE_UNMUTED";
        public static final String DEFAULT_ALTERNATE_ROUTES_SHOWN = "DEFAULT_ALTERNATE_ROUTES_SHOWN";
        public static final String SHORTER_TIME_ROUTES_SHOWN = "SHORTER_TIME_ROUTES_SHOWN";
        public static final String SHORTER_DISTANCE_ROUTES_SHOWN = "SHORTER_DISTANCE_ROUTES_SHOWN";
        public static final String TURN_GUIDANCE_ANNOUNCED = "TURN_GUIDANCE_ANNOUNCED";
        public static final String EXIT_GUIDANCE_ANNOUNCED = "EXIT_GUIDANCE_ANNOUNCED";
        public static final String ENTER_GUIDANCE_ANNOUNCED = "ENTER_GUIDANCE_ANNOUNCED";
        public static final String MERGE_GUIDANCE_ANNOUNCED = "MERGE_GUIDANCE_ANNOUNCED";
        public static final String LANE_GUIDANCE_ANNOUNCED = "LANE_GUIDANCE_ANNOUNCED";
        public static final String SPEED_LIMIT_REGULATION_ANNOUNCED = "SPEED_LIMIT_REGULATION_ANNOUNCED";
        public static final String CARPOOL_RULES_REGULATION_ANNOUNCED = "CARPOOL_RULES_REGULATION_ANNOUNCED";
    }

    public static class ErrorType {
        public static final String NAVIGATION_START_FAILED = "NAVIGATION_START_FAILED";
        public static final String SHOW_PREVIOUS_WAYPOINTS_FAILED = "SHOW_PREVIOUS_WAYPOINTS_FAILED";
        public static final String PREVIOUS_NAVIGATION_START_FAILED = "PREVIOUS_NAVIGATION_START_FAILED";
        public static final String ROUTE_OVERVIEW_FAILED = "ROUTE_OVERVIEW_FAILED";
        public static final String DIRECTIONS_LIST_FAILED = "DIRECTIONS_LIST_FAILED";
        public static final String ZOOM_IN_FAILED = "ZOOM_IN_FAILED";
        public static final String ZOOM_OUT_FAILED = "ZOOM_OUT_FAILED";
        public static final String CENTER_FAILED = "CENTER_FAILED";
        public static final String ORIENT_NORTH_FAILED = "ORIENT_NORTH_FAILED";
        public static final String SCROLL_NORTH_FAILED = "SCROLL_NORTH_FAILED";
        public static final String SCROLL_UP_FAILED = "SCROLL_UP_FAILED";
        public static final String SCROLL_EAST_FAILED = "SCROLL_EAST_FAILED";
        public static final String SCROLL_RIGHT_FAILED = "SCROLL_RIGHT_FAILED";
        public static final String SCROLL_SOUTH_FAILED = "SCROLL_SOUTH_FAILED";
        public static final String SCROLL_DOWN_FAILED = "SCROLL_DOWN_FAILED";
        public static final String SCROLL_WEST_FAILED = "SCROLL_WEST_FAILED";
        public static final String SCROLL_LEFT_FAILED = "SCROLL_LEFT_FAILED";
        public static final String MUTED_ROUTE_GUIDANCE_FAILED = "MUTED_ROUTE_GUIDANCE_FAILED";
        public static final String UNMUTED_ROUTE_GUIDANCE_FAILED = "UNMUTED_ROUTE_GUIDANCE_FAILED";
        public static final String DEFAULT_ALTERNATE_ROUTES_FAILED = "DEFAULT_ALTERNATE_ROUTES_FAILED";
        public static final String SHORTER_TIME_ROUTES_FAILED = "SHORTER_TIME_ROUTES_FAILED";
        public static final String SHORTER_DISTANCE_ROUTES_FAILED = "SHORTER_DISTANCE_ROUTES_FAILED";
        public static final String TURN_GUIDANCE_FAILED = "TURN_GUIDANCE_FAILED";
        public static final String EXIT_GUIDANCE_FAILED = "EXIT_GUIDANCE_FAILED";
        public static final String ENTER_GUIDANCE_FAILED = "ENTER_GUIDANCE_FAILED";
        public static final String MERGE_GUIDANCE_FAILED = "MERGE_GUIDANCE_FAILED";
        public static final String LANE_GUIDANCE_FAILED = "LANE_GUIDANCE_FAILED";
        public static final String SPEED_LIMIT_REGULATION_FAILED = "SPEED_LIMIT_REGULATION_FAILED";
        public static final String CARPOOL_RULES_REGULATION_FAILED = "CARPOOL_RULES_REGULATION_FAILED";
    }

    public static class ErrorCode {
        public static final String INTERNAL_SERVICE_ERROR = "INTERNAL_SERVICE_ERROR";
        public static final String ROUTE_NOT_FOUND = "ROUTE_NOT_FOUND";
        public static final String NO_PREVIOUS_WAYPOINTS = "NO_PREVIOUS_WAYPOINTS";
        public static final String NOT_SUPPORTED = "NOT_SUPPORTED";
        public static final String NOT_ALLOWED = "NOT_ALLOWED";
    }

    public static class AlternateRouteType {
        public static final String DEFAULT = "DEFAULT";
        public static final String SHORTER_TIME = "SHORTER_TIME";
        public static final String SHORTER_DISTANCE = "SHORTER_DISTANCE";
    }

    public static class ControlDisplay {
        public static final String SHOW_ROUTE_OVERVIEW = "SHOW_ROUTE_OVERVIEW";
        public static final String SHOW_DIRECTIONS_LIST = "SHOW_DIRECTIONS_LIST";
        public static final String ZOOM_IN = "ZOOM_IN";
        public static final String ZOOM_OUT = "ZOOM_OUT";
        public static final String CENTER_MAP_ON_CURRENT_LOCATION = "CENTER_MAP_ON_CURRENT_LOCATION";
        public static final String ORIENT_NORTH = "ORIENT_NORTH";
        public static final String SCROLL_NORTH = "SCROLL_NORTH";
        public static final String SCROLL_UP = "SCROLL_UP";
        public static final String SCROLL_EAST = "SCROLL_EAST";
        public static final String SCROLL_RIGHT = "SCROLL_RIGHT";
        public static final String SCROLL_SOUTH = "SCROLL_SOUTH";
        public static final String SCROLL_DOWN = "SCROLL_DOWN";
        public static final String SCROLL_WEST = "SCROLL_WEST";
        public static final String SCROLL_LEFT = "SCROLL_LEFT";
        public static final String MUTE_ROUTE_GUIDANCE = "MUTE_ROUTE_GUIDANCE";
        public static final String UNMUTE_ROUTE_GUIDANCE = "UNMUTE_ROUTE_GUIDANCE";
    }

    public static class RoadRegulation {
        public static final String SPEED_LIMIT = "SPEED_LIMIT";
        public static final String CARPOOL_RULES = "CARPOOL_RULES";
    }
}
