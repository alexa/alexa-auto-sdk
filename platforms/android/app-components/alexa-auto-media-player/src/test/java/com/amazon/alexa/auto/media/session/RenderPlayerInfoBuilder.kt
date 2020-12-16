package com.amazon.alexa.auto.media.session

import com.amazon.alexa.auto.aacs.common.MediaSourceProvider
import com.amazon.alexa.auto.aacs.common.MediaSourceProviderLogo
import com.amazon.alexa.auto.aacs.common.PlaybackControl
import com.amazon.alexa.auto.aacs.common.RenderPlayerArt
import com.amazon.alexa.auto.aacs.common.RenderPlayerContent
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfoPayload

/**
 * Builder class to help building @c RenderPlayerInfo.
 */
class RenderPlayerInfoBuilder {
    private var mRenderPlayerArt : RenderPlayerArt? = null
    private var mHeader : String? = null
    private var mMediaLength : Long? = null
    private var mMediaSourceProvider : MediaSourceProvider? = null
    private var mTitle : String? = null
    private var mSubText1 : String? = null
    private var mSubText2 : String? = null
    private val mPlaybackControls : MutableList<PlaybackControl> = arrayListOf()

    fun withArt(art : RenderPlayerArt): RenderPlayerInfoBuilder {
        mRenderPlayerArt = art
        return this
    }

    fun withHeader(header : String): RenderPlayerInfoBuilder {
        mHeader = header
        return this
    }

    fun withMediaLength(mediaLength : Long) : RenderPlayerInfoBuilder {
        mMediaLength = mediaLength
        return this
    }

    fun withMediaSourceProvider(sourceProvider : MediaSourceProvider) : RenderPlayerInfoBuilder {
        mMediaSourceProvider = sourceProvider
        return this
    }

    fun withTitle(title : String) : RenderPlayerInfoBuilder {
        mTitle = title
        return this
    }

    fun withSubText1(subText1 : String) : RenderPlayerInfoBuilder {
        mSubText1 = subText1
        return this
    }

    fun withSubText2(subText2 : String) : RenderPlayerInfoBuilder {
        mSubText2 = subText2
        return this
    }

    fun addPlaybackControl(control : PlaybackControl) : RenderPlayerInfoBuilder {
        mPlaybackControls.add(control)
        return this
    }

    fun build(): RenderPlayerInfo {
        return RenderPlayerInfo(
            // All non null fields are supplied with defaults.
            RenderPlayerInfoPayload(
                RenderPlayerContent(
                    mRenderPlayerArt ?: RenderPlayerArt(emptyList()),
                    mHeader,
                    mMediaLength ?: MediaMetadataProverTest.MEDIA_LENGTH,
                    mMediaSourceProvider ?: MediaSourceProvider(
                        MediaSourceProviderLogo(listOf()),
                        MediaMetadataProvider.MEDIA_PROVIDER_AMAZON_MUSIC
                    ),
                    mTitle,
                    mSubText1,
                    mSubText2
                ),
                mPlaybackControls
            ),
            "audio-player-state",
            0,
            "focus-state"
        )
    }
}