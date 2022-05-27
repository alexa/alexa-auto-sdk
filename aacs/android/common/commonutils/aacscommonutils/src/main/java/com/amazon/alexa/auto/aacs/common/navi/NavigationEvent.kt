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
enum class EventName {
    @Json(name = "NAVIGATION_STARTED")
    NAVIGATION_STARTED,
    @Json(name = "PREVIOUS_WAYPOINTS_SHOWN")
    PREVIOUS_WAYPOINTS_SHOWN,
    @Json(name = "PREVIOUS_NAVIGATION_STARTED")
    PREVIOUS_NAVIGATION_STARTED,
    @Json(name = "ROUTE_OVERVIEW_SHOWN")
    ROUTE_OVERVIEW_SHOWN,
    @Json(name = "DIRECTIONS_LIST_SHOWN")
    DIRECTIONS_LIST_SHOWN,
    @Json(name = "ZOOMED_IN")
    ZOOMED_IN,
    @Json(name = "ZOOMED_OUT")
    ZOOMED_OUT,
    @Json(name = "MAP_CENTERED")
    MAP_CENTERED,
    @Json(name = "ORIENTED_NORTH")
    ORIENTED_NORTH,
    @Json(name = "SCROLLED_NORTH")
    SCROLLED_NORTH,
    @Json(name = "SCROLLED_UP")
    SCROLLED_UP,
    @Json(name = "SCROLLED_EAST")
    SCROLLED_EAST,
    @Json(name = "SCROLLED_RIGHT")
    SCROLLED_RIGHT,
    @Json(name = "SCROLLED_SOUTH")
    SCROLLED_SOUTH,
    @Json(name = "SCROLLED_DOWN")
    SCROLLED_DOWN,
    @Json(name = "SCROLLED_WEST")
    SCROLLED_WEST,
    @Json(name = "SCROLLED_LEFT")
    SCROLLED_LEFT,
    @Json(name = "ROUTE_GUIDANCE_MUTED")
    ROUTE_GUIDANCE_MUTED,
    @Json(name = "ROUTE_GUIDANCE_UNMUTED")
    ROUTE_GUIDANCE_UNMUTED,
    @Json(name = "DEFAULT_ALTERNATE_ROUTES_SHOWN")
    DEFAULT_ALTERNATE_ROUTES_SHOWN,
    @Json(name = "SHORTER_TIME_ROUTES_SHOWN")
    SHORTER_TIME_ROUTES_SHOWN,
    @Json(name = "SHORTER_DISTANCE_ROUTES_SHOWN")
    SHORTER_DISTANCE_ROUTES_SHOWN,
    @Json(name = "TURN_GUIDANCE_ANNOUNCED")
    TURN_GUIDANCE_ANNOUNCED,
    @Json(name = "EXIT_GUIDANCE_ANNOUNCED")
    EXIT_GUIDANCE_ANNOUNCED,
    @Json(name = "ENTER_GUIDANCE_ANNOUNCED")
    ENTER_GUIDANCE_ANNOUNCED,
    @Json(name = "MERGE_GUIDANCE_ANNOUNCED")
    MERGE_GUIDANCE_ANNOUNCED,
    @Json(name = "LANE_GUIDANCE_ANNOUNCED")
    LANE_GUIDANCE_ANNOUNCED,
    @Json(name = "SPEED_LIMIT_REGULATION_ANNOUNCED")
    SPEED_LIMIT_REGULATION_ANNOUNCED,
    @Json(name = "CARPOOL_RULES_REGULATION_ANNOUNCED")
    CARPOOL_RULES_REGULATION_ANNOUNCED
}

@JsonClass(generateAdapter = true)
data class NavigationEvent (
    val event : EventName
)