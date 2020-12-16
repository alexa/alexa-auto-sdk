package com.amazon.alexa.auto.media.session

import android.content.Context
import android.os.Build
import android.support.v4.media.MediaMetadataCompat
import android.support.v4.media.session.MediaSessionCompat
import android.support.v4.media.session.PlaybackStateCompat
import androidx.test.core.app.ApplicationProvider
import com.amazon.aacsconstants.TemplateRuntimeConstants
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_LOOP
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_PLAY_PAUSE
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_SHUFFLE
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_SKIP_BACKWARD
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_SKIP_FORWARD
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_TYPE_BUTTON
import com.amazon.alexa.auto.aacs.common.PlaybackControl
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages
import com.amazon.alexa.auto.media.R
import com.amazon.alexa.auto.media.player.MediaPlayerExo
import com.amazon.alexa.auto.media.player.MediaState
import com.google.android.exoplayer2.PlaybackParameters
import com.google.android.exoplayer2.Player
import com.google.android.exoplayer2.SimpleExoPlayer
import io.reactivex.rxjava3.core.Observable
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentCaptor
import org.mockito.Captor
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.Mockito.`when`
import org.mockito.Mockito.any
import org.mockito.Mockito.anyBoolean
import org.mockito.Mockito.mock
import org.mockito.Mockito.times
import org.mockito.Mockito.verify
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config
import java.util.*

@RunWith(RobolectricTestRunner::class)
@Config(sdk = intArrayOf(Build.VERSION_CODES.O))
class MediaSessionManagerTest {
    lateinit var mClassUnderTest: MediaSessionManager

    // MediaSessionManager dependencies
    @Mock lateinit var mMockMediaSession: MediaSessionCompat
    @Mock lateinit var mMockMediaPlayer: MediaPlayerExo
    @Mock lateinit var mMockPlayer: SimpleExoPlayer
    @Mock lateinit var mMockMetadataProvder: MediaMetadataProvider
    @Mock lateinit var mMockController: PlaybackController
    @Mock lateinit var mMockMessageSender: PlaybackControlMessages
    @Mock lateinit var mMockSkipForwardActionProvider: PlaybackControlButtonActionProvider
    @Mock lateinit var mMockSkipBackwardAcitonProvider: PlaybackControlButtonActionProvider

    // Argument capture
    @Captor private lateinit var mPlaybackCompatArgs: ArgumentCaptor<PlaybackStateCompat>

    lateinit var mSkipForwardAction: PlaybackStateCompat.CustomAction
    lateinit var mSkipBackwardAction: PlaybackStateCompat.CustomAction
    lateinit var mCustomActionProviders: CustomActionProviders

    @Before
    fun setup() {
        MockitoAnnotations.openMocks(this)

        mSkipForwardAction = PlaybackStateCompat.CustomAction.Builder(
            "skip-forward", "skip-forward", R.drawable.media_skip_forward_selected).build()
        mSkipBackwardAction = PlaybackStateCompat.CustomAction.Builder(
            "skip-backward", "skip-backward", R.drawable.media_skip_backward_selected).build()

        `when`(mMockPlayer.playbackParameters).thenReturn(PlaybackParameters(1.0f))
        `when`(mMockMediaPlayer.player).thenReturn(mMockPlayer)
        `when`(mMockMediaPlayer.mediaState).thenReturn(MediaState(true, Player.STATE_READY))
        `when`(mMockMetadataProvder.updateMetadata(any())).thenReturn(
            Observable.just(Optional.empty()))
        `when`(mMockMetadataProvder.updateMetadata(any())).thenReturn(
            Observable.just(Optional.empty()))

        val context = ApplicationProvider.getApplicationContext<Context>();
        `when`(mMockSkipForwardActionProvider.customAction).thenReturn(mSkipForwardAction)
        `when`(mMockSkipBackwardAcitonProvider.customAction).thenReturn(mSkipBackwardAction)

        mCustomActionProviders = CustomActionProviders(context, mMockMessageSender)
        mCustomActionProviders.mSkipBackwardProvider = mMockSkipBackwardAcitonProvider
        mCustomActionProviders.mSkipForwardProvider = mMockSkipForwardActionProvider

        mClassUnderTest = MediaSessionManager(
            mMockMediaPlayer, mMockMediaSession, mMockMetadataProvder,
            mMockController, mCustomActionProviders)
    }

    @Test
    fun testUpdateNullRenderPlayerInfo() {
        mClassUnderTest.updateMediaSession(null)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        // Verify playback state.
        assertEquals(0, mPlaybackCompatArgs.value.actions)
        assertEquals(0, mPlaybackCompatArgs.value.customActions.size)

        // Verify shuffle and repeat mode
        verify(mMockMediaSession, times(1)).setShuffleMode(PlaybackStateCompat.SHUFFLE_MODE_INVALID)
        verify(mMockMediaSession, times(1)).setRepeatMode(PlaybackStateCompat.REPEAT_MODE_INVALID)

        // Verify media metadata
        verify(mMockMediaSession, times(1)).setMetadata(null)
    }

