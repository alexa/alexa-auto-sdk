package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class Address (
    val addressLine1 : String?,
    val addressLine2 : String?,
    val addressLine3 : String?,
    val city : String,
    val stateOrRegion : String,
    val districtOrCounty : String?,
    val postalCode : String,
    val countryCode : String
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
) {
}