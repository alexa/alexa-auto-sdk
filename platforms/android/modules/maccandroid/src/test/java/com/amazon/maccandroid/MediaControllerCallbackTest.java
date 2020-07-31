package com.amazon.maccandroid;

import static org.junit.Assert.*;

import android.content.Context;
import android.drm.DrmStore;
import android.support.v4.media.session.PlaybackStateCompat;

import com.amazon.maccandroid.model.PlayerEvents;
import com.amazon.maccandroid.model.errors.MediaAppPlayerError;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.HashSet;
import java.util.Set;

public class MediaControllerCallbackTest {
    public static final String TEST_PACKAGE_NAME = "testPackageName";
    public static final String TEST_CLASS_NAME = "testClassName";
    public static final String TEST_SPI_VERSION = "testSpiVersion";
    public static final String TEST_PLAYER_COOKIE = "testPlayerCookie";
    private MediaControllerCallback mClassUnderTest;
    private Context mMockContext;
    private MediaApp mMediaApp;
    private MediaAppsStateReporter mMockMediaAppsStateReporter;

    private PlaybackStateCompat mPlayingPlaybackState =
            new PlaybackStateCompat.Builder().setState(PlaybackStateCompat.STATE_PLAYING, 0, (float) 0.0).build();
    private PlaybackStateCompat mStopedPlaybackState =
            new PlaybackStateCompat.Builder().setState(PlaybackStateCompat.STATE_STOPPED, 0, (float) 0.0).build();
    private PlaybackStateCompat mPausedPlaybackState =
            new PlaybackStateCompat.Builder().setState(PlaybackStateCompat.STATE_PAUSED, 0, (float) 0.0).build();
    private PlaybackStateCompat mSkipToNextPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_SKIPPING_TO_NEXT, 0, (float) 0.0)
                    .build();
    private PlaybackStateCompat mSkipToPreviousPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_SKIPPING_TO_PREVIOUS, 0, (float) 0.0)
                    .build();
    private PlaybackStateCompat mRewindPlaybackState =
            new PlaybackStateCompat.Builder().setState(PlaybackStateCompat.STATE_REWINDING, 0, (float) 0.0).build();
    private PlaybackStateCompat mFastForwardPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_FAST_FORWARDING, 0, (float) 0.0)
                    .build();
    private PlaybackStateCompat mNonePlaybackState =
            new PlaybackStateCompat.Builder().setState(PlaybackStateCompat.STATE_NONE, 0, (float) 0.0).build();
    private PlaybackStateCompat mErrorActionAbortedPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_ACTION_ABORTED, null)
                    .build();
    private PlaybackStateCompat mErrorAppErrorPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_APP_ERROR, null)
                    .build();
    private PlaybackStateCompat mErrorAuthenticationExpiredPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_AUTHENTICATION_EXPIRED, null)
                    .build();
    private PlaybackStateCompat mErrorConcurrentStreamPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_CONCURRENT_STREAM_LIMIT, null)
                    .build();
    private PlaybackStateCompat mErrorContentAlreadyPlayingPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_CONTENT_ALREADY_PLAYING, null)
                    .build();
    private PlaybackStateCompat mErrorEndOfQueuePlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_END_OF_QUEUE, null)
                    .build();
    private PlaybackStateCompat mErrorNotAvailableInRegionPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_NOT_AVAILABLE_IN_REGION, null)
                    .build();
    private PlaybackStateCompat mErrorNotSupportedPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_NOT_SUPPORTED, null)
                    .build();
    private PlaybackStateCompat mErrorParentalControlRestrictedPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_PARENTAL_CONTROL_RESTRICTED, null)
                    .build();
    private PlaybackStateCompat mErrorPremiumAccountRequiredPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_PREMIUM_ACCOUNT_REQUIRED, null)
                    .build();
    private PlaybackStateCompat mErrorSkipLimitReachedPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_SKIP_LIMIT_REACHED, null)
                    .build();
    private PlaybackStateCompat mErrorUnknownErrorPlaybackState =
            new PlaybackStateCompat.Builder()
                    .setState(PlaybackStateCompat.STATE_ERROR, 0, (float) 0.0)
                    .setErrorMessage(PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR, null)
                    .build();

    @Before
    public void setUp() throws Exception {
        mMockContext = Mockito.mock(Context.class);
        mMediaApp =
                new MediaApp(mMockContext, TEST_PACKAGE_NAME, TEST_CLASS_NAME, TEST_SPI_VERSION, TEST_PLAYER_COOKIE) {
                    @Override
                    void initValidationData(Context context) {}
                };
        MediaAppsRepository.getInstance().addDiscoveredMediaApp(mMediaApp);
        MediaAppsRepository.getInstance().addAuthorizedMediaApp(mMediaApp);
        mClassUnderTest = new MediaControllerCallback(TEST_PACKAGE_NAME, mMediaApp);
        mMockMediaAppsStateReporter = Mockito.mock(MediaAppsStateReporter.class);
        MediaAppsStateReporter.setTestStateReporter(mMockMediaAppsStateReporter);
    }

    @After
    public void tearDown() throws Exception {}

    @Test
    public void testPlayerEvents() {
        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Set<PlayerEvents> eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStarted);
        eventsToSend.add(PlayerEvents.PlaybackSessionStarted);
        eventsToSend.add(PlayerEvents.PlayModeChanged);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mStopedPlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mPausedPlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mRewindPlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        eventsToSend.add(PlayerEvents.PlayModeChanged);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mFastForwardPlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        eventsToSend.add(PlayerEvents.PlayModeChanged);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mSkipToPreviousPlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        eventsToSend.add(PlayerEvents.PlaybackPrevious);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mSkipToNextPlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        eventsToSend.add(PlayerEvents.PlaybackNext);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));

        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mNonePlaybackState);
        eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlaybackStopped);
        eventsToSend.add(PlayerEvents.PlaybackSessionEnded);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportPlayerEvent(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(eventsToSend));
    }

    @Test
    public void testErrorEvents() {
        mClassUnderTest.onPlaybackStateChanged(mPlayingPlaybackState);
        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorActionAbortedPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.UNPLAYABLE_BY_ACCOUNT));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorAppErrorPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.INTERNAL_ERROR));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorAuthenticationExpiredPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(
                        Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.UNPLAYABLE_BY_AUTHORIZATION));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorConcurrentStreamPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME),
                        Mockito.eq(MediaAppPlayerError.UNPLAYABLE_BY_STREAM_CONCURRENCY));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorContentAlreadyPlayingPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME),
                        Mockito.eq(MediaAppPlayerError.OPERATION_REJECTED_UNINTERRUPTIBLE));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorEndOfQueuePlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(
                        Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.OPERATION_REJECTED_END_OF_QUEUE));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorNotAvailableInRegionPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.UNPLAYABLE_BY_REGION));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorNotSupportedPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.OPERATION_UNSUPPORTED));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorParentalControlRestrictedPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(
                        Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.UNPLAYABLE_BY_PARENTAL_CONTROL));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorPremiumAccountRequiredPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.UNPLAYABLE_BY_SUBSCRIPTION));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorSkipLimitReachedPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(
                        Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.OPERATION_REJECTED_SKIP_LIMIT));

        Mockito.reset(mMockMediaAppsStateReporter);
        mClassUnderTest.onPlaybackStateChanged(mErrorUnknownErrorPlaybackState);
        Mockito.verify(mMockMediaAppsStateReporter)
                .reportError(Mockito.eq(TEST_PACKAGE_NAME), Mockito.eq(MediaAppPlayerError.UNKNOWN_ERROR));
    }
}