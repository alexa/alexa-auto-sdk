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
package com.amazon.alexa.auto.media.browse;

import static android.app.Service.STOP_FOREGROUND_REMOVE;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.MediaDescriptionCompat;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.media.MediaBrowserServiceCompat;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthStatus;
import com.amazon.alexa.auto.apis.media.MediaSourceInfo;
import com.amazon.alexa.auto.apis.media.PlayerInfoCache;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.apps.common.util.FileUtil;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.Constants;
import com.amazon.alexa.auto.media.R;
import com.amazon.alexa.auto.media.ShutdownActionReceiver;
import com.amazon.alexa.auto.media.aacs.handlers.AudioPlayerHandler;
import com.amazon.alexa.auto.media.dependencies.AndroidModule;
import com.amazon.alexa.auto.media.dependencies.DaggerMediaComponent;
import com.amazon.alexa.auto.media.dependencies.MediaModule;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;
import com.amazon.alexa.auto.media.player.MediaState;
import com.amazon.alexa.auto.media.player.NotificationController;
import com.amazon.alexa.auto.media.session.MediaSessionManager;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.PlaybackException;
import com.google.android.exoplayer2.Player;

import org.json.JSONException;
import org.json.JSONObject;

import java.security.cert.PKIXRevocationChecker.Option;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * MediaBrowseService which is responsible for keeping track the media playback
 * and its metadata
 */
public class AlexaMediaBrowseService extends MediaBrowserServiceCompat {
    private static final String TAG = AlexaMediaBrowseService.class.getSimpleName();

    // Time service will run while idle. If the service doesn't get busy during
    // this time, then the service will be stopped.
    public static final long SERVICE_IDLE_TIMEOUT_MS = 1000 * 60 * 1; // 1 minute.
    private static final String MEDIA_ID = "media-id";
    private static final String ROOT_ID = "root-id";

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
    ShutdownActionReceiver mShutdownActionReceiver;

    // Internal Dependencies.
    private MediaPlayerEventListener mMediaPlayerEventListener;

    // State
    private final ServiceLifecycle mServiceLifecycle;

    // Subscriptions
    @Nullable
    private Disposable mAuthSubscription;
    @Nullable
    private Disposable mPlayerInfoSubscription;
    @Nullable
    private Disposable mAlexaEnablementSubscription;

    private ResumeOnBootSessionCallback mResumeOnBootCallback;

    private boolean mAlexaEnabled;
    private boolean mUserAuthenticated;

    /**
     * Creates an instance of Service.
     */
    public AlexaMediaBrowseService() {
        mMediaPlayerEventListener = new MediaPlayerEventListener();
        mServiceLifecycle = new ServiceLifecycle();
        mResumeOnBootCallback = new ResumeOnBootSessionCallback();
        mAlexaEnabled = false;
        mUserAuthenticated = false;
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

        mMediaPlayer.addListener(mMediaPlayerEventListener);
        mShutdownActionReceiver.onCreate(mMediaSessionManager);

        boolean loadedPreviousInfoFromCache = initializeSessionData();

        // If we were playing the last time the media session was closed and there was no player data just loaded
        // from the in-memory cache, it is most likely that this onCreate() is due to OS connecting to the browse
        // service on vehicle boot.
        // Set the playback state to PAUSED and include PLAY in actions so the Alexa media session is a candidate to
        // receive `onPlay` to resume automatically.
        //
        // Note that CarMediaService will use system config `R.integer.config_mediaBootAutoplay` to decide whether a
        // media app should resume on boot, so separate AACS config is not needed. There will be no `onPlay` from OS
        // in case of config_mediaBootAutoplay set not to autoplay and our auto-resume window will close naturally.
        int lastState = getSharedPreferences(Constants.MEDIA_PREFERENCES, Context.MODE_PRIVATE)
                                .getInt(Constants.PREFERENCE_PREVIOUS_PLAYBACK_STATE, PlaybackStateCompat.STATE_NONE);
        boolean resumeOnBoot = (lastState == PlaybackStateCompat.STATE_PLAYING) && !loadedPreviousInfoFromCache;

        if (resumeOnBoot) {
            Log.i(TAG, "Media was playing when last session closed. Initializing to paused state");
            mMediaSessionManager.initStateForResumeOnBoot();
            mMediaSessionManager.setupMediaSessionCallbacks(mResumeOnBootCallback);
            mResumeOnBootCallback.startAutoResumeWindow();
            mMediaSessionManager.activateMediaSession();
        } else {
            mMediaSessionManager.setupMediaSessionCallbacks();
            setUpAlexaStatusSubscriptions();
        }
        mServiceLifecycle.startServiceWithIdleTimer();
        setSessionToken(mMediaSessionManager.getMediaSession().getSessionToken());
    }

