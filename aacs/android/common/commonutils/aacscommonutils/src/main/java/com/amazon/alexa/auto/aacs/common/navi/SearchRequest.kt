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
data class SearchLocation(
    val streetAddress : String?,
    val city : String?,
    val state : String?,
    val country : String?,
    val postalCode : String?,
    val location : String?
)

@JsonClass(generateAdapter = true)
data class SearchRequest(
    val requestId : String,
    val query : String,
    val queryType : String,
    val geolocation : List<Double>?,
    val locale : String,
    val numOfResults : Int?,
    val rankingStrategy : String,
    val route : List<List<Double>>?,
    val destination : List<Double>?,
    val searchLocation : SearchLocation?
)
