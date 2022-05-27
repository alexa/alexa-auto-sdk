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
    val controls : List<PlaybackControl>?
)

@JsonClass(generateAdapter = true)
data class RenderPlayerInfo (
    val payload : RenderPlayerInfoPayload,
    val audioPlayerState : String,
    val offset : Int,
    val focusState :  String
) {
    fun isControlEnabled(controlName: String) : Boolean =
        payload.controls?.firstOrNull() { it.name.equals(controlName) }?.enabled ?: false

    fun getControl(controlName: String) =
        payload.controls?.firstOrNull() { it.name.equals(controlName) }
}