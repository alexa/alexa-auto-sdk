package com.amazon.alexa.auto.aacs.common

import com.squareup.moshi.JsonClass
import java.util.*

@JsonClass(generateAdapter = true)
data class ImageSource (
    val size : String?,
    val url : String
)

@JsonClass(generateAdapter = true)
data class MediaSourceProviderLogo (
    val sources : List<ImageSource>
)

@JsonClass(generateAdapter = true)
data class MediaSourceProvider(
    val logo : MediaSourceProviderLogo,
    val name : String
)

@JsonClass(generateAdapter = true)
data class RenderPlayerArt (
    val sources : List<ImageSource>
)

@JsonClass(generateAdapter = true)
data class RenderPlayerContent (
    val art : RenderPlayerArt,
    val header : String?,
    val mediaLengthInMilliseconds : Long,
    val provider : MediaSourceProvider,
    val title : String?,
    val titleSubtext1 : String?,
    val titleSubtext2 : String?
)

@JsonClass(generateAdapter = true)
data class PlaybackControl (
    val enabled : Boolean,
    val name : String,
    val selected : Boolean,
    val type : String
)

@JsonClass(generateAdapter = true)
data class RenderPlayerInfoPayload (
    val content : RenderPlayerContent,
    val controls : List<PlaybackControl>
)

@JsonClass(generateAdapter = true)
data class RenderPlayerInfo (
    val payload : RenderPlayerInfoPayload,
    val audioPlayerState : String,
    val offset : Int,
    val focusState :  String
) {
    fun isControlEnabled(controlName: String) : Boolean =
        payload.controls.firstOrNull() { it.name.equals(controlName) }?.enabled ?: false

    fun getControl(controlName: String) =
        payload.controls.firstOrNull() { it.name.equals(controlName) }
}