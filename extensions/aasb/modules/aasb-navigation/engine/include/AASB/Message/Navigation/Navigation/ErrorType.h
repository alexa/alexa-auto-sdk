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

/*********************************************************
**********************************************************
**********************************************************

THIS FILE IS AUTOGENERATED. DO NOT EDIT

**********************************************************
**********************************************************
*********************************************************/

#ifndef NAVIGATION_ERRORTYPE_H
#define NAVIGATION_ERRORTYPE_H

#include <string>
#include <vector>

#include <unordered_map>
#include <AACE/Engine/Utils/UUID/UUID.h>
#include <nlohmann/json.hpp>

namespace aasb {
namespace message {
namespace navigation {
namespace navigation {

//Enum Definition
enum class ErrorType {
    NAVIGATION_START_FAILED,
    SHOW_PREVIOUS_WAYPOINTS_FAILED,
    PREVIOUS_NAVIGATION_START_FAILED,
    ROUTE_OVERVIEW_FAILED,
    DIRECTIONS_LIST_FAILED,
    ZOOM_IN_FAILED,
    ZOOM_OUT_FAILED,
    CENTER_FAILED,
    ORIENT_NORTH_FAILED,
    SCROLL_NORTH_FAILED,
    SCROLL_UP_FAILED,
    SCROLL_EAST_FAILED,
    SCROLL_RIGHT_FAILED,
    SCROLL_SOUTH_FAILED,
    SCROLL_DOWN_FAILED,
    SCROLL_WEST_FAILED,
    SCROLL_LEFT_FAILED,
    MUTED_ROUTE_GUIDANCE_FAILED,
    UNMUTED_ROUTE_GUIDANCE_FAILED,
    DEFAULT_ALTERNATE_ROUTES_FAILED,
    SHORTER_TIME_ROUTES_FAILED,
    SHORTER_DISTANCE_ROUTES_FAILED,
    TURN_GUIDANCE_FAILED,
    EXIT_GUIDANCE_FAILED,
    ENTER_GUIDANCE_FAILED,
    MERGE_GUIDANCE_FAILED,
    LANE_GUIDANCE_FAILED,
    SPEED_LIMIT_REGULATION_FAILED,
    CARPOOL_RULES_REGULATION_FAILED,
};

inline std::string toString(ErrorType enumValue) {
    switch (enumValue) {
        case (ErrorType::NAVIGATION_START_FAILED):
            return "NAVIGATION_START_FAILED";
        case (ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED):
            return "SHOW_PREVIOUS_WAYPOINTS_FAILED";
        case (ErrorType::PREVIOUS_NAVIGATION_START_FAILED):
            return "PREVIOUS_NAVIGATION_START_FAILED";
        case (ErrorType::ROUTE_OVERVIEW_FAILED):
            return "ROUTE_OVERVIEW_FAILED";
        case (ErrorType::DIRECTIONS_LIST_FAILED):
            return "DIRECTIONS_LIST_FAILED";
        case (ErrorType::ZOOM_IN_FAILED):
            return "ZOOM_IN_FAILED";
        case (ErrorType::ZOOM_OUT_FAILED):
            return "ZOOM_OUT_FAILED";
        case (ErrorType::CENTER_FAILED):
            return "CENTER_FAILED";
        case (ErrorType::ORIENT_NORTH_FAILED):
            return "ORIENT_NORTH_FAILED";
        case (ErrorType::SCROLL_NORTH_FAILED):
            return "SCROLL_NORTH_FAILED";
        case (ErrorType::SCROLL_UP_FAILED):
            return "SCROLL_UP_FAILED";
        case (ErrorType::SCROLL_EAST_FAILED):
            return "SCROLL_EAST_FAILED";
        case (ErrorType::SCROLL_RIGHT_FAILED):
            return "SCROLL_RIGHT_FAILED";
        case (ErrorType::SCROLL_SOUTH_FAILED):
            return "SCROLL_SOUTH_FAILED";
        case (ErrorType::SCROLL_DOWN_FAILED):
            return "SCROLL_DOWN_FAILED";
        case (ErrorType::SCROLL_WEST_FAILED):
            return "SCROLL_WEST_FAILED";
        case (ErrorType::SCROLL_LEFT_FAILED):
            return "SCROLL_LEFT_FAILED";
        case (ErrorType::MUTED_ROUTE_GUIDANCE_FAILED):
            return "MUTED_ROUTE_GUIDANCE_FAILED";
        case (ErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED):
            return "UNMUTED_ROUTE_GUIDANCE_FAILED";
        case (ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED):
            return "DEFAULT_ALTERNATE_ROUTES_FAILED";
        case (ErrorType::SHORTER_TIME_ROUTES_FAILED):
            return "SHORTER_TIME_ROUTES_FAILED";
        case (ErrorType::SHORTER_DISTANCE_ROUTES_FAILED):
            return "SHORTER_DISTANCE_ROUTES_FAILED";
        case (ErrorType::TURN_GUIDANCE_FAILED):
            return "TURN_GUIDANCE_FAILED";
        case (ErrorType::EXIT_GUIDANCE_FAILED):
            return "EXIT_GUIDANCE_FAILED";
        case (ErrorType::ENTER_GUIDANCE_FAILED):
            return "ENTER_GUIDANCE_FAILED";
        case (ErrorType::MERGE_GUIDANCE_FAILED):
            return "MERGE_GUIDANCE_FAILED";
        case (ErrorType::LANE_GUIDANCE_FAILED):
            return "LANE_GUIDANCE_FAILED";
        case (ErrorType::SPEED_LIMIT_REGULATION_FAILED):
            return "SPEED_LIMIT_REGULATION_FAILED";
        case (ErrorType::CARPOOL_RULES_REGULATION_FAILED):
            return "CARPOOL_RULES_REGULATION_FAILED";
    }
    throw std::runtime_error("invalidErrorTypeType");
}

inline ErrorType toErrorType(const std::string& stringValue) {
    static std::unordered_map<std::string, ErrorType> map = {
        {"NAVIGATION_START_FAILED", ErrorType::NAVIGATION_START_FAILED},
        {"SHOW_PREVIOUS_WAYPOINTS_FAILED", ErrorType::SHOW_PREVIOUS_WAYPOINTS_FAILED},
        {"PREVIOUS_NAVIGATION_START_FAILED", ErrorType::PREVIOUS_NAVIGATION_START_FAILED},
        {"ROUTE_OVERVIEW_FAILED", ErrorType::ROUTE_OVERVIEW_FAILED},
        {"DIRECTIONS_LIST_FAILED", ErrorType::DIRECTIONS_LIST_FAILED},
        {"ZOOM_IN_FAILED", ErrorType::ZOOM_IN_FAILED},
        {"ZOOM_OUT_FAILED", ErrorType::ZOOM_OUT_FAILED},
        {"CENTER_FAILED", ErrorType::CENTER_FAILED},
        {"ORIENT_NORTH_FAILED", ErrorType::ORIENT_NORTH_FAILED},
        {"SCROLL_NORTH_FAILED", ErrorType::SCROLL_NORTH_FAILED},
        {"SCROLL_UP_FAILED", ErrorType::SCROLL_UP_FAILED},
        {"SCROLL_EAST_FAILED", ErrorType::SCROLL_EAST_FAILED},
        {"SCROLL_RIGHT_FAILED", ErrorType::SCROLL_RIGHT_FAILED},
        {"SCROLL_SOUTH_FAILED", ErrorType::SCROLL_SOUTH_FAILED},
        {"SCROLL_DOWN_FAILED", ErrorType::SCROLL_DOWN_FAILED},
        {"SCROLL_WEST_FAILED", ErrorType::SCROLL_WEST_FAILED},
        {"SCROLL_LEFT_FAILED", ErrorType::SCROLL_LEFT_FAILED},
        {"MUTED_ROUTE_GUIDANCE_FAILED", ErrorType::MUTED_ROUTE_GUIDANCE_FAILED},
        {"UNMUTED_ROUTE_GUIDANCE_FAILED", ErrorType::UNMUTED_ROUTE_GUIDANCE_FAILED},
        {"DEFAULT_ALTERNATE_ROUTES_FAILED", ErrorType::DEFAULT_ALTERNATE_ROUTES_FAILED},
        {"SHORTER_TIME_ROUTES_FAILED", ErrorType::SHORTER_TIME_ROUTES_FAILED},
        {"SHORTER_DISTANCE_ROUTES_FAILED", ErrorType::SHORTER_DISTANCE_ROUTES_FAILED},
        {"TURN_GUIDANCE_FAILED", ErrorType::TURN_GUIDANCE_FAILED},
        {"EXIT_GUIDANCE_FAILED", ErrorType::EXIT_GUIDANCE_FAILED},
        {"ENTER_GUIDANCE_FAILED", ErrorType::ENTER_GUIDANCE_FAILED},
        {"MERGE_GUIDANCE_FAILED", ErrorType::MERGE_GUIDANCE_FAILED},
        {"LANE_GUIDANCE_FAILED", ErrorType::LANE_GUIDANCE_FAILED},
        {"SPEED_LIMIT_REGULATION_FAILED", ErrorType::SPEED_LIMIT_REGULATION_FAILED},
        {"CARPOOL_RULES_REGULATION_FAILED", ErrorType::CARPOOL_RULES_REGULATION_FAILED},
    };

    auto search = map.find(stringValue);
    if (search != map.end()) {
        return search->second;
    }
    throw std::runtime_error("invalidErrorTypeType");
}

inline void to_json(nlohmann::json& j, const ErrorType& c) {
    j = toString(c);
}

inline void from_json(const nlohmann::json& j, ErrorType& c) {
    c = toErrorType(j);
}

}  // namespace navigation
}  // namespace navigation
}  // namespace message
}  // namespace aasb

#endif  // NAVIGATION_ERRORTYPE_H