    @Test
    fun testUpdateWithShuffleMode() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_SHUFFLE, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setShuffleMode(PlaybackStateCompat.SHUFFLE_MODE_NONE)
        verify(mMockMediaSession, times(1)).setRepeatMode(PlaybackStateCompat.REPEAT_MODE_INVALID)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE)
    }

    @Test
    fun testUpdateWithRepeatModeEnabled() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_LOOP, true, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setShuffleMode(PlaybackStateCompat.SHUFFLE_MODE_INVALID)
        verify(mMockMediaSession, times(1)).setRepeatMode(PlaybackStateCompat.REPEAT_MODE_ALL)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(PlaybackStateCompat.ACTION_SET_REPEAT_MODE,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SET_REPEAT_MODE)
    }

    @Test
    fun testUpdateWithRepeatModeDisabled() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_LOOP, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setRepeatMode(PlaybackStateCompat.REPEAT_MODE_NONE)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(PlaybackStateCompat.ACTION_SET_REPEAT_MODE,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SET_REPEAT_MODE)
    }

    @Test
    fun testUpdateWithSkipForwardCustomAction() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_SKIP_FORWARD, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        // Verify Custom action were included in playback state.
        assertEquals(1, mPlaybackCompatArgs.value.customActions.size)
        assertEquals(mSkipForwardAction, mPlaybackCompatArgs.value.customActions.get(0))
    }

    @Test
    fun testUpdateWithSkipBackwardCustomAction() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_SKIP_BACKWARD, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        // Verify Custom action were included in playback state.
        assertEquals(1, mPlaybackCompatArgs.value.customActions.size)
        assertEquals(mSkipBackwardAction, mPlaybackCompatArgs.value.customActions.get(0))
    }

    @Test
    fun testAllPlaybackControlDisabled() {
        val playerInfo = RenderPlayerInfoBuilder().build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(0L,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_PLAY_PAUSE)
        assertEquals(0L,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE)
        assertEquals(0L,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SET_REPEAT_MODE)
    }

    @Test
    fun testUpdateWithPlayPauseEnabled() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_PLAY_PAUSE, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(PlaybackStateCompat.ACTION_PLAY,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_PLAY)
        assertEquals(PlaybackStateCompat.ACTION_PAUSE,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_PAUSE)
    }

    @Test
    fun testUpdateWithSkipToPrevious() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, TemplateRuntimeConstants.CONTROL_NAME_PREVIOUS, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS)
    }

    @Test
    fun testUpdateWithSkipToNext() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, TemplateRuntimeConstants.CONTROL_NAME_NEXT, false, CONTROL_TYPE_BUTTON))
            .build()

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setPlaybackState(mPlaybackCompatArgs.capture())

        assertEquals(PlaybackStateCompat.ACTION_SKIP_TO_NEXT,
            mPlaybackCompatArgs.value.actions and PlaybackStateCompat.ACTION_SKIP_TO_NEXT)
    }

    @Test
    fun testUpdateWithMetadata() {
        val playerInfo = RenderPlayerInfoBuilder().build()

        val metadata = mock(MediaMetadataCompat::class.java)

        Mockito.`when`(mMockMetadataProvder.updateMetadata(any())).thenReturn(
            Observable.just(Optional.of(metadata)))
        Mockito.`when`(mMockMetadataProvder.updateMetadata(any())).thenReturn(
            Observable.just(Optional.of(metadata)))

        mClassUnderTest.updateMediaSession(playerInfo)

        verify(mMockMediaSession, times(1)).setMetadata(metadata)
    }

    @Test
    fun testMediaSessionControlPlay() {
        val eventListeners = activateMediaSession()

        eventListeners.first.onPlay()
        verify(mMockController, times(0)).setPlay(anyBoolean())

        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_PLAY_PAUSE, false, CONTROL_TYPE_BUTTON))
            .build()
        mClassUnderTest.updateMediaSession(playerInfo)

        eventListeners.first.onPlay()
        verify(mMockController, times(1)).setPlay(true)
    }

    /**
     * Activate the media session and return the callbacks registered
     * by {@link MediaSessionManager}.
     */
    fun activateMediaSession() : Pair<MediaSessionCompat.Callback, Player.EventListener> {
        mClassUnderTest.activateMediaSession()

        val mediaSessionCallbackArgs = ArgumentCaptor.forClass(
            MediaSessionCompat.Callback::class.java)
        val playerListenerArgs = ArgumentCaptor.forClass(Player.EventListener::class.java)

        verify(mMockMediaSession, times(1)).setCallback(mediaSessionCallbackArgs.capture())
        verify(mMockPlayer, times(1)).addListener(playerListenerArgs.capture())

        return Pair(mediaSessionCallbackArgs.value, playerListenerArgs.value)
    }
}