package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.JsonClass
import java.util.*

@JsonClass(generateAdapter = true)
data class LocalSearchDetailTemplate (
    val type : String,
    val token : String,
    val phoneNumber : String,
    val travelTime : String,
    val image : POIImage,
    val title : Title,
    val coordinate: Coordinate,
    val address : String,
    val provider: String,
    val travelDistance: String,
    val hoursOfOperation: List<HourOfOperation>?,
    val url : String,
    val rating: Rating,
)
