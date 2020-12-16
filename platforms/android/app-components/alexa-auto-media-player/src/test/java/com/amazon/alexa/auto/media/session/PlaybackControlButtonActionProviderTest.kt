package com.amazon.alexa.auto.media.session

import android.os.Build
import com.amazon.aacsconstants.PlaybackConstants
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages
import com.google.android.exoplayer2.ControlDispatcher
import com.google.android.exoplayer2.Player

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mock
import org.mockito.Mockito.times
import org.mockito.Mockito.verify
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
@Config(sdk = intArrayOf(Build.VERSION_CODES.O))
class PlaybackControlButtonActionProviderTest {

    companion object {
        val BUTTON_RESOURCE_ID = 42
        val BUTTON_DISPLAY_NAME = "test-btn-display-name"
        val BUTTON_CONTROL_NAME = PlaybackConstants.PlaybackButton.SKIP_FORWARD
    }

    lateinit var mClassUnderTest: PlaybackControlButtonActionProvider

    @Mock lateinit var mMessageSender: PlaybackControlMessages

    @Before
    fun setup() {
        MockitoAnnotations.initMocks(this)

        mClassUnderTest = PlaybackControlButtonActionProvider(
            mMessageSender, BUTTON_RESOURCE_ID, BUTTON_DISPLAY_NAME, BUTTON_CONTROL_NAME,
            false, false)
    }

    @Test
    fun testCustomActionIsCreated() {
        val customAction = mClassUnderTest.getCustomAction()

        assertNotNull(customAction)
        assertEquals(BUTTON_RESOURCE_ID, customAction?.icon)
        assertEquals(BUTTON_DISPLAY_NAME, customAction?.name)
    }

    @Test
    fun testExecuteCustomAction() {
        mClassUnderTest.onCustomAction(BUTTON_CONTROL_NAME, null)

        verify(mMessageSender, times(1)).sendButtonCommandToAACS(BUTTON_CONTROL_NAME)
    }
}