    /**
     * Initialize the session data in MediaSessionManager. If data from the last session is present on the in-memory
     * cache, load it into the session
     * @return True if data was loaded from cache
     */
    private boolean initializeSessionData() {
        Log.d(TAG, "initializeSessionData");
        @NonNull
        PlayerInfoCache playerInfoCache = AlexaApp.from(this).getRootComponent().getPlayerInfoCache();

        mMediaSessionManager.setIdleState();

        boolean loadedPreviousInfoFromCache = false;
        Optional<MediaSourceInfo> activeMediaSource = playerInfoCache.getMediaSourceInfo();
        Optional<RenderPlayerInfo> activeMediaMetadata = playerInfoCache.getPlayerInfo();
        loadedPreviousInfoFromCache = activeMediaSource.isPresent() && activeMediaMetadata.isPresent();

        if (loadedPreviousInfoFromCache) {
            Log.i(TAG, "Using cached URL and offset from last session");
            boolean sourceSet =
                    mAudioPlayerHandler.prepareUrl(activeMediaSource.get(), playerInfoCache.getPlaybackPosition());
            if (!sourceSet) {
                Log.e(TAG, "Failed to initialize URL from last session");
                loadedPreviousInfoFromCache = false;
            } else {
                Log.d(TAG, "Using cached metadata from last session");
                mMediaSessionManager.updateMediaSession(activeMediaMetadata.get());
                mMediaSessionManager.activateMediaSession();
                mServiceLifecycle.markServiceBusy();
            }
        } else {
            Log.d(TAG, "No stored source info from last session");
        }

        mPlayerInfoSubscription = playerInfoCache.observePlayerInfo().subscribe(playerInfo -> {
            if (playerInfo.isPresent()) {
                Log.v(TAG, "Updating media session data");
                mMediaSessionManager.updateMediaSession(playerInfo.get());
            } else {
                Log.v(TAG, "Setting media session data empty");
                mMediaSessionManager.updateMediaSession(null);
            }
        });

        return loadedPreviousInfoFromCache;
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
                String command = message.action;
                if (Action.AudioOutput.PREPARE.equals(command)) {
                    resetCachedPlaybackPosition();
                }
                try {
                    mAudioPlayerHandler.handleAACSCommand(message);
                } catch (Exception e) {
                    Log.e(TAG, "Cannot handle message. Error: ");
                    return;
                }

                if (Action.AudioOutput.PREPARE.equals(command) || Action.AudioOutput.PLAY.equals(command)
                        || Action.AudioOutput.RESUME.equals(command)) {
                    mMediaSessionManager.activateMediaSession();
                    // We know we need background work now, so we will mark the
                    // service as started.
                    mServiceLifecycle.markServiceBusy();
                }
            } else if (Topic.MEDIA_PLAYBACK_REQUESTER.equalsIgnoreCase(message.topic)) {
                if (mResumeOnBootCallback.isWindowActive) {
                    mResumeOnBootCallback.autoResumeResponseReceived(message.payload);
                } else {
                    Log.w(TAG, "MediaPlaybackRequestor response received but callback is null");
                }
            }
        });

        if (Intent.ACTION_MEDIA_BUTTON.equals(intent.getAction()) && mMediaSessionManager != null) {
            mMediaSessionManager.onMediaButtonIntentReceived(intent);
        }

        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        cachePlaybackPosition();

        mResumeOnBootCallback.onDestroy();

        if (mPlayerInfoSubscription != null) {
            mPlayerInfoSubscription.dispose();
        }
        if (mAlexaEnablementSubscription != null) {
            mAlexaEnablementSubscription.dispose();
        }

        if (mAuthSubscription != null) {
            mAuthSubscription.dispose();
        }

        prepareMediaSessionForRelease();
        mAudioPlayerHandler.close();
        mShutdownActionReceiver.onDestroy();
        mMediaSessionManager.releaseMediaSession();
        mMediaPlayer.getPlayer().release();
    }

    @Override
    public BrowserRoot onGetRoot(@NonNull String clientPackageName, int clientUid, Bundle rootHints) {
        Log.d(TAG, "onGetRoot. clientPackageName: " + clientPackageName + ", clientUid: " + clientUid);
        return new BrowserRoot(ROOT_ID, null);
    }

    @Override
    public void onLoadChildren(
            @NonNull final String parentMediaId, @NonNull final Result<List<MediaBrowserCompat.MediaItem>> result) {
        Log.d(TAG, "onLoadChildren");
        List<MediaBrowserCompat.MediaItem> mediaItems = new ArrayList<>();

        String displayText = getString(R.string.alexa_music_hint_1);
        if (!mAlexaEnabled) {
            displayText = getString(R.string.alexa_music_alexa_not_selected_va);
        } else if (!mUserAuthenticated) {
            displayText = getString(R.string.alexa_music_login_required);
        }

        MediaDescriptionCompat desc = new MediaDescriptionCompat.Builder()
                                              .setMediaId(MEDIA_ID)
                                              .setTitle(getString(R.string.alexa_media_app_name))
                                              .setSubtitle(displayText)
                                              .setIconUri(Uri.parse("android.resource://" + getPackageName() + "/"
                                                      + "drawable/media_item_place_holder"))
                                              .build();

        MediaBrowserCompat.MediaItem songList =
                new MediaBrowserCompat.MediaItem(desc, MediaBrowserCompat.MediaItem.FLAG_BROWSABLE);
        mediaItems.add(songList);
        result.sendResult(mediaItems);
    }

    private void prepareMediaSessionForRelease() {
        Log.i(TAG, "prepareMediaSessionForRelease");
        // remove listener so notifications from player don't set this service as busy again
        // when we're trying to kill it
        mMediaPlayer.removeListener(mMediaPlayerEventListener);
        mMediaSessionManager.deactivateMediaSession();
        // stop player after deactivateMediaSession() so session does not
        // receive callback and overwrite the last stored PlaybackState
        mMediaPlayer.stopAndFlushPlayer();
    }

    private void clearCachedPlayerData() {
        PlayerInfoCache playerInfoCache = AlexaApp.from(this).getRootComponent().getPlayerInfoCache();
        playerInfoCache.clearAllData();
    }

    private void resetCachedPlaybackPosition() {
        PlayerInfoCache playerInfoCache = AlexaApp.from(this).getRootComponent().getPlayerInfoCache();
        playerInfoCache.setPlaybackPosition(0);
    }

    private void cachePlaybackPosition() {
        PlayerInfoCache playerInfoCache = AlexaApp.from(this).getRootComponent().getPlayerInfoCache();
        long lastPosition = mMediaSessionManager.getPlaybackPosition();
        playerInfoCache.setPlaybackPosition(lastPosition);
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
            mMediaSessionManager.activateMediaSession();
            mServiceLifecycle.markServiceBusy();
        }
    }

    /**
     * Start the service in foreground with a notification.
     */
    private void startAlexaMediaService() {
        Log.i(TAG, "Starting Alexa Media Service in foreground");
        Notification notification = mNotificationController.createServiceStartNotification(getApplicationContext());
        startForeground(NotificationController.ALEXA_MEDIA_NOTIFICATION_ID, notification);
    }

    /**
     * Stop the AlexaMediaBrowseService so that system can destroy and reclaim
     * the memory.
     */
    private void stopAlexaMediaService() {
        Log.i(TAG, "Stopping Alexa Media Service");
        stopForeground(STOP_FOREGROUND_REMOVE);
        stopSelf();
    }

    /**
     * Listener for Media Player Events.
     */
    @VisibleForTesting
    class MediaPlayerEventListener implements Player.EventListener {
        @Override
        public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
            MediaState mediaState = new MediaState(playWhenReady, playbackState);
            Log.d(TAG, "onPlayerStateChanged; playWhenReady: " + playWhenReady + "; state: " + playbackState);
            cachePlaybackPosition();
            handleLifecycleForMediaState(mediaState);
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
        private boolean mServiceIsBusy = false;
        private Handler mHandler = new Handler();

        boolean isServiceBusy() {
            return mServiceIsBusy;
        }

        void stopServiceImmediately() {
            Log.d(TAG, "stopServiceImmediately");
            mServiceIsBusy = false;
            cancelServiceStopTimer();
            stopAlexaMediaService();
        }

        /**
         * Start the current service and also schedule a timer so that service
         * is stopped after idle timeout.
         */
        void startServiceWithIdleTimer() {
            startAlexaMediaService();

            if (mServiceIsBusy) {
                return;
            }

            // Reset the timer for stopping the service.
            Log.d(TAG, "Resetting service auto-stop timer.");
            cancelServiceStopTimer();
            scheduleServiceStopTimer(SERVICE_IDLE_TIMEOUT_MS);
        }

        /**
         * Signal that service is now doing some useful work.
         * This method cancels "Stop after idle timeout" timer started with
         * startServiceWithIdleTimer method.
         */
        void markServiceBusy() {
            if (!mServiceIsBusy) {
                Log.i(TAG, "Marking service busy. Service will not stop until marked idle");
                mServiceIsBusy = true;
            }
            cancelServiceStopTimer();
        }

        /**
         * Signal that service is done with useful work and can be stopped
         * after idle timeout has expired.
         */
        void markServiceIdle() {
            Log.i(TAG, "Marking service idle. Service will stop after the timeout unless started again");
            mServiceIsBusy = false;
            startServiceWithIdleTimer();
        }

        /**
         * Schedule a timer which will stop the service after elapsed time.
         * Elapsed time is defined with SERVICE_IDLE_TIMEOUT_MS constant.
         */
        private void scheduleServiceStopTimer(long timeoutMs) {
            Log.v(TAG, "Scheduling stop in " + timeoutMs + " milliseconds");
            mHandler.postDelayed(this, timeoutMs);
        }

        /**
         * Cancels the timer to stop the service after elapsed time.
         */
        private void cancelServiceStopTimer() {
            Log.v(TAG, "Stopping idle stop timer");
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
        }
    }

    /**
     * Temporary session callback handler for the duration of expected onPlay()
     * by system to resume playback on boot. Once the playback resume request completes
     * or the valid window for resuming elapses, reset the session callback to the
     * default from MediaSessionManager for all further playback.
     */
    @VisibleForTesting
    class ResumeOnBootSessionCallback extends MediaSessionCompat.Callback implements Runnable {
        private final String TAG = "ResumeOnBootSessionCallback";
        private static final long THRESHOLD_MS = 50000;
        private static final String MEDIA_PLAYBACK_REQUEST_STATUS = "mediaPlaybackRequestStatus";
        private static final String SUCCESS = "SUCCESS";
        private static final String FAILED_CAN_RETRY = "FAILED_CAN_RETRY";
        private static final String FAILED_TIMEOUT = "FAILED_TIMEOUT";
        private static final String ERROR = "ERROR";

        private Handler mHandler = new Handler();
        @Nullable
        private Disposable mAutoResumeSubscription;
        boolean isWindowActive = false;
        boolean isResumeRequestSent = false;

        @Override
        public void onPlay() {
            Log.i(TAG, "onPlay");
            if (!isResumeRequestSent) {
                mServiceLifecycle.markServiceBusy();
                mMediaSessionManager.setBufferingDuringResumeOnBoot();
                AlexaSetupController setupController =
                        AlexaApp.from(AlexaMediaBrowseService.this).getRootComponent().getAlexaSetupController();
                mAutoResumeSubscription =
                        setupController.observeAlexaCloudConnectionStatus().distinctUntilChanged().subscribe(
                                alexaCloudConnected -> {
                                    Log.d(TAG,
                                            "Alexa cloud connection status changed. connected=" + alexaCloudConnected);
                                    if (alexaCloudConnected) {
                                        // Send the resume AACS message and wait for async response
                                        Log.i(TAG, "Sending media resume request to AACS");
                                        mMediaSessionManager.requestMediaPlayback();
                                        isResumeRequestSent = true;
                                    }
                                });
            }
        }

        @Override
        public void onStop() {
            Log.d(TAG, "onStop");
            autoResumeWindowEnded(false);
            mMediaSessionManager.stopPlayback();
        }

        public void startAutoResumeWindow() {
            Log.v(TAG, "Starting auto-resume window. Scheduled to close in " + THRESHOLD_MS + " milliseconds");
            mHandler.postDelayed(this, THRESHOLD_MS);
            isWindowActive = true;
        }

        public void autoResumeResponseReceived(String responsePayload) {
            Log.v(TAG, "Auto-resume response was received");
            mHandler.removeCallbacks(this);

            try {
                JSONObject messagePayload = new JSONObject(responsePayload);
                String status = messagePayload.getString(MEDIA_PLAYBACK_REQUEST_STATUS);

                if (SUCCESS.equalsIgnoreCase(status)) {
                    Log.i(TAG, "RequestMediaPlayback was sent successfully");
                } else {
                    Log.e(TAG, "RequestMediaPlayback response was an error");
                    // We don't need to retry because we waited for connection before requesting
                }

            } catch (Exception exception) {
                Log.w(TAG,
                        String.format("Failed to parse RequestMediaPlayback response message: %s error: %s",
                                responsePayload, exception.getMessage()));
            }
            autoResumeWindowEnded(false);
        }

        public void autoResumeWindowEnded(boolean abortedForDestroy) {
            Log.d(TAG, "Auto-resume window is over");
            mHandler.removeCallbacks(this);
            isWindowActive = false;
            isResumeRequestSent = false;
            if (mAutoResumeSubscription != null) {
                mAutoResumeSubscription.dispose();
                mAutoResumeSubscription = null;
            }

            // deactivate session because we cannot accept any more commands until Alexa starts media.
            // Prepare message will re-activate it, if received.
            mMediaSessionManager.deactivateMediaSession();
            if (!abortedForDestroy) {
                mMediaSessionManager.setupMediaSessionCallbacks();
                mMediaSessionManager.setIdleState();
                mServiceLifecycle.markServiceIdle();
                setUpAlexaStatusSubscriptions();
            }
        }

        public void onDestroy() {
            if (isWindowActive) {
                autoResumeWindowEnded(true);
            }
        }

        @Override
        public void run() {
            autoResumeWindowEnded(false);
        }
    }

    private void setUpAlexaStatusSubscriptions() {
        @NonNull
        AlexaApp app = AlexaApp.from(this);
        @NonNull
        AuthController authController = app.getRootComponent().getAuthController();
        @NonNull
        AlexaSetupController setupController = app.getRootComponent().getAlexaSetupController();

        createAlexaEnablementSubscription(setupController, authController);
    }

    private void createAlexaEnablementSubscription(
            @NonNull AlexaSetupController setupController, @NonNull AuthController authController) {
        mAlexaEnablementSubscription = setupController.observeAACSReadiness().distinctUntilChanged().subscribe(alexaIsEnabled
                -> {
            Log.i(TAG, "Alexa enablement changed. alexaIsEnabled=" + alexaIsEnabled);
            mAlexaEnabled = alexaIsEnabled;
            notifyChildrenChanged(ROOT_ID);
            if (!alexaIsEnabled) {
                // We don't need the auth subscription util Alexa is enabled
                if (mAuthSubscription != null) {
                    mAuthSubscription.dispose();
                }

                boolean wasSessionActive = mMediaSessionManager.isMediaSessionActive();
                // deactivate session since Alexa is not usable
                mMediaSessionManager.deactivateMediaSession();

                // Error will show message about switching default voice assistant until enabling alexa outside
                // default assistant selection workflow is implemented
                Intent selectVAIntent = setupController.createIntentForLaunchingVoiceAssistantSwitchUI();
                PendingIntent pendingIntent = PendingIntent.getActivity(
                        this, 0, selectVAIntent, PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT);
                mMediaSessionManager.setMediaSessionError(PlaybackStateCompat.ERROR_CODE_AUTHENTICATION_EXPIRED,
                        getString(R.string.alexa_music_alexa_not_selected_va),
                        getString(R.string.alexa_music_select_va), pendingIntent, true);

                if (wasSessionActive) {
                    Log.i(TAG,
                            "Session is active but Alexa was disabled. Stopping work, clearing cached data, and closing session");
                    clearCachedPlayerData();
                    prepareMediaSessionForRelease();
                    mServiceLifecycle.stopServiceImmediately();
                }
            } else {
                mMediaSessionManager.activateMediaSession();
                // Re-do setup that was torn down while Alexa was disabled
                mMediaSessionManager.setupMediaSessionCallbacks();
                mMediaSessionManager.setIdleState(); // remove error state
                mMediaPlayer.addListener(mMediaPlayerEventListener);
                createAlexaAuthSubscription(setupController, authController);
            }
        });
    }

    private void createAlexaAuthSubscription(
            @NonNull AlexaSetupController setupController, @NonNull AuthController authController) {
        if (mAuthSubscription != null) {
            mAuthSubscription.dispose();
        }
        mAuthSubscription =
                authController.observeAuthChangeOrLogOut()
                        .map(AuthStatus::getLoggedIn)
                        .distinctUntilChanged()
                        .subscribe(loggedIn -> {
                            mUserAuthenticated = loggedIn;
                            notifyChildrenChanged(ROOT_ID);
                            if (!loggedIn) {
                                Log.d(TAG, "Auth status is not logged in");
                                // deactivate session since Alexa is not usable
                                mMediaSessionManager.deactivateMediaSession();

                                Intent loginIntent = setupController.createIntentForLaunchingVoiceAssistantSwitchUI();
                                PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, loginIntent,
                                        PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT);
                                mMediaSessionManager.setMediaSessionError(
                                        PlaybackStateCompat.ERROR_CODE_AUTHENTICATION_EXPIRED,
                                        getString(R.string.alexa_music_login_required),
                                        getString(R.string.alexa_music_login_text), pendingIntent, true);
                            } else {
                                Log.d(TAG, "Auth status is logged in");
                                mMediaSessionManager.setupMediaSessionCallbacks();
                                initializeSessionData();
                                mMediaSessionManager.activateMediaSession();
                            }
                        });
    }
}
