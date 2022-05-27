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

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class Address (
    val addressLine1 : String?,
    val addressLine2 : String?,
    val addressLine3 : String?,
    val city : String?,
    val stateOrRegion : String?,
    val districtOrCounty : String?,
    val postalCode : String?,
    val countryCode : String?
)


@JsonClass(generateAdapter = true)
data class AlexaWaypoint (
    val type : String,
    val coordinate : List<Double>,
    val address : Address,
    val name : String?
)

@JsonClass(generateAdapter = true)
data class StartNavigation (
    val waypoints : List<AlexaWaypoint>,
    val transportationMode : String
)