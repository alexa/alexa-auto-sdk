package com.amazon.alexa.auto.media.player

import android.media.AudioFocusRequest
import android.media.AudioManager
import android.os.Build
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mock
import org.mockito.Mockito.`when`
import org.mockito.Mockito.any
import org.mockito.Mockito.times
import org.mockito.Mockito.verify
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = intArrayOf(Build.VERSION_CODES.O))
class MediaPlayerAudioFocusControllerTest {
    private lateinit var mClassUnderTest: MediaPlayerAudioFocusController

    @Mock lateinit var mMockAudioManager: AudioManager
    @Mock lateinit var mMockPlaybackController : MediaPlayerAudioFocusController.PlaybackController

    @Before
    fun setup() {
        MockitoAnnotations.openMocks(this)

        mClassUnderTest = MediaPlayerAudioFocusController(mMockAudioManager)
        mClassUnderTest.setPlaybackController(mMockPlaybackController)
    }

    @Test
    fun `test that focus is acquired on start of playback`() {
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        verify(mMockPlaybackController, times(1)).startPlaybackNow()
    }

    @Test
    fun `test that focus is not re-acquired on next playback when we already have a focus`() {
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)

        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Focus already acquired, lets try again.
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Verify playback starts twice.
        verify(mMockPlaybackController, times(2)).startPlaybackNow()

        // But Focus is acquired only once.
        verify(mMockAudioManager, times(1)).requestAudioFocus(any(AudioFocusRequest::class.java))
    }

    @Test
    fun `test that focus is not re-acquired when playback is requested in ducking state`() {
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)

        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Make it duck
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK)

        // At duck state, lets try again.
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Verify playback starts twice.
        verify(mMockPlaybackController, times(2)).startPlaybackNow()

        // But Focus is acquired only once.
        verify(mMockAudioManager, times(1)).requestAudioFocus(any(AudioFocusRequest::class.java))
    }

    @Test
    fun `test that playback is deferred on audio focus acquisition delay`() {
        // Make it so that focus grant is deferred.
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_DELAYED)
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        verify(mMockPlaybackController, times(0)).startPlaybackNow()

        // Grant the focus now.
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_GAIN)

        // Verify playback is started now.
        verify(mMockPlaybackController, times(1)).startPlaybackNow()
    }

    @Test
    fun `test that controller is notified on failure to acquire focus`() {
        // Make it so that focus grant is denied.
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_FAILED)
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        verify(mMockPlaybackController, times(0)).startPlaybackNow()
        verify(mMockPlaybackController, times(1)).failedToAcquireFocus()
    }

    @Test
    fun `test that focus is acquired again on next playback after focus is relinquished`() {
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)

        mClassUnderTest.startPlaybackAfterAcquiringFocus()
        mClassUnderTest.relinquishAudioFocusIfCurrentlyAcquired()
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Verify focus is acquired twice and playback is started twice.
        verify(mMockAudioManager, times(2)).requestAudioFocus(any(AudioFocusRequest::class.java))
        verify(mMockPlaybackController, times(2)).startPlaybackNow()
    }

    @Test
    fun `test that focus is not acquired again if previous request is still waiting for it`() {
        // Make it so that focus grant is deferred.
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_DELAYED)

        // Try playback a few times.
        mClassUnderTest.startPlaybackAfterAcquiringFocus()
        mClassUnderTest.startPlaybackAfterAcquiringFocus()
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Verify that playback doesn't begin.
        verify(mMockPlaybackController, times(0)).startPlaybackNow()
        // And neither do we make more than one attempt on acquiring focus.
        verify(mMockAudioManager, times(1)).requestAudioFocus(any(AudioFocusRequest::class.java))
    }

    @Test
    fun `test that volume is adjusted when ducked and unducked`() {
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)

        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Duck.
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK)

        // Verify Volume is adjusted to ducked level.
        verify(mMockPlaybackController, times(1)).adjustPlaybackVolume(
            MediaPlayerAudioFocusController.VOLUME_MULTIPLIER_DUCK)

        // UnDuck.
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_GAIN)

        // Verify Volume is adjusted to normal level.
        verify(mMockPlaybackController, times(1)).adjustPlaybackVolume(
            MediaPlayerAudioFocusController.VOLUME_MULTIPLIER_NORMAL)

        // Verify playback wasn't started twice.
        verify(mMockPlaybackController, times(1)).startPlaybackNow()
    }

    @Test
    fun `test playback resume is requested after reacquiring transiently lost focus`() {
        // Acquire focus
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Loose it transiently
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)


        // Notify focus is acquired.
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_GAIN)

        // Verify playback is started.
        verify(mMockPlaybackController, times(1)).requestResumingPlayback()
    }

    @Test
    fun `test that multiple focus acquisitions signals dont restart playback`() {
        // Acquire focus.
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Send multiple focus acquisition signals.
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_GAIN)
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_GAIN)
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_GAIN)

        // Verify playback resume is requested only once
        verify(mMockPlaybackController, times(0)).requestResumingPlayback()

        // And start playback is requested only once.
        verify(mMockPlaybackController, times(1)).startPlaybackNow()
    }

    @Test
    fun `test that playback stop is requested on focus loss`() {
        // Notify focus is acquired a few times
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_LOSS)

        // Verify playback is started only once
        verify(mMockPlaybackController, times(1)).requestStopPlayback()
    }

    @Test
    fun `test that playback pause is requested on transient focus loss`() {
        // Notify focus is acquired a few times
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)

        // Verify playback is started only once
        verify(mMockPlaybackController, times(1)).requestPausePlayback()
    }

    @Test
    fun `test that playback stop is requested on focus none signal`() {
        // Notify focus is acquired a few times
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_NONE)

        // Verify playback is started only once
        verify(mMockPlaybackController, times(1)).requestStopPlayback()
    }

    @Test
    fun `test that focus is abandoned on closing the focus controller object`() {
        // Acquire focus
        `when`(mMockAudioManager.requestAudioFocus(any(AudioFocusRequest::class.java)))
            .thenReturn(AudioManager.AUDIOFOCUS_REQUEST_GRANTED)
        mClassUnderTest.startPlaybackAfterAcquiringFocus()

        // Loose it transiently
        mClassUnderTest.onAudioFocusChange(AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)

        // Close the object.
        mClassUnderTest.close()

        // Verify focus is Abandoned.
        verify(mMockAudioManager, times(1)).abandonAudioFocusRequest(
            any(AudioFocusRequest::class.java))
    }
}