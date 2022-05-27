/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.aacs.common.navi

import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = false)
public enum class ErrorType {
    @Json(name = "NAVIGATION_START_FAILED")
    NAVIGATION_START_FAILED,
    @Json(name = "SHOW_PREVIOUS_WAYPOINTS_FAILED")
    SHOW_PREVIOUS_WAYPOINTS_FAILED,
    @Json(name = "PREVIOUS_NAVIGATION_START_FAILED")
    PREVIOUS_NAVIGATION_START_FAILED,
    @Json(name = "ROUTE_OVERVIEW_FAILED")
    ROUTE_OVERVIEW_FAILED,
    @Json(name = "DIRECTIONS_LIST_FAILED")
    DIRECTIONS_LIST_FAILED,
    @Json(name = "ZOOM_IN_FAILED")
    ZOOM_IN_FAILED,
    @Json(name = "ZOOM_OUT_FAILED")
    ZOOM_OUT_FAILED,
    @Json(name = "CENTER_FAILED")
    CENTER_FAILED,
    @Json(name = "ORIENT_NORTH_FAILED")
    ORIENT_NORTH_FAILED,
    @Json(name = "SCROLL_NORTH_FAILED")
    SCROLL_NORTH_FAILED,
    @Json(name = "SCROLL_UP_FAILED")
    SCROLL_UP_FAILED,
    @Json(name = "SCROLL_EAST_FAILED")
    SCROLL_EAST_FAILED,
    @Json(name = "SCROLL_RIGHT_FAILED")
    SCROLL_RIGHT_FAILED,
    @Json(name = "SCROLL_SOUTH_FAILED")
    SCROLL_SOUTH_FAILED,
    @Json(name = "SCROLL_DOWN_FAILED")
    SCROLL_DOWN_FAILED,
    @Json(name = "SCROLL_WEST_FAILED")
    SCROLL_WEST_FAILED,
    @Json(name = "SCROLL_LEFT_FAILED")
    SCROLL_LEFT_FAILED,
    @Json(name = "MUTED_ROUTE_GUIDANCE_FAILED")
    MUTED_ROUTE_GUIDANCE_FAILED,
    @Json(name = "UNMUTED_ROUTE_GUIDANCE_FAILED")
    UNMUTED_ROUTE_GUIDANCE_FAILED,
    @Json(name = "DEFAULT_ALTERNATE_ROUTES_FAILED")
    DEFAULT_ALTERNATE_ROUTES_FAILED,
    @Json(name = "SHORTER_TIME_ROUTES_FAILED")
    SHORTER_TIME_ROUTES_FAILED,
    @Json(name = "SHORTER_DISTANCE_ROUTES_FAILED")
    SHORTER_DISTANCE_ROUTES_FAILED,
    @Json(name = "TURN_GUIDANCE_FAILED")
    TURN_GUIDANCE_FAILED,
    @Json(name = "EXIT_GUIDANCE_FAILED")
    EXIT_GUIDANCE_FAILED,
    @Json(name = "ENTER_GUIDANCE_FAILED")
    ENTER_GUIDANCE_FAILED,
    @Json(name = "MERGE_GUIDANCE_FAILED")
    MERGE_GUIDANCE_FAILED,
    @Json(name = "LANE_GUIDANCE_FAILED")
    LANE_GUIDANCE_FAILED,
    @Json(name = "SPEED_LIMIT_REGULATION_FAILED")
    SPEED_LIMIT_REGULATION_FAILED,
    @Json(name = "CARPOOL_RULES_REGULATION_FAILED")
    CARPOOL_RULES_REGULATION_FAILED
}

@JsonClass(generateAdapter = false)
public enum class ErrorCode {
    @Json(name = "INTERNAL_SERVICE_ERROR")
    INTERNAL_SERVICE_ERROR,
    @Json(name = "ROUTE_NOT_FOUND")
    ROUTE_NOT_FOUND,
    @Json(name = "NO_PREVIOUS_WAYPOINTS")
    NO_PREVIOUS_WAYPOINTS,
    @Json(name = "NOT_SUPPORTED")
    NOT_SUPPORTED,
    @Json(name = "NOT_ALLOWED")
    NOT_ALLOWED
}

@JsonClass(generateAdapter = true)
data class NavigationError (
    val type : ErrorType,
    val code : ErrorCode,
    val description : String
)