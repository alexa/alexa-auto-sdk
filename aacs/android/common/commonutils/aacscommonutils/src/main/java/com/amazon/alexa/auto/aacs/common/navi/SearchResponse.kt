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

import com.amazon.alexa.auto.aacs.common.Coordinate
import com.squareup.moshi.Json
import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = false)
enum class SearchResponseStatus {
    @Json(name = "SUCCESS")
    SUCCESS,
    @Json(name = "FAIL")
    FAIL
}

@JsonClass(generateAdapter = true)
data class Result (
    val type : String,
    val score : Double,
    val confidence : String,
    val provider : String,
    val id : String,
    val address : Address
)

@JsonClass(generateAdapter = true)
data class TravelDistance (
    val straightLineDistanceInMeters: Double,
    val navigationDistanceInMeters: Double
)

@JsonClass(generateAdapter = true)
data class TravelTime (
    val ideal: String,
    val predicted : String
)

@JsonClass(generateAdapter = true)
data class RouteInfo (
    val travelDistance : TravelDistance,
    val travelTime : TravelTime
)

@JsonClass(generateAdapter = true)
data class Hours(
    val open : String,
    val close : String
)

@JsonClass(generateAdapter = true)
data class HoursOfOperation (
    val dayOfWeek: String,
    val hours: List<Hours>,
    val type : String
)

@JsonClass(generateAdapter = true)
data class POI(
    val name: String,
    val categories: List<String>,
    val phoneNumber: String,
    val website: String,
    val rating: Double,
    val hoursOfOperation : HoursOfOperation,
    val imageFilePath : String
)

@JsonClass(generateAdapter = true)
data class SearchResponseData (
    val results : List<Result>,
    val totalNumResults : Int,
    val appliedRankingStrategy : String,
    val navigationPosition : Coordinate,
    val routingInfo : RouteInfo,
    val poi : POI
)

@JsonClass(generateAdapter = true)
data class SearchResponse(
    val requestId : String,
    val status : SearchResponseStatus,
    val data : SearchResponseData,
)

@JsonClass(generateAdapter = true)
data class SearchError(
    val errorCode : String,
    val errorMessage : String
)

@JsonClass(generateAdapter = true)
data class SearchErrorResponse(
    val requestId : String,
    val status : SearchResponseStatus,
    val error : SearchError
)

