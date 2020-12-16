package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.JsonClass
import java.util.*

@JsonClass(generateAdapter = true)
data class PointOfInterest (
    val priceRange : String?,
    val coordinate : Coordinate,
    val hoursOfOperation : List<HourOfOperation>,
    val address : String,
    val provider : String,
    val url : String,
    val rating : Rating,
    val travelDistance : String,
    val phoneNumber : String,
    val travelTime : String,
    val title : Title,
    val currentStatus : String?,
    val image : POIImage
)

@JsonClass(generateAdapter = true)
data class LocalSearchListTemplate (
    val type : String,
    val token : String,
    val title : String,
    val onInteraction : String,
    val pointOfInterests : List<PointOfInterest>
)