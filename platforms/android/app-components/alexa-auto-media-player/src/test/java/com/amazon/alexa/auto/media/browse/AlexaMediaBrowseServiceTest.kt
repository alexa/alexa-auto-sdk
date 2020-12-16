package com.amazon.alexa.auto.media.browse

import android.app.Application
import android.app.Notification
import android.app.PendingIntent
import android.content.Intent
import android.os.Build
import android.support.v4.media.session.MediaSessionCompat
import android.support.v4.media.session.PlaybackStateCompat
import com.amazon.aacsconstants.Action
import com.amazon.aacsconstants.Topic
import com.amazon.alexa.auto.aacs.common.AACSMessage
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder
import com.amazon.alexa.auto.aacs.common.AACSMessageSender
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent
import com.amazon.alexa.auto.apis.auth.AuthController
import com.amazon.alexa.auto.apis.auth.AuthStatus
import com.amazon.alexa.auto.apis.auth.UserIdentity
import com.amazon.alexa.auto.apis.setup.AlexaSetupController
import com.amazon.alexa.auto.media.R
import com.amazon.alexa.auto.media.aacs.handlers.AudioPlayerHandler
import com.amazon.alexa.auto.media.aacs.handlers.TemplateRuntimeHandler
import com.amazon.alexa.auto.media.player.MediaPlayerExo
import com.amazon.alexa.auto.media.player.NotificationController
import com.amazon.alexa.auto.media.session.MediaSessionManager
import com.google.android.exoplayer2.Player
import io.reactivex.rxjava3.subjects.BehaviorSubject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers.any
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.Mockito.`when`
import org.mockito.Mockito.eq
import org.mockito.Mockito.mock
import org.mockito.Mockito.times
import org.mockito.Mockito.verify
import org.mockito.MockitoAnnotations
import org.robolectric.Robolectric
import org.robolectric.RobolectricTestRunner
import org.robolectric.Shadows
import org.robolectric.android.controller.ServiceController
import org.robolectric.annotation.Config
import org.robolectric.shadows.ShadowLooper
import org.robolectric.shadows.ShadowService
import java.util.concurrent.TimeUnit

class MockApplication : Application(), AlexaApp {
    val mMockRootComponent = mock(AlexaAppRootComponent::class.java)

    override fun getRootComponent(): AlexaAppRootComponent {
        return mMockRootComponent
    }
}

@RunWith(RobolectricTestRunner::class)
@Config(sdk = intArrayOf(Build.VERSION_CODES.O), application = MockApplication::class)
class AlexaMediaBrowseServiceTest {
    lateinit var mServiceController : ServiceController<AlexaMediaBrowseService>
    lateinit var mShadowService : ShadowService

    @Mock lateinit var mMockMediaPlayer: MediaPlayerExo
    @Mock lateinit var mMockMediaSessionManager: MediaSessionManager
    @Mock lateinit var mMockNotificationController: NotificationController
    @Mock lateinit var mMockAACSMessageSender: AACSMessageSender
    @Mock lateinit var mMockAudioPlayerHandler: AudioPlayerHandler
    @Mock lateinit var mMockTemplateruntimeHandler: TemplateRuntimeHandler

    @Mock lateinit var mMockMediaSession: MediaSessionCompat
    @Mock lateinit var mMockNotification: Notification

    @Mock lateinit var mMockAuthController: AuthController
    @Mock lateinit var mMockSetupController: AlexaSetupController

    val mAuthStatusSubject = BehaviorSubject.create<AuthStatus>()
    val mVASelectionSubject = BehaviorSubject.create<Boolean>()
    val mLoginUILaunchIntent = Intent("login-launch-intent")
    val mVoiceAssistantSelectionLaunchIntent = Intent("voice-assistant-selection")

    @Before
    fun setup() {
        MockitoAnnotations.openMocks(this)

        mServiceController = Robolectric.buildService(AlexaMediaBrowseService::class.java)
        mShadowService = Shadows.shadowOf(mServiceController.get())

        val service = mServiceController.get()
        service.mMediaPlayer = mMockMediaPlayer
        service.mMediaSessionManager = mMockMediaSessionManager
        service.mNotificationController = mMockNotificationController
        service.mAACSMessageSender = mMockAACSMessageSender
        service.mAudioPlayerHandler = mMockAudioPlayerHandler
        service.mTemplateruntimeHandler = mMockTemplateruntimeHandler

        Mockito.`when`(mMockMediaSessionManager.mediaSession).thenReturn(mMockMediaSession)
        Mockito.`when`(mMockNotificationController.createServiceStartNotification(Mockito.any()))
            .thenReturn(mMockNotification)

        val app = mServiceController.get().application as AlexaApp
        `when`(app.rootComponent.authController).thenReturn(mMockAuthController)
        `when`(app.rootComponent.alexaSetupController).thenReturn(mMockSetupController)
        `when`(mMockAuthController.observeAuthChangeOrLogOut()).thenReturn(mAuthStatusSubject)
        `when`(mMockSetupController.observeVoiceAssistantSelection()).thenReturn(mVASelectionSubject)
        `when`(mMockSetupController.createIntentForLaunchingLoginUI())
            .thenReturn(mLoginUILaunchIntent)
        `when`(mMockSetupController.createIntentForLaunchingVoiceAssistantSwitchUI())
            .thenReturn(mVoiceAssistantSelectionLaunchIntent)
    }

    @Test
    fun testAnyStartCommandStartsTheService() {
        val intent = Intent()
        mServiceController.get().onStartCommand(intent, 0, 0)

        // If foreground notification is present, then service has been started.
        assertEquals(mMockNotification, mShadowService.lastForegroundNotification)
    }

    @Test
    fun testServiceIsStoppedAfterIdleTimeout() {
        val intent = Intent()
        mServiceController.get().onStartCommand(intent, 0, 0)
        assertFalse(mShadowService.isStoppedBySelf)

        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS, TimeUnit.MILLISECONDS)

        assertTrue(mShadowService.isStoppedBySelf)
    }

    @Test
    fun testIdleTimerIsResetOnNewStartCommand() {
        val intent = Intent()
        mServiceController.get().onStartCommand(intent, 0, 0)
        assertFalse(mShadowService.isStoppedBySelf)

        // Skip first half the first timeout.
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS / 2, TimeUnit.MILLISECONDS)
        assertFalse(mShadowService.isStoppedBySelf)

        // Send another start command to reset the idle timer.
        mServiceController.get().onStartCommand(intent, 0, 0)

        // Skip second half of the first timeout (and first half of the second timeout)
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS / 2, TimeUnit.MILLISECONDS)

        // Since idle timer has been reset, so half the time should not cause service stoppage.
        assertFalse(mShadowService.isStoppedBySelf)

        // Skip second half of the second timeout.
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS / 2, TimeUnit.MILLISECONDS)

        // Now the service should have been destroyed.
        assertTrue(mShadowService.isStoppedBySelf)
    }

    @Test
    fun testStopOnIdleTimerIsCancelledOnPrepareCommand() {
        // Test that on prepare command service gets into long running mode and
        // is not auto stopped after idle timeout.

        val intent = AACSMessageBuilder.buildEmbeddedMessageIntent(
            Topic.AUDIO_OUTPUT, Action.AudioOutput.PREPARE, null)
        assertTrue(intent.isPresent)
        mServiceController.get().onStartCommand(intent.get(), 0, 0)

        assertFalse(mShadowService.isStoppedBySelf)

        // Skip twice the idle time
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS * 2, TimeUnit.MILLISECONDS)

        // Verify that service is still not stopped.
        assertFalse(mShadowService.isStoppedBySelf)
    }

    @Test
    fun testOnPlaybackStoppageServiceIsStoppedAfterIdleTimeout() {
        // Start service such that no stop on idle timer is kicked off.
        val intent = AACSMessageBuilder.buildEmbeddedMessageIntent(
            Topic.AUDIO_OUTPUT, Action.AudioOutput.PREPARE, null)
        assertTrue(intent.isPresent)
        mServiceController.get().onStartCommand(intent.get(), 0, 0)

        // Publish the media state that playback is stopped.
        mServiceController.get().getEventListener().onPlayerStateChanged(false, Player.STATE_IDLE)

        // Skip the idle timeout
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS, TimeUnit.MILLISECONDS)

        // Verify that service is stopped.
        assertTrue(mShadowService.isStoppedBySelf)
    }

    @Test
    fun testIsNotStoppedAfterPlaybackResumes() {
        val intent = Intent()
        mServiceController.get().onStartCommand(intent, 0, 0)
        assertFalse(mShadowService.isStoppedBySelf)

        // Singal playback resume
        mServiceController.get().getEventListener().onPlayerStateChanged(
            true, Player.STATE_BUFFERING)

        // Skip the idle timeout
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS, TimeUnit.MILLISECONDS)

        // Verify service isn't stopped after idle timeout.
        assertFalse(mShadowService.isStoppedBySelf)
    }

    @Test
    fun testAnyIntentDoesNotScheduleStopOnIdleTimerWhenServiceIsBusy() {
        // Start service such that no stop on idle timer is kicked off.
        val intent = AACSMessageBuilder.buildEmbeddedMessageIntent(
            Topic.AUDIO_OUTPUT, Action.AudioOutput.PREPARE, null)
        assertTrue(intent.isPresent)
        mServiceController.get().onStartCommand(intent.get(), 0, 0)

        val anotherIntent = Intent()
        mServiceController.get().onStartCommand(anotherIntent, 0, 0)

        // Skip the idle timeout
        ShadowLooper.idleMainLooper(
            AlexaMediaBrowseService.SERVICE_IDLE_TIMEOUT_MS, TimeUnit.MILLISECONDS)

        // Verify that service is still running.
        assertFalse(mShadowService.isStoppedBySelf)
    }

    @Test
    fun testAudioOutputCommandsAreDispatchedToHandler() {
        val action = "any-action"
        val intent = AACSMessageBuilder.buildEmbeddedMessageIntent(
            Topic.AUDIO_OUTPUT, action, null)

        mServiceController.get().onStartCommand(intent.get(), 0, 0)

        val commandCaptor = ArgumentCaptor.forClass(AACSMessage::class.java)
        verify(mMockAudioPlayerHandler, times(1)).handleAACSCommand(commandCaptor.capture())

        assertEquals(action, commandCaptor.value.action)
        assertEquals(Topic.AUDIO_OUTPUT, commandCaptor.value.topic)
    }

    @Test
    fun testTemplateRuntimeCommandsAreDispatchedToHandler() {
        val action = "any-action"
        val intent = AACSMessageBuilder.buildEmbeddedMessageIntent(
            Topic.TEMPLATE_RUNTIME, action, null)

        mServiceController.get().onStartCommand(intent.get(), 0, 0)

        val commandCaptor = ArgumentCaptor.forClass(AACSMessage::class.java)
        verify(mMockTemplateruntimeHandler, times(1)).handleAACSCommand(commandCaptor.capture())

        assertEquals(action, commandCaptor.value.action)
        assertEquals(Topic.TEMPLATE_RUNTIME, commandCaptor.value.topic)
    }

    @Test
    fun testThatServiceIsInitializedWithAuthErrorOnLogout() {
        mServiceController.get().initializeMediaSessionWithErrorForGuidingUser()

        mAuthStatusSubject.onNext(AuthStatus(false, null))

        verify(mMockMediaSessionManager, times(1)).setMediaSessionError(
            eq(PlaybackStateCompat.ERROR_CODE_AUTHENTICATION_EXPIRED),
            eq(mServiceController.get().getString(R.string.alexa_music_login_required)),
            eq(mServiceController.get().getString(R.string.alexa_music_login_text)),
            any(PendingIntent::class.java))
    }

    @Test
    fun testThatServiceIsInitializedWithVASelectionErrorOnLogin() {
        mServiceController.get().initializeMediaSessionWithErrorForGuidingUser()

        mVASelectionSubject.onNext(false)
        mAuthStatusSubject.onNext(AuthStatus(true, UserIdentity("user1")))

        verify(mMockMediaSessionManager, times(1)).setMediaSessionError(
            eq(PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR),
            eq(mServiceController.get().getString(R.string.alexa_music_alexa_not_selected_va)),
            eq(mServiceController.get().getString(R.string.alexa_music_select_va)),
            any(PendingIntent::class.java))
    }

    @Test
    fun testThatServiceIsInitializedWithHintsErrorOnLoginAndVASelection() {
        mServiceController.get().initializeMediaSessionWithErrorForGuidingUser()

        mVASelectionSubject.onNext(true)
        mAuthStatusSubject.onNext(AuthStatus(true, UserIdentity("user1")))

        verify(mMockMediaSessionManager, times(1)).setMediaSessionError(
            PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR,
            mServiceController.get().getString(R.string.alexa_music_hint_1),
            null, null)
    }
}