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
package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = false)
enum class ControlDisplay {
    @Json(name = "SHOW_ROUTE_OVERVIEW")
    SHOW_ROUTE_OVERVIEW,
    @Json(name = "SHOW_DIRECTIONS_LIST")
    SHOW_DIRECTIONS_LIST,
    @Json(name = "ZOOM_IN")
    ZOOM_IN,
    @Json(name = "ZOOM_OUT")
    ZOOM_OUT,
    @Json(name = "CENTER_MAP_ON_CURRENT_LOCATION")
    CENTER_MAP_ON_CURRENT_LOCATION,
    @Json(name = "ORIENT_NORTH")
    ORIENT_NORTH,
    @Json(name = "SCROLL_NORTH")
    SCROLL_NORTH,
    @Json(name = "SCROLL_UP")
    SCROLL_UP,
    @Json(name = "SCROLL_EAST")
    SCROLL_EAST,
    @Json(name = "SCROLL_RIGHT")
    SCROLL_RIGHT,
    @Json(name = "SCROLL_SOUTH")
    SCROLL_SOUTH,
    @Json(name = "SCROLL_DOWN")
    SCROLL_DOWN,
    @Json(name = "SCROLL_WEST")
    SCROLL_WEST,
    @Json(name = "SCROLL_LEFT")
    SCROLL_LEFT,
    @Json(name = "MUTE_ROUTE_GUIDANCE")
    MUTE_ROUTE_GUIDANCE,
    @Json(name = "UNMUTE_ROUTE_GUIDANCE")
    UNMUTE_ROUTE_GUIDANCE
}

@JsonClass(generateAdapter = true)
data class ControlDisplayData (
    val controlDisplay : ControlDisplay
)