package com.amazon.alexa.auto.media.browse;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.media.MediaBrowserServiceCompat;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.R;
import com.amazon.alexa.auto.media.aacs.handlers.AudioPlayerHandler;
import com.amazon.alexa.auto.media.aacs.handlers.TemplateRuntimeHandler;
import com.amazon.alexa.auto.media.dependencies.AndroidModule;
import com.amazon.alexa.auto.media.dependencies.DaggerMediaComponent;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;
import com.amazon.alexa.auto.media.player.MediaState;
import com.amazon.alexa.auto.media.player.NotificationController;
import com.amazon.alexa.auto.media.session.MediaSessionManager;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.Player;

import java.util.ArrayList;
import java.util.List;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * MediaBrowseService which is responsible for keeping track the media playback
 * and its metadata
 */
public class AlexaMediaBrowseService extends MediaBrowserServiceCompat {
    private static final String TAG = AlexaMediaBrowseService.class.getSimpleName();

    // Idle timeout until service can remain up while idle. If service doesn't get
    // busy during this time, then service is stopped.
    public static final long SERVICE_IDLE_TIMEOUT_MS = 1000 * 60 * 30; // 30 minutes.

    // External Dependencies.
    @Inject
    MediaPlayerExo mMediaPlayer;
    @Inject
    MediaSessionManager mMediaSessionManager;
    @Inject
    NotificationController mNotificationController;
    @Inject
    AACSMessageSender mAACSMessageSender;
    @Inject
    AudioPlayerHandler mAudioPlayerHandler;
    @Inject
    TemplateRuntimeHandler mTemplateruntimeHandler;

    // Internal Dependencies.
    private MediaPlayerEventListener mMediaPlayerEventListener;

    // State
    private final ServiceLifecycle mServiceLifecycle;

    // Subscriptions
    @Nullable
    private Disposable mAuthSubscription;
    @Nullable
    private Disposable mVASelectionSubscription;

    /**
     * Creates an instance of Service.
     */
    public AlexaMediaBrowseService() {
        mMediaPlayerEventListener = new MediaPlayerEventListener();
        mServiceLifecycle = new ServiceLifecycle();
    }

    /**
     * Obtain Media Player event listener.
     *
     * @return Media Player event listener.
     */
    @VisibleForTesting
    MediaPlayerEventListener getEventListener() {
        return mMediaPlayerEventListener;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");

        DaggerMediaComponent.builder()
                .androidModule(new AndroidModule(getApplicationContext()))
                .build()
                .injectMediaBrowseService(this);

        mMediaSessionManager.activateMediaSession();
        setSessionToken(mMediaSessionManager.getMediaSession().getSessionToken());

        initializeMediaSessionWithErrorForGuidingUser();

        mMediaPlayer.getPlayer().addListener(mMediaPlayerEventListener);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            return super.onStartCommand(intent, flags, startId);
        }

        Log.d(TAG, "onStartCommand " + intent.getAction());

