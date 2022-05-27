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
package com.amazon.alexa.auto.media.session;

import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioManager;
import android.media.session.PlaybackState;
import android.os.Bundle;
import android.os.SystemClock;
import android.support.v4.media.RatingCompat;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.media.session.MediaButtonReceiver;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.TemplateRuntimeConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.PlaybackControl;
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.R;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.PlaybackException;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.util.Log;
import com.google.android.exoplayer2.util.RepeatModeUtil;

import org.jetbrains.annotations.NotNull;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import io.reactivex.rxjava3.disposables.Disposable;

/**
 * Central component to tie in the {@link android.media.session.MediaSession} with current
 * media playback.
 * The class is responsible for managing the media session from media content provider
 * side. Following are some of those details:
 * <ul>
 *     <li>
 *         Publishes the current playback state and media controls available (including custom
 *         controls) to Media Session.
 *     </li>
 *     <li>
 *         Coordinates with {@link MediaMetadataProvider} to compute the metadata
 *         and publishes the metadata into Media Session.
 *     </li>
 *     <li>
 *         Listen for Media Session callbacks (such as pause, play, seek, skip next etc.)
 *         and use the {@link PlaybackController} to carry out these tasks.
 *     </li>
 * </ul>
 */
public class MediaSessionManager {
    private static final String TAG = MediaSessionManager.class.getSimpleName();

    // Fast Forward jump
    public static final int FAST_FORWARD_MS = 30000;
    // Rewind jump
    public static final int REWIND_MS = FAST_FORWARD_MS;

    // Constant Strings used
    private static final String LAST_PLAYBACK_STATE = "LAST_PLAYBACK_STATE";
    private static final String MEDIA_RESUME_SUPPORT = "MEDIA_RESUME_SUPPORT";
    private static final String FAILED_CAN_RETRY = "FAILED_CAN_RETRY";
    private static final String ERROR = "ERROR";
    private static final String FAILED_TIMEOUT = "FAILED_TIMEOUT";
    private static final String MEDIA_PLAYBACK_REQUEST_STATUS = "mediaPlaybackRequestStatus";
    private static final String STATUS = "status";

    @NonNull
    private final MediaPlayerExo mMediaPlayer;
    @NonNull
    private final MediaSessionCompat mMediaSession;
    @NonNull
    private final MediaMetadataProvider mMetadataProvider;
    @NonNull
    private final PlaybackController mControlDispatcher;
    @NonNull
    private final CustomActionProviders mCustomActionProviders;
    @NotNull
    private final SharedPreferences mSharedPreferences;

    // Map of custom action name and action object.
    @NonNull
    private final Map<String, CustomActionProvider> mCustomActions;

    @NonNull
    private final MediaPlayerEventListener mMediaEventListener;
    @NonNull
    private final MediaSessionCallbackHandler mMediaSessionCallback;

    // Last Updated Render Player Info
    @Nullable
    RenderPlayerInfo mLastRenderPlayerInfo;

    // Subscriptions.
    private Disposable mMetadataUpdateSubscription;

    // Shutdown state
    boolean isDeviceShuttingDown = false;

    // Media resume retry variables.
    private final ExecutorService executor;
    private boolean mMediaResumeSupported;
    private int reattempts = 10;
    private boolean isConnected = false;
    private boolean isMediaResumePending = false;

    // App Focus Management for Media Resume
    private AudioManager mAudioManager;
    private AudioFocusRequest mAudioFocusRequest;
    boolean isAppFocused;

    /**
     * Construct an instance of @c MediaSessionManager.
     *
     * @param mediaPlayer Media Player.
     * @param mediaSession Media session
     * @param metadataProvider Metadata for currently played media content.
     * @param controlDispatcher Control commands handler.
     * @param actionProviders Custom action providers.
     */
    public MediaSessionManager(@NonNull MediaPlayerExo mediaPlayer, @NonNull MediaSessionCompat mediaSession,
            @NonNull MediaMetadataProvider metadataProvider, @NonNull PlaybackController controlDispatcher,
            @NonNull CustomActionProviders actionProviders, @NonNull SharedPreferences sharedPreferences,
            @NotNull AudioManager audioManager) {
        this.mMediaPlayer = mediaPlayer;
        this.mMediaSession = mediaSession;
        this.mMetadataProvider = metadataProvider;
        this.mControlDispatcher = controlDispatcher;
        this.mCustomActionProviders = actionProviders;
        this.mSharedPreferences = sharedPreferences;

        this.mCustomActions = new HashMap<>();

        this.mMediaEventListener = new MediaPlayerEventListener();
        this.mMediaSessionCallback = new MediaSessionCallbackHandler();
        this.mAudioManager = audioManager;

        executor = Executors.newSingleThreadExecutor();
        initAppFocusManager();
    }

    /**
     * Media Resume starts its cycle when {@link MediaSessionCallbackHandler.onPlay()} is received
     * by the platform. This class request AudioFocus if Media Resume is expected. If Audio focus
     * is lost before Alexa gets connected and request media resume, this class assumes that
     * some other audio is playing and drops the plan of requesting media resume
     */
    private void initAppFocusManager() {
        AudioAttributes mAudioAttributes = new AudioAttributes.Builder()
                                                   .setUsage(AudioAttributes.USAGE_MEDIA)
                                                   .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                                   .build();

        AppFocusManager mAppFocusManager = new AppFocusManager();

        mAudioFocusRequest = new AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN)
                                     .setAudioAttributes(mAudioAttributes)
                                     .setWillPauseWhenDucked(false)
                                     .setOnAudioFocusChangeListener(mAppFocusManager)
                                     .build();
    }

    /**
     * Fetch the media session managed by this manager.
     *
     * @return Media Session.
     */
    @NonNull
    public MediaSessionCompat getMediaSession() {
        return mMediaSession;
    }

    /**
     * This method needs to be called on the reboot if media resume is expected.
     * It ensures that Automotive Android calls onPlay() if Alexa Media is last playing active media app
     */
    public void setupMediaResume() {
        if (mayAutoResume()) {
            setPlaybackStatePaused();
        }
    }

    /**
     * Activate the media session.
     */
    public void activateMediaSession() {
        if (!mMediaSession.isActive()) {
            mMediaSession.setActive(true);
        }
    }

    public void setupMediaSessionCallbacks() {
        Log.d(TAG, "setupMediaSessionCallbacks");
        mMediaSession.setCallback(mMediaSessionCallback);
        mMediaPlayer.getPlayer().addListener(mMediaEventListener);
    }

    /**
     * Deactivate the media session.
     */
    public void deactivateMediaSession() {
        if (!isDeviceShuttingDown) {
            mMediaSession.setActive(false);
            mMediaSession.setCallback(null);
        }
        mMediaPlayer.getPlayer().removeListener(mMediaEventListener);
    }

    /**
     * Destroy the media session.
     */
    public void destroyMediaSession() {
        if (!isDeviceShuttingDown) {
            getMediaSession().release();
        }
    }

    /**
     * Set the media session to error state.
     *
     * @param errorCode Error code (must be one of the PlaybackStateCompat.ERROR_CODE_XYZ).
     * @param errorStr Error description.
     * @param resolutionStr If there is a resolution of error, then the text which will
     *        appear on resolution button in the UI.
     * @param resolutionIntent If there is a resolution of error, then pending intent that
     *        is invoked when resolution button in the UI is clicked.
     */
    public void setMediaSessionError(int errorCode, @NonNull String errorStr, @Nullable String resolutionStr,
            @Nullable PendingIntent resolutionIntent) {
        Log.i(TAG, "Setting media session error with code:" + errorCode + " Error:" + errorStr);

        PlaybackStateCompat.Builder builder = new PlaybackStateCompat.Builder();
        builder.setState(PlaybackStateCompat.STATE_ERROR,
                       0, // Playback position.
                       0.0f, // Playback speed.
                       SystemClock.elapsedRealtime())
                .setErrorMessage(errorCode, errorStr);

        if (resolutionStr != null && resolutionIntent != null) {
            Bundle extras = new Bundle();
            extras.putString("android.media.extras.ERROR_RESOLUTION_ACTION_LABEL", resolutionStr);
            extras.putParcelable("android.media.extras.ERROR_RESOLUTION_ACTION_INTENT", resolutionIntent);
            builder.setExtras(extras);
        }
        activateMediaSession();
        Log.d(TAG, "setPlaybackState error");
        mMediaSession.setPlaybackState(builder.build());
    }

    /**
     * Update the media session based on @c RenderPlayerInfo.
     *
     * @param renderPlayerInfo Render Player Info.
     */
    public void updateMediaSession(@Nullable RenderPlayerInfo renderPlayerInfo) {
        mLastRenderPlayerInfo = renderPlayerInfo;
        updateShuffleRepeatMode();
        updateCustomActions();
        updatePlaybackState();
        updateMediaMetadata();
    }

    /**
     * Update the Repeat and Shuffle Mode based on the last render player info.
     */
    private void updateShuffleRepeatMode() {
        mMediaSession.setShuffleMode(PlaybackStateCompat.SHUFFLE_MODE_INVALID);
        mMediaSession.setRepeatMode(PlaybackStateCompat.REPEAT_MODE_INVALID);

        if (mLastRenderPlayerInfo == null) {
            return;
        }

        PlaybackControl shuffleControl =
                mLastRenderPlayerInfo.getControl(TemplateRuntimeConstants.CONTROL_NAME_SHUFFLE);
        PlaybackControl repeatControl = mLastRenderPlayerInfo.getControl(TemplateRuntimeConstants.CONTROL_NAME_LOOP);

        if (shuffleControl != null && shuffleControl.getEnabled()) {
            mMediaSession.setShuffleMode(shuffleControl.getSelected() ? PlaybackStateCompat.SHUFFLE_MODE_ALL
                                                                      : PlaybackStateCompat.SHUFFLE_MODE_NONE);
        }

        if (repeatControl != null && repeatControl.getEnabled()) {
            mMediaSession.setRepeatMode(repeatControl.getSelected() ? PlaybackStateCompat.REPEAT_MODE_ALL
                                                                    : PlaybackStateCompat.REPEAT_MODE_NONE);
        }
    }

    /**
     * Updates the custom actions for Media Session based on the last render player info.
     */
    private void updateCustomActions() {
        mCustomActions.clear();

        if (mLastRenderPlayerInfo == null) {
            return;
        }

        List<CustomActionProvider> providers =
                mCustomActionProviders.computeCustomActionProviders(mLastRenderPlayerInfo);
        for (CustomActionProvider provider : providers) {
            mCustomActions.put(provider.getCustomAction().getAction(), provider);
        }
    }

    /**
     * Update media metadata for Media Session based on the last render player info.
     */
    private void updateMediaMetadata() {
        if (mMetadataUpdateSubscription != null) {
            mMetadataUpdateSubscription.dispose();
        }

        if (mLastRenderPlayerInfo == null) {
            mMediaSession.setMetadata(null);
            return;
        }

        mMetadataUpdateSubscription =
                mMetadataProvider.updateMetadata(mLastRenderPlayerInfo).subscribe(metadataOptional -> {
                    metadataOptional.ifPresent(mMediaSession::setMetadata);
                });
    }

    /**
     * Updates media session playback state based on the last render player info.
     */
    public final void updatePlaybackState() {
        if (isDeviceShuttingDown) {
            Log.d(TAG, "Ignore updatePlaybackState");
            return;
        }
        PlaybackStateCompat.Builder builder = new PlaybackStateCompat.Builder();

        int playbackState = 0;
        ExoPlaybackException exoError = mMediaPlayer.getPlayer().getPlayerError();
        if (exoError != null) {
            playbackState = PlaybackStateCompat.STATE_ERROR;
            // TODO: Add error string in media session.
        } else {
            playbackState = mMediaPlayer.getMediaState().toMediaSessionState();
        }

        if (playbackState == PlaybackStateCompat.STATE_PLAYING) {
            activateMediaSession();
        }

        PlaybackParameters playbackParameters = mMediaPlayer.getPlayer().getPlaybackParameters();

        builder.setActiveQueueItemId(MediaSessionCompat.QueueItem.UNKNOWN_ID) // until media browse
                .setBufferedPosition(mMediaPlayer.getPlayer().getBufferedPosition())
                .setState(playbackState, mMediaPlayer.getPlayer().getCurrentPosition(), playbackParameters.speed,
                        SystemClock.elapsedRealtime());

        if (mLastRenderPlayerInfo == null) {
            mMediaSession.setPlaybackState(builder.build());
            return;
        }

        for (CustomActionProvider actionProvider : mCustomActions.values()) {
            PlaybackStateCompat.CustomAction action = actionProvider.getCustomAction();
            if (action != null) {
                builder.addCustomAction(action);
            }
        }

        builder.setActions(computePlaybackActions(mLastRenderPlayerInfo));

        PlaybackStateCompat playbackStateCompat = builder.build();
        mMediaSession.setPlaybackState(playbackStateCompat);
        storePlaybackState(playbackStateCompat.getState());
    }

    private void storePlaybackState(int playbackState) {
        if (playbackState == PlaybackStateCompat.STATE_ERROR) {
            // We don't want to store the error state because this can not be resumable condition
            return;
        }
        Log.d(TAG, String.format("storePlaybackState %d", playbackState));
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putInt(LAST_PLAYBACK_STATE, playbackState);
        editor.apply();
    }

    /**
     * Computes the supported playback actions (for Media Session).
     *
     * @param playerInfo Render Player Info needed to compute the actions.
     * @return Supported Media Session playback actions as flags.
     */
    private static long computePlaybackActions(@NonNull RenderPlayerInfo playerInfo) {
        long enabledActions = PlaybackStateCompat.ACTION_STOP;

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_SHUFFLE)) {
            enabledActions |= PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_LOOP)) {
            enabledActions |= PlaybackStateCompat.ACTION_SET_REPEAT_MODE;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_NEXT)) {
            enabledActions |= PlaybackStateCompat.ACTION_SKIP_TO_NEXT;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_PREVIOUS)) {
            enabledActions |= PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_SKIP_BACKWARD)) {
            enabledActions |= PlaybackStateCompat.ACTION_REWIND;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_SKIP_FORWARD)) {
            // Only when we can fast forward, do we allow to seek.
            enabledActions |= PlaybackStateCompat.ACTION_FAST_FORWARD | PlaybackStateCompat.ACTION_SEEK_TO;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_PLAY_PAUSE)) {
            enabledActions |= PlaybackStateCompat.ACTION_PLAY | PlaybackStateCompat.ACTION_PAUSE;
        }

        if (playerInfo.isControlEnabled(TemplateRuntimeConstants.CONTROL_NAME_THUMBS_UP)) {
            enabledActions |= PlaybackStateCompat.ACTION_SET_RATING;
        }

        return enabledActions;
    }

    /**
     * Tells if the playback action can be dispatched. It can only be dispatched
     * if current playback state supports the given playback action.
     *
     * @param action Action to test.
     * @return true if action can be dispatched.
     */
    private boolean canDispatchPlaybackAction(long action) {
        if (action == PlaybackStateCompat.ACTION_STOP)
            return true;

        return mLastRenderPlayerInfo != null && (computePlaybackActions(mLastRenderPlayerInfo) & action) != 0;
    }

    /**
     * Seek the Media Player with given offset.
     *
     * @param offsetMs Offset in milliseconds, negative offset would rewind
     *        the playback.
     */
    private void seekToOffset(long offsetMs) {
        Player player = mMediaPlayer.getPlayer();
        long positionMs = player.getCurrentPosition() + offsetMs;
        long durationMs = player.getDuration();
        if (durationMs != C.TIME_UNSET) {
            positionMs = Math.min(positionMs, durationMs);
        }
        positionMs = Math.max(positionMs, 0);
        mControlDispatcher.seekTo(
                mMediaPlayer.getPlayer(), mMediaPlayer.getPlayer().getCurrentWindowIndex(), positionMs);
    }

    public void onMediaButtonIntentReceived(Intent intent) {
        MediaButtonReceiver.handleIntent(mMediaSession, intent);
    }

    public boolean mayAutoResume() {
        // Set it true by default for the very first time where config might be not read yet.
        // Because of the '&&' this logic would not give unexpected result
        mMediaResumeSupported = mSharedPreferences.getBoolean(MEDIA_RESUME_SUPPORT, true);
        return mMediaResumeSupported
                && PlaybackState.STATE_PLAYING
                == mSharedPreferences.getInt(LAST_PLAYBACK_STATE, PlaybackState.STATE_NONE);
    }

    /**
     * If device is shutdown while playing the Alexa music,
     * set the playback state pause after reboot. This provides automotive android a
     * chance to resume the paused music.
     */
    private void setPlaybackStatePaused() {
        PlaybackStateCompat.Builder builder = new PlaybackStateCompat.Builder();
        builder.setActiveQueueItemId(MediaSessionCompat.QueueItem.UNKNOWN_ID) // until media browse
                .setState(PlaybackStateCompat.STATE_PAUSED, PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f,
                        SystemClock.elapsedRealtime())
                .setActions(PlaybackStateCompat.ACTION_PLAY | PlaybackStateCompat.ACTION_PLAY_FROM_MEDIA_ID);
        Log.d(TAG, "setPlaybackState paused");
        mMediaSession.setPlaybackState(builder.build());
    }

    public void shutdown() {
        Log.d(TAG, "device is shutting down");
        isDeviceShuttingDown = true;
    }

    public void handleMediaResumeResponse(AACSMessage message) {
        if (message.action.contains(Action.MediaPlaybackRequestor.MEDIA_PLAYBACK_RESPONSE)) {
            try {
                JSONObject messagePayload = new JSONObject(message.payload);
                Log.i(TAG, messagePayload.toString());
                if (messagePayload.getString(MEDIA_PLAYBACK_REQUEST_STATUS).equals(FAILED_CAN_RETRY)
                        && reattempts > 0) {
                    executor.execute(new ExecuteMediaPlaybackRequest(1000));
                } else if (ERROR.equalsIgnoreCase(messagePayload.getString(MEDIA_PLAYBACK_REQUEST_STATUS))
                        || FAILED_TIMEOUT.equalsIgnoreCase(messagePayload.getString(MEDIA_PLAYBACK_REQUEST_STATUS))) {
                    setMediaSessionError(PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR, "", null, null);
                    abandonAudioFocus();
                } else {
                    abandonAudioFocus();
                }
            } catch (Exception exception) {
                Log.w(TAG,
                        String.format("Failed to parse Media resume response message: %s error: %s", message.payload,
                                exception.getMessage()));
            }
        }
    }

    public void connectionStatusChanged(String payload) {
        Preconditions.checkArgument(payload != null && !payload.isEmpty());
        Log.d(TAG, payload);
        try {
            JSONObject jsonPayload = new JSONObject(payload);
            String status = jsonPayload.getString(STATUS);
            if (status.equalsIgnoreCase(AASBConstants.AlexaClient.ALEXA_CLIENT_STATUS_CONNECTED)) {
                isConnected = true;
                handleAlexaConnected();
            }
        } catch (Exception e) {
            Log.e(TAG, "failed to get connectionStatusChanged");
        }
    }

    private void handleAlexaConnected() {
        if (isMediaResumePending) {
            isMediaResumePending = false;
            executor.execute(new ExecuteMediaPlaybackRequest(0));
        } else {
            Log.w(TAG, "handleAlexaConnected resume request is not pending");
            abandonAudioFocus();
        }
    }

    /**
     * Set if media resume supported or not
     * @param mediaResumeSupported
     */
    public void mediaResumeSupported(boolean mediaResumeSupported) {
        mMediaResumeSupported = mediaResumeSupported;
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putBoolean(MEDIA_RESUME_SUPPORT, mediaResumeSupported);
        editor.apply();
    }

    /**
     * Listener for Media Player Events.
     */
    @VisibleForTesting
    class MediaPlayerEventListener implements Player.EventListener {
        @Override
        public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
            updatePlaybackState();
        }

        @Override
        public void onPlayerError(PlaybackException error) {
            updatePlaybackState();
        }

        @Override
        public void onIsPlayingChanged(boolean isPlaying) {
            updatePlaybackState();
        }
    }

    /**
     * Handler for Media Session callbacks.
     */
    @VisibleForTesting
    class MediaSessionCallbackHandler extends MediaSessionCompat.Callback {
        @Override
        public void onPlay() {
            Log.d(TAG, "onPlay isConnected:" + isConnected);
            if (mayAutoResume() && mLastRenderPlayerInfo == null) {
                if (isConnected) {
                    // Send Media Resume request
                    executor.execute(new ExecuteMediaPlaybackRequest(0));
                } else {
                    // Media Resume request will be sent after Alexa is authenticated and connected
                    isMediaResumePending = true;
                }

                mMediaSession.setPlaybackState(
                        new PlaybackStateCompat.Builder()
                                .setActiveQueueItemId(MediaSessionCompat.QueueItem.UNKNOWN_ID) // until media browse
                                .setState(PlaybackStateCompat.STATE_BUFFERING, PlaybackState.PLAYBACK_POSITION_UNKNOWN,
                                        1.0f, SystemClock.elapsedRealtime())
                                .setActions(PlaybackStateCompat.ACTION_PLAY | PlaybackStateCompat.ACTION_PAUSE)
                                .build());
                requestAudioFocus();
                return;
            }
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_PLAY)) {
                mControlDispatcher.setPlay(true);
            }
        }

        @Override
        public void onPause() {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_PAUSE)) {
                mControlDispatcher.setPlay(false);
            }
        }

        @Override
        public void onSeekTo(long positionMs) {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_SEEK_TO)) {
                mControlDispatcher.seekTo(
                        mMediaPlayer.getPlayer(), mMediaPlayer.getPlayer().getCurrentWindowIndex(), positionMs);
            }
        }

        @Override
        public void onFastForward() {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_FAST_FORWARD)) {
                if (mMediaPlayer.getPlayer().isCurrentWindowSeekable()) {
                    seekToOffset(FAST_FORWARD_MS);
                }
            }
        }

        @Override
        public void onRewind() {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_REWIND)) {
                seekToOffset(-REWIND_MS);
            }
        }

        @Override
        public void onStop() {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_STOP)) {
                mControlDispatcher.stop();
            }
        }

        @Override
        public void onSetShuffleMode(@PlaybackStateCompat.ShuffleMode int shuffleMode) {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE)) {
                boolean shuffleModeEnabled;
                switch (shuffleMode) {
                    case PlaybackStateCompat.SHUFFLE_MODE_ALL:
                    case PlaybackStateCompat.SHUFFLE_MODE_GROUP:
                        shuffleModeEnabled = true;
                        break;
                    case PlaybackStateCompat.SHUFFLE_MODE_NONE:
                    case PlaybackStateCompat.SHUFFLE_MODE_INVALID:
                    default:
                        shuffleModeEnabled = false;
                        break;
                }
                mControlDispatcher.setShuffleModeEnabled(shuffleModeEnabled);
            }
        }

        @Override
        public void onSetRepeatMode(@PlaybackStateCompat.RepeatMode int mediaSessionRepeatMode) {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_SET_REPEAT_MODE)) {
                @RepeatModeUtil.RepeatToggleModes
                int repeatMode;
                switch (mediaSessionRepeatMode) {
                    case PlaybackStateCompat.REPEAT_MODE_ALL:
                    case PlaybackStateCompat.REPEAT_MODE_GROUP:
                        repeatMode = Player.REPEAT_MODE_ALL;
                        break;
                    case PlaybackStateCompat.REPEAT_MODE_ONE:
                        repeatMode = Player.REPEAT_MODE_ONE;
                        break;
                    case PlaybackStateCompat.REPEAT_MODE_NONE:
                    case PlaybackStateCompat.REPEAT_MODE_INVALID:
                    default:
                        repeatMode = Player.REPEAT_MODE_OFF;
                        break;
                }
                mControlDispatcher.setRepeatMode(repeatMode);
            }
        }

        @Override
        public void onSkipToNext() {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_SKIP_TO_NEXT)) {
                mControlDispatcher.skipToNext();
            }
        }

        @Override
        public void onSkipToPrevious() {
            if (canDispatchPlaybackAction(PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS)) {
                mControlDispatcher.skipToPrevious();
            }
        }

        @Override
        public void onCustomAction(String action, @Nullable Bundle extras) {
            CustomActionProvider actionProvider = mCustomActions.get(action);
            if (actionProvider != null) {
                actionProvider.onCustomAction(action, extras);
            }
        }

        @Override
        public void onSetRating(RatingCompat rating) {
            mControlDispatcher.setRating(rating);
        }

        @Override
        public void onSetRating(RatingCompat rating, @Nullable Bundle extras) {
            mControlDispatcher.setRating(rating);
        }

        @Override
        public boolean onMediaButtonEvent(Intent mediaButtonEvent) {
            Log.d(TAG, "onMediaButtonEvent");
            onMediaButtonIntentReceived(mediaButtonEvent);
            return true;
        }
    }

    private class ExecuteMediaPlaybackRequest implements Runnable {
        int delayMs;
        { delayMs = 0; }

        private ExecuteMediaPlaybackRequest(int delay) {
            delayMs = delay;
        }

        @Override
        public void run() {
            if (delayMs > 0) {
                try {
                    Thread.sleep(delayMs);
                } catch (InterruptedException e) {
                }
            }
            Log.d(TAG, "Retrying for media resume");
            mControlDispatcher.requestMediaPlayback();
            reattempts--;
        }
    }

    // This code is to handle a case where auto resuming a media can be cancelled if driver plays
    // any other audio before the media resumes
    private void requestAudioFocus() {
        if (mAudioManager.requestAudioFocus(mAudioFocusRequest) == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            isAppFocused = true;
        }
    }

    private void abandonAudioFocus() {
        if (isAppFocused
                && mAudioManager.abandonAudioFocusRequest(mAudioFocusRequest)
                        == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            isAppFocused = false;
        }
    }

    private class AppFocusManager implements AudioManager.OnAudioFocusChangeListener {
        @Override
        public void onAudioFocusChange(int focusChange) {
            if (focusChange == AudioManager.AUDIOFOCUS_LOSS || focusChange == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT
                    || focusChange == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK) {
                isMediaResumePending = false;
                abandonAudioFocus();
            }
        }
    }
}
