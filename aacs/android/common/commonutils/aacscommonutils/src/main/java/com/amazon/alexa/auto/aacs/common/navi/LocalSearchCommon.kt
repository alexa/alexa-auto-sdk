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

import com.squareup.moshi.JsonClass
import java.util.*

@JsonClass(generateAdapter = true)
data class Title (
        val subTitle : String?,
        val mainTitle : String,
)

@JsonClass(generateAdapter = true)
data class Provider (
        val image : POIImage,
        val name : String,
)

@JsonClass(generateAdapter = true)
data class Image (
        val url : String?
)

@JsonClass(generateAdapter = true)
data class POIImage(
        val sources : List<Image>?
)

@JsonClass(generateAdapter = true)
data class Rating (
        val image : POIImage?,
        val value : String?,
        val provider : Provider?,
        val reviewCount : String?,

        )

@JsonClass(generateAdapter = true)
data class HourOfOperation (
        val dayOfWeek : String,
        val type : String?,
        val hours : String,
)

@JsonClass(generateAdapter = true)
data class Coordinate (
        val longitudeInDegrees : Double,
        val latitudeInDegrees : Double
)