        mServiceLifecycle.startServiceWithIdleTimer();

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            if (Topic.AUDIO_OUTPUT.equals(message.topic)) {
                mAudioPlayerHandler.handleAACSCommand(message);
                if (Action.AudioOutput.PREPARE.equals(message.action)) {
                    // We know we need background work now, so we will mark the
                    // service as started.
                    mServiceLifecycle.markServiceBusy();
                }
            } else if (Topic.TEMPLATE_RUNTIME.equals(message.topic)) {
                mTemplateruntimeHandler.handleAACSCommand(message);
            }
        });

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        if (mVASelectionSubscription != null)
            mVASelectionSubscription.dispose();
        if (mAuthSubscription != null)
            mAuthSubscription.dispose();
        mMediaSessionManager.deactivateMediaSession();
        mMediaSessionManager.destroyMediaSession();

        mMediaPlayer.getPlayer().removeListener(mMediaPlayerEventListener);
        mMediaPlayer.getPlayer().release();

        try {
            mAudioPlayerHandler.close();
        } catch (Exception exception) {
            Log.e(TAG, "Error closing the Media Player Handler");
        }
    }

    @Override
    public BrowserRoot onGetRoot(@NonNull String clientPackageName, int clientUid, Bundle rootHints) {
        return new BrowserRoot("root-id", null);
    }

    @Override
    public void onLoadChildren(
            @NonNull final String parentMediaId, @NonNull final Result<List<MediaBrowserCompat.MediaItem>> result) {
        result.sendResult(new ArrayList<>());
    }

    /**
     * Handle lifecycle of this service based on current media state.
     *
     * @param currentState Media state from which next lifecycle event is
     *                     computed.
     */
    private void handleLifecycleForMediaState(MediaState currentState) {
        if (currentState.isPlaybackStopped()) {
            // We are done doing useful work.
            Log.i(TAG, "Media player is stopped. Marking the service idle.");
            mServiceLifecycle.markServiceIdle();
        } else {
            Log.i(TAG, "Media player is resuming. Marking the service busy.");
            mServiceLifecycle.markServiceBusy();
        }
    }

    /**
     * Mark the service as started and create and attach notification if platform
     * requires it.
     */
    private void startAlexaMediaService() {
        Log.i(TAG, "Starting Alexa Media Service (in foreground when required");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification notification = mNotificationController.createServiceStartNotification(getApplicationContext());
            startForeground(NotificationController.ALEXA_MEDIA_NOTIFICATION_ID, notification);
        }
    }

    /**
     * Stop the AlexaMediaBrowseService so that system can destroy and reclaim
     * the memory.
     */
    private void stopAlexaMediaService() {
        Log.i(TAG, "Stopping Alexa Media Service");
        stopSelf();
    }

    /**
     * If user is not authenticated and Alexa is not a selected voice assistant,
     * then initialize the media session with following set of error(s) to guide
     * the user to setup.
     * <ul>
     *     <li>
     *          If we can find {@link AuthController}, then Monitor the Alexa
     *          auth state and guide user to login UI if the user is not authenticated
     *          yet.
     *     </li>
     *     <li>
     *          If we can find {@link AlexaSetupController}, then check if Alexa
     *          is a currently selected voice assistant, if not, then launch the UI
     *          that would let customer switch the voice assistant.
     *     </li>
     *     <li>
     *          If user is authenticated and Alexa is currently selected voice
     *          assistant (or we cannot determine that), then show the hint that
     *          user can use the VUI to start the music. This is done because we
     *          do not yet have the content for Media Browse service. We will
     *          remove this error (a guiding hint) once the content is available.
     *     </li>
     * </ul>
     */
    @VisibleForTesting
    void initializeMediaSessionWithErrorForGuidingUser() {
        final AlexaApp app;
        try {
            app = AlexaApp.from(this);
        } catch (ClassCastException exception) {
            // This application doesn't provide AlexaApp. Ignore the auth state altogether.
            Log.i(TAG, "AlexaApp not found. Will show hint that ");
            mMediaSessionManager.setMediaSessionError(
                    PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR, getString(R.string.alexa_music_hint_1), null, null);
            return;
        }

        @
        NonNull AuthController authController = app.getRootComponent().getAuthController();
        @NonNull
        AlexaSetupController setupController = app.getRootComponent().getAlexaSetupController();

        mAuthSubscription = authController.observeAuthChangeOrLogOut().subscribe(authStatus -> {
            if (!authStatus.getLoggedIn()) {
                if (mVASelectionSubscription != null) {
                    mVASelectionSubscription.dispose(); // Let's not be distracted when logged out.
                    mVASelectionSubscription = null;
                }

                Intent loginIntent = setupController.createIntentForLaunchingLoginUI();
                PendingIntent pendingIntent =
                        PendingIntent.getActivity(this, 0, loginIntent, PendingIntent.FLAG_UPDATE_CURRENT);
                mMediaSessionManager.setMediaSessionError(PlaybackStateCompat.ERROR_CODE_AUTHENTICATION_EXPIRED,
                        getString(R.string.alexa_music_login_required), getString(R.string.alexa_music_login_text),
                        pendingIntent);
            } else {
                observeVoiceAssistantSelectionToSetMediaSessionState(setupController);
            }
        });
    }

    /**
     * Start observing the current voice assistant selection state. If Alexa becomes
     * currently selected voice assistant, then show hint for playing music as media
     * session error. Else, direct user to select Alexa as their voice assistant.
     *
     * @param setupController Alexa setup controller.
     */
    private void observeVoiceAssistantSelectionToSetMediaSessionState(@NonNull AlexaSetupController setupController) {
        mVASelectionSubscription = setupController.observeVoiceAssistantSelection().subscribe(alexaAsSelectedVA -> {
            if (alexaAsSelectedVA) {
                mMediaSessionManager.setMediaSessionError(PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR,
                        getString(R.string.alexa_music_hint_1), null, null);
            } else {
                Intent selectVAIntent = setupController.createIntentForLaunchingVoiceAssistantSwitchUI();
                PendingIntent pendingIntent =
                        PendingIntent.getActivity(this, 0, selectVAIntent, PendingIntent.FLAG_UPDATE_CURRENT);
                mMediaSessionManager.setMediaSessionError(PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR,
                        getString(R.string.alexa_music_alexa_not_selected_va),
                        getString(R.string.alexa_music_select_va), pendingIntent);
            }
        });
    }

    /**
     * Listener for Media Player Events.
     */
    @VisibleForTesting
    class MediaPlayerEventListener implements Player.EventListener {
        @Override
        public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
            MediaState mediaState = new MediaState(playWhenReady, playbackState);

            Log.d(TAG, "onPlayerStateChanged ready: " + playWhenReady + " state: " + playbackState);

            mAudioPlayerHandler.processMediaStateChange(mediaState.toAACSMediaState());
            handleLifecycleForMediaState(mediaState);
        }

        @Override
        public void onPlayerError(ExoPlaybackException error) {
            String message = error.toString();
            Log.w(TAG, "PLAYER ERROR: " + message);
            mAudioPlayerHandler.processMediaError(message);
        }
    }

    /**
     * Manages service lifecycle and schedule start/stop of the service based
     * on the signal it receives from outer class.
     *
     * It is required for the service to call startForeground within a short
     * window of receiving onStartCommand callback. We do not wish to keep this
     * service running if it is not doing anything useful. We do not wish to
     * stop service after processing each onStartCommand callback either because
     * we are likely to receive multiple back to back intents from AACS as part
     * of music domain protocol. This class maintains an idle timer to stop the
     * service if service isn't doing any useful work (i.e. playing music).
     */
    private class ServiceLifecycle implements Runnable {
        private boolean mServiceStarted = false;
        private boolean mServiceIsBusy = false;
        private Handler mHandler = new Handler();

        /**
         * Start the current service and also schedule a timer so that service
         * is stopped after idle timeout.
         */
        void startServiceWithIdleTimer() {
            if (!mServiceStarted) {
                startAlexaMediaService();
                mServiceStarted = true;
            }

            if (mServiceIsBusy) {
                return;
            }

            Log.v(TAG, "Resetting service auto-stop timer.");

            // Reset the timer for stopping the service.
            cancelServiceStopTimer();
            scheduleServiceStopTimer(SERVICE_IDLE_TIMEOUT_MS);
        }

        /**
         * Signal that service is now doing some useful work.
         * This method cancels "Stop after idle timeout" timer started with
         * startServiceWithIdleTimer method.
         */
        void markServiceBusy() {
            Log.i(TAG, "Marking service busy. Service would not stop until marked idle");

            mServiceIsBusy = true;
            cancelServiceStopTimer();
        }

        /**
         * Signal that service is done with useful work and can be stopped
         * after idle timeout has expired.
         */
        void markServiceIdle() {
            Log.i(TAG, "Marking service idle. Service would auto stop after timeout unless reset");

            mServiceIsBusy = false;
            startServiceWithIdleTimer();
        }

        /**
         * Schedule a timer which will stop the service after elapsed time.
         * Elapsed time is defined with SERVICE_IDLE_TIMEOUT_MS constant.
         */
        private void scheduleServiceStopTimer(long timeoutMs) {
            mHandler.postDelayed(this, timeoutMs);
        }

        /**
         * Cancels the timer to stop the service after elapsed time.
         */
        private void cancelServiceStopTimer() {
            mHandler.removeCallbacks(this);
        }

        /**
         * Stop Service timer is ticked.
         */
        @Override
        public void run() {
            Preconditions.checkArgument(
                    !mServiceIsBusy, "Could not have scheduled a stop task when service is still busy.");

            Log.i(TAG, "Service Stop on idle timer kicked in. Stopping the service.");

            stopAlexaMediaService();
            mServiceStarted = false;
        }
    }
}