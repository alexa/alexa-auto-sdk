package com.amazon.alexa.auto.media.session

import android.support.v4.media.RatingCompat
import com.amazon.aacsconstants.PlaybackConstants
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages
import com.google.android.exoplayer2.Player
import org.junit.Before
import org.junit.Test
import org.mockito.Mock
import org.mockito.Mockito.anyBoolean
import org.mockito.Mockito.eq
import org.mockito.Mockito.times
import org.mockito.Mockito.verify
import org.mockito.MockitoAnnotations

class PlaybackControllerTest {
    lateinit var mClassUnderTest: PlaybackController

    @Mock lateinit var mMockPlaybackControlMessages: PlaybackControlMessages
    @Mock lateinit var mMockPlayer: Player

    @Before
    fun setup() {
        MockitoAnnotations.openMocks(this)
        mClassUnderTest = PlaybackController(mMockPlaybackControlMessages)
    }

    @Test
    fun testRepeatModeOffIsDispatchedToAACS() {
        mClassUnderTest.setRepeatMode(Player.REPEAT_MODE_OFF)
        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.REPEAT, false)
    }

    @Test
    fun testRepeatModeOneIsDispatchedToAACS() {
        mClassUnderTest.setRepeatMode(Player.REPEAT_MODE_ONE)
        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.REPEAT, true)
    }

    @Test
    fun testRepeatModeAllIsDispatchedToAACS() {
        mClassUnderTest.setRepeatMode(Player.REPEAT_MODE_ALL)
        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.REPEAT, true)
    }

    @Test
    fun testShuffleModeEnableIsDispatchedToAACS() {
        mClassUnderTest.setShuffleModeEnabled(true)
        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.SHUFFLE, true)
    }

    @Test
    fun testShuffleModeDisableIsDispatchedToAACS() {
        mClassUnderTest.setShuffleModeEnabled(false)
        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.SHUFFLE, false)
    }

    @Test
    fun testSkipToPreviousIsDispatchedToAACS() {
        mClassUnderTest.skipToPrevious()

        verify(mMockPlaybackControlMessages, times(1)).sendButtonCommandToAACS(
            PlaybackConstants.PlaybackButton.PREVIOUS)
    }

    @Test
    fun testSkipToNextIsDispatchedToAACS() {
        mClassUnderTest.skipToNext()

        verify(mMockPlaybackControlMessages, times(1)).sendButtonCommandToAACS(
            PlaybackConstants.PlaybackButton.NEXT)
    }

    @Test
    fun testStopIsDispatchedToAACS() {
        mClassUnderTest.stop()

        verify(mMockPlaybackControlMessages, times(1)).sendButtonCommandToAACS(
            PlaybackConstants.PlaybackButton.PAUSE)
    }

    @Test
    fun testPauseIsDispatchedToAACS() {
        mClassUnderTest.setPlay(false)

        verify(mMockPlaybackControlMessages, times(1)).sendButtonCommandToAACS(
            PlaybackConstants.PlaybackButton.PAUSE)
    }

    @Test
    fun testPlayIsDispatchedToAACS() {
        mClassUnderTest.setPlay(true)

        verify(mMockPlaybackControlMessages, times(1)).sendButtonCommandToAACS(
            PlaybackConstants.PlaybackButton.PLAY)
    }

    @Test
    fun testUnratedRatingIsDispatchedToAACS() {
        val rating = RatingCompat.newUnratedRating(RatingCompat.RATING_THUMB_UP_DOWN)
        mClassUnderTest.setRating(rating)

        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.THUMBS_UP, false)
        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.THUMBS_DOWN, false)
    }

    @Test
    fun testThumbsUpRatingIsDispatchedToAACS() {
        val rating = RatingCompat.newThumbRating(true)
        mClassUnderTest.setRating(rating)

        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.THUMBS_UP, true)
        verify(mMockPlaybackControlMessages, times(0)).sendToggleCommandToAACS(
            eq(PlaybackConstants.ToggleButton.THUMBS_DOWN), anyBoolean())
    }

    @Test
    fun testThumbsDownRatingIsDispatchedToAACS() {
        val rating = RatingCompat.newThumbRating(false)
        mClassUnderTest.setRating(rating)

        verify(mMockPlaybackControlMessages, times(1)).sendToggleCommandToAACS(
            PlaybackConstants.ToggleButton.THUMBS_DOWN, true)
        verify(mMockPlaybackControlMessages, times(0)).sendToggleCommandToAACS(
            eq(PlaybackConstants.ToggleButton.THUMBS_UP), anyBoolean())
    }
}