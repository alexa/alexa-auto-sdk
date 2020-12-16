package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.JsonClass
import java.util.*

@JsonClass(generateAdapter = true)
data class Title (
        val subTitle : String,
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
        val image : POIImage,
        val value : String,
        val provider : Provider,
        val reviewCount : String,

        )

@JsonClass(generateAdapter = true)
data class HourOfOperation (
        val dayOfWeek : String,
        val type : String,
        val hours : String,
)

@JsonClass(generateAdapter = true)
data class Coordinate (
        val longitudeInDegrees : Double,
        val latitudeInDegrees : Double
)
