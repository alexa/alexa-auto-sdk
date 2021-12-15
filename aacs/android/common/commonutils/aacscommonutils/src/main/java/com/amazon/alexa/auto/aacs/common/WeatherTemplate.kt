package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.JsonClass

@JsonClass(generateAdapter = true)
data class WeatherTemplate (
        val type: String,
        val token: String,
        val lowTemperature: Temperature,
        val title: WeatherTitle,
        val currentWeatherIcon: DisplayAsset,
        val description: String,
        val highTemperature: Temperature,
        val currentWeather: String,
        val weatherForecast: List<WeatherForecast>
)

@JsonClass(generateAdapter = true)
data class Temperature (
        val value: String,
        val arrow: DisplayAsset
)

@JsonClass(generateAdapter = true)
data class DisplayAsset (
        val contentDescription: String,
        val sources: List<AssetSource>
)

@JsonClass(generateAdapter = true)
data class AssetSource (
        val widthPixels: Int,
        val darkBackgroundUrl: String,
        val size: String,
        val heightPixels: Int,
        val url: String
)

@JsonClass(generateAdapter = true)
data class WeatherTitle (
        val subTitle: String,
        val mainTitle: String
)

@JsonClass(generateAdapter = true)
data class WeatherForecast (
        val highTemperature: String,
        val date: String,
        val lowTemperature: String,
        val image: DisplayAsset,
        val day: String
)