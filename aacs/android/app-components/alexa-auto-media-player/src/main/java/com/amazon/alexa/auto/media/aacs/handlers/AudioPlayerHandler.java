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
package com.amazon.alexa.auto.media.aacs.handlers;

import android.net.Uri;
import android.util.Log;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;
import com.amazon.alexa.auto.apis.media.MediaSourceInfo;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.MusicStreamAttributeUpdater;
import com.amazon.alexa.auto.media.player.MediaPlayerAudioFocusController;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;
import com.google.android.exoplayer2.PlaybackException;
import com.google.android.exoplayer2.Player;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Handler for AudioPlayer commands coming from AACS.
 */
public class AudioPlayerHandler implements AutoCloseable, IMediaDuckingObserver {
    private final static String TAG = AudioPlayerHandler.class.getSimpleName();

    @Documented
    @Retention(RetentionPolicy.SOURCE)
    @IntDef({STATE_NONE, STATE_PREPARED, STATE_STARTING_PLAY, STATE_PLAYING, STATE_PAUSING, STATE_PAUSED,
            STATE_RESUMING, STATE_STOPPING, STATE_STOPPED, STATE_FINISHED, STATE_UNRECOVERABLE_ERROR})
    private @interface AudioPlayerState {}

    private static final int STATE_NONE = 0;
    private static final int STATE_PREPARED = 1;
    private static final int STATE_STARTING_PLAY = 2;
    private static final int STATE_PLAYING = 3;
    private static final int STATE_PAUSING = 4;
    private static final int STATE_PAUSED = 5;
    private static final int STATE_RESUMING = 6;
    private static final int STATE_STOPPING = 7;
    private static final int STATE_STOPPED = 8;
    private static final int STATE_FINISHED = 9;
    private static final int STATE_UNRECOVERABLE_ERROR = 10;

    // Dependencies.
    @NonNull
    private final AACSMessageSender mAACSSender;
    @NonNull
    private final MediaPlayerExo mTargetMediaPlayer;
    @NonNull
    private final MediaPlayerAudioFocusController mAudioFocusController;
    @NonNull
    private final PlaybackControlMessages mPlaybackControlMessages;
    @NonNull
    private final MusicStreamAttributeUpdater mMusicStreamAttributeUpdater;

    // Internal Dependencies.
    @NonNull
    private final AudioFocusPlayerController mAudioFocusPlayerController;
    @NonNull
    private MediaPlayerEventListener mMediaPlayerEventListener;

    // State.
    private String currentSourceToken;

    /**
     * Track the state Auto SDK Engine expects the AudioOutput interface implementation to be in to ensure accuracy of
     * AudioOutput.MediaStateChanged message reporting.
     */
    @AudioPlayerState
    private int mCurrentState;

    /**
     * Constructs the AACS AudioPlayer commands handler.
     *
     * @param sender To send messages back to AACS.
     * @param targetMediaPlayer Target media player.
     * @param audioFocusController Manages audio focus for playback.
     * @param playbackControlMessages For sending playback control messages to AACS.
     * @param musicStreamAttributeUpdater Helper to change volume for music stream.
     */
    public AudioPlayerHandler(@NonNull AACSMessageSender sender, @NonNull MediaPlayerExo targetMediaPlayer,
            @NonNull MediaPlayerAudioFocusController audioFocusController,
            @NonNull PlaybackControlMessages playbackControlMessages,
            @NonNull MusicStreamAttributeUpdater musicStreamAttributeUpdater) {
        mAACSSender = sender;
        mTargetMediaPlayer = targetMediaPlayer;
        mAudioFocusController = audioFocusController;
        mPlaybackControlMessages = playbackControlMessages;
        mMusicStreamAttributeUpdater = musicStreamAttributeUpdater;

        mAudioFocusPlayerController = new AudioFocusPlayerController();
        mAudioFocusController.setPlayerController(mAudioFocusPlayerController);
        setCurrentState(STATE_NONE);
        mMediaPlayerEventListener = new MediaPlayerEventListener();
        mTargetMediaPlayer.addListener(mMediaPlayerEventListener);
    }

    @Override
    public void close() {
        mAudioFocusController.close();
    }

    /**
     * Handle commands coming from AACS.
     *
     * @param message AACS Message.
     * @throw Exception from parsing or handling the message.
     */
    public void handleAACSCommand(@NonNull AACSMessage message) throws Exception {
        JSONObject payloadJSON;
        if (message.payload == null || message.payload.isEmpty()) {
            payloadJSON = null;
        } else {
            payloadJSON = new JSONObject(message.payload);
        }

        String channel = payloadJSON.optString(AASBConstants.AudioOutput.CHANNEL, "missing");
        if (!AASBConstants.AudioOutput.Channel.AUDIO_PLAYER.equals(channel)) {
            throw new WrongTargetChannelException(channel);
        }

        switch (message.action) {
            case Action.AudioOutput.PREPARE:
                handlePrepare(message.messageId, payloadJSON);
                break;
            case Action.AudioOutput.MAY_DUCK:
                handleMayDuck();
                break;
            case Action.AudioOutput.PAUSE:
                handlePause();
                break;
            case Action.AudioOutput.STOP:
                handleStop();
                break;
            case Action.AudioOutput.PLAY:
                handlePlay();
                break;
            case Action.AudioOutput.RESUME:
                handleResume();
                break;
            case Action.AudioOutput.START_DUCKING:
                mAudioFocusController.startDucking();
                break;
            case Action.AudioOutput.STOP_DUCKING:
                mAudioFocusController.stopDucking();
                break;
            case Action.AudioOutput.GET_POSITION:
                handleGetPosition(message.messageId);
                break;
            case Action.AudioOutput.GET_DURATION:
                handleGetDuration(message.messageId);
                break;
            case Action.AudioOutput.SET_POSITION:
                handleSetPosition(message.messageId, payloadJSON);
                break;
            case Action.AudioOutput.VOLUME_CHANGED:
                handleChangeVolume(message.messageId, payloadJSON);
                break;
            case Action.AudioOutput.MUTED_STATE_CHANGED:
                handleSetMuteState(message.messageId, payloadJSON);
                break;
            default:
                throw new Exception("Unrecognized action: " + message.action);
        }
    }

    /**
     * Send an AudioOutput.MediaStateChanged message to AACS and handle audio focus.
     *
     * @param state New media state to publish.
     */
    private void processMediaStateChange(String state) {
        Log.v(TAG, "processMediaStateChange");
        try {
            if (MediaConstants.MediaState.STOPPED.equals(state)) {
                Log.d(TAG, "Media state is stopped. Releasing focus");
                mAudioFocusController.relinquishAudioFocusIfCurrentlyAcquired();
            }
            // clang-format off
            String payload = new JSONStringer()
                 .object()
                    .key(MediaConstants.CHANNEL)
                        .value(AASBConstants.AudioOutput.Channel.AUDIO_PLAYER)
                    .key(MediaConstants.TOKEN)
                        .value(this.currentSourceToken)
                    .key(MediaConstants.STATE)
                        .value(state)
                .endObject().toString();
            // clang-format on

            Log.d(TAG, "publishing media state to AACS. state:" + state);
            mAACSSender.sendMessage(Topic.AUDIO_OUTPUT, Action.AudioOutput.MEDIA_STATE_CHANGED, payload);
        } catch (JSONException e) {
            Log.e(TAG, "failed to create mediaStateChanged JSON payload.");
        }
    }

    /**
     * Send an AudioOutput.MediaError message to AACS and release audio focus.
     * @param errorType Error type.
     * @param errMsg Error message.
     */
    private void processMediaError(String errorType, String errMsg) {
        try {
            Log.i(TAG, String.format("processMediaError; error type: %s; message: %s", errorType, errMsg));
            mAudioFocusController.relinquishAudioFocusIfCurrentlyAcquired();

            // clang-format off
            String payload = new JSONStringer()
                .object()
                    .key(MediaConstants.CHANNEL)
                        .value(AASBConstants.AudioOutput.Channel.AUDIO_PLAYER)
                    .key(MediaConstants.TOKEN)
                        .value(this.currentSourceToken)
                    .key(MediaConstants.MEDIA_ERROR)
                        .value(errorType)
                    .key(MediaConstants.ERROR_DESCRIPTION)
                        .value(errMsg)
                .endObject().toString();
            // clang-format on

            Log.d(TAG, "publishing media error to AACS. error message:" + errMsg);
            mAACSSender.sendMessage(Topic.AUDIO_OUTPUT, Action.AudioOutput.MEDIA_ERROR, payload);
        } catch (JSONException e) {
            Log.e(TAG, "failed to create mediaError JSON payload.");
        }
    }

    @Override
    public void reportClientDuckingState(boolean isDucked) {
        try {
            // clang-format off
            String payload = new JSONStringer()
                .object()
                    .key(MediaConstants.CHANNEL)
                        .value(AASBConstants.AudioOutput.Channel.AUDIO_PLAYER)
                    .key(MediaConstants.TOKEN)
                        .value(this.currentSourceToken)
                    .key(MediaConstants.FOCUS_ACTION)
                        .value(isDucked ? MediaConstants.AudioFocusEvent.REPORT_DUCKING_STARTED
                            : MediaConstants.AudioFocusEvent.REPORT_DUCKING_STOPPED)
                .endObject().toString();
            // clang-format on

            Log.d(TAG, "publishing ducking state to AACS. state:" + (isDucked ? "Ducked" : "Unducked"));
            mAACSSender.sendMessage(Topic.AUDIO_OUTPUT, Action.AudioOutput.AUDIO_FOCUS_EVENT, payload);
        } catch (JSONException e) {
            Log.e(TAG, "failed to create AudioFocusEvent JSON payload.");
        }
    }

    public boolean prepareUrl(@NonNull MediaSourceInfo sourceInfo, long position) {
        Log.d(TAG, "prepareUrl");
        String url = sourceInfo.getUrl();
        String token = sourceInfo.getToken();
        boolean repeating = sourceInfo.getRepeating();
        this.currentSourceToken = token;
        if (url.isEmpty()) {
            Log.e(TAG, "Empty URL");
            setCurrentState(STATE_NONE);
            processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INVALID_REQUEST, "empty url");
            return false;
        }
        if (token.isEmpty()) {
            Log.w(TAG, "Token is empty");
        }
        Log.d(TAG, String.format("Preparing media player with position: %d, URL: %s, token: %s", position, url, token));

        try {
            mTargetMediaPlayer.prepare(Uri.parse(url), position);
        } catch (Exception e) {
            Log.e(TAG, "failed to prepare, " + e.toString());
            setCurrentState(STATE_NONE);
            processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "prepare failed");
            return false;
        }
        setCurrentState(STATE_PREPARED);
        mAudioFocusController.setMixability(false, this);
        return true;
    }

    private void handlePrepare(@NonNull String messageId, @Nullable JSONObject payloadJson)
            throws JSONException, NullPayloadException, Exception {
        Log.i(TAG, "Prepare requested");
        if (payloadJson == null) {
            setCurrentState(STATE_NONE);
            processMediaError(
                    MediaConstants.MediaError.MEDIA_ERROR_INVALID_REQUEST, "AASB message missing payload content");
            throw new NullPayloadException("Prepare");
        }

        boolean repeating = payloadJson.optBoolean(AASBConstants.AudioOutput.REPEATING, false);
        String token = payloadJson.optString(AASBConstants.AudioOutput.TOKEN, "");
        String url = payloadJson.optString(AASBConstants.AudioOutput.URL, "");
        boolean success = prepareUrl(new MediaSourceInfo(url, token, repeating), 0);
        if (!success) {
            throw new Exception("Could not prepare URL");
        }
    }

    private void handleMayDuck() {
        Log.d(TAG, "handleMayDuck");
        mAudioFocusController.setMixability(true, this);
    }

    private void handlePause() {
        int currentState = getCurrentState();
        String stateStr = audioPlayerStateString(currentState);
        Log.i(TAG, "Pause requested. Current state: " + stateStr);
        switch (currentState) {
            case STATE_PREPARED:
                setCurrentState(STATE_PAUSED);
                processMediaStateChange(MediaConstants.MediaState.STOPPED);
                pausePlayerAndReleaseFocus();
                return;
            case STATE_FINISHED:
            case STATE_STARTING_PLAY:
            case STATE_PLAYING:
            case STATE_RESUMING:
                setCurrentState(STATE_PAUSING);
                pausePlayerAndReleaseFocus();
                return;
            case STATE_PAUSING:
            case STATE_STOPPING:
                // Will already notify Engine once pause/stop happens.
                // Nothing more to do.
                return;
            case STATE_PAUSED:
            case STATE_STOPPED:
                // Already in the intended state
                // Notify again since Engine is out of sync
                processMediaStateChange(MediaConstants.MediaState.STOPPED);
                return;
            case STATE_NONE:
                Log.e(TAG, "Pause requested before source was prepared");
                processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "");
                return;
            case STATE_UNRECOVERABLE_ERROR:
            default:
                processMediaError(
                        MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "already in an error state");
        }
    }

    private void handleStop() {
        int currentState = getCurrentState();
        String stateStr = audioPlayerStateString(currentState);
        Log.i(TAG, "Stop requested. Current state: " + stateStr);
        switch (currentState) {
            case STATE_PREPARED:
                setCurrentState(STATE_STOPPED);
                processMediaStateChange(MediaConstants.MediaState.STOPPED);
                pausePlayerAndReleaseFocus();
                return;
            case STATE_FINISHED:
            case STATE_STARTING_PLAY:
            case STATE_PLAYING:
            case STATE_RESUMING:
                setCurrentState(STATE_STOPPING);
                pausePlayerAndReleaseFocus();
                return;
            case STATE_PAUSING:
                setCurrentState(STATE_STOPPING);
            case STATE_STOPPING:
                // Will already notify Engine once pause/stop happens.
                // Nothing more to do.
                return;
            case STATE_PAUSED:
            case STATE_STOPPED:
                // Already in the intended state
                // Notify again since Engine is out of sync
                setCurrentState(STATE_STOPPED);
                processMediaStateChange(MediaConstants.MediaState.STOPPED);
                return;
            case STATE_NONE:
                Log.e(TAG, "Stop requested before source was prepared");
            case STATE_UNRECOVERABLE_ERROR:
                // Since Stop is intended to invalidate the source until next Prepare, this is recoverable,
                // so MediaStateChanged instead of MediaError in this case will keep Engine in a healthy state
                setCurrentState(STATE_STOPPED);
                processMediaStateChange(MediaConstants.MediaState.STOPPED);
                return;
            default:
                processMediaError(
                        MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "unrecognized current state");
        }
    }

    private void handlePlay() {
        int currentState = getCurrentState();
        String stateStr = audioPlayerStateString(currentState);
        Log.i(TAG, "Play requested. Current state: " + stateStr);
        switch (currentState) {
            case STATE_STARTING_PLAY:
            case STATE_RESUMING:
                // Will already notify Engine once playback starts.
                // Nothing more to do.
                return;
            case STATE_FINISHED:
                Log.w(TAG, "Can't play more while already finished");
            case STATE_PLAYING:
                // Notify Engine that playing started already since it's out of sync
                processMediaStateChange(MediaConstants.MediaState.PLAYING);
                return;
            case STATE_PAUSING:
            case STATE_PAUSED:
                Log.w(TAG, "Expected resume request when paused/pausing rather than play request");
                // fall through since effect is the same
            case STATE_PREPARED:
                setCurrentState(STATE_STARTING_PLAY);
                mAudioFocusController.resetAudioFocusRequest();
                mAudioFocusController.startPlaybackAfterAcquiringFocus();
                return;
            case STATE_STOPPING:
            case STATE_STOPPED:
                // Note that even though this implementation is currently using pause to handle AudioOutput.Stop,
                // per the AudioOutput.Stop API, Stop is a stop+flush request for the media item and not meant to be
                // resumed again.
                Log.e(TAG, "Can't play from stopped state without preparing a new source");
                processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INVALID_REQUEST, "play request not expected");
                return;
            case STATE_NONE:
                Log.e(TAG, "play requested before source was prepared");
                processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INVALID_REQUEST, "play request not expected");
                return;
            case STATE_UNRECOVERABLE_ERROR:
            default:
                setCurrentState(STATE_UNRECOVERABLE_ERROR);
                processMediaError(
                        MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "already in an error state");
        }
    }

    private void handleResume() {
        int currentState = getCurrentState();
        String stateStr = audioPlayerStateString(currentState);
        Log.i(TAG, "Resume requested. Current state: " + stateStr);
        switch (currentState) {
            case STATE_STARTING_PLAY:
                // Resume is not expected here, but fall through since effect is the same.
            case STATE_RESUMING:
                // Will already notify Engine once playback starts.
                // Nothing more to do.
                return;
            case STATE_FINISHED:
                Log.w(TAG, "Can't play more while already finished");
            case STATE_PLAYING:
                // Notify Engine that playing started already since it's out of sync
                processMediaStateChange(MediaConstants.MediaState.PLAYING);
                return;
            case STATE_PAUSING:
            case STATE_PAUSED:
            case STATE_PREPARED:
                setCurrentState(STATE_RESUMING);
                mAudioFocusController.startPlaybackAfterAcquiringFocus();
                return;
            case STATE_STOPPING:
            case STATE_STOPPED:
                // Note that even though this implementation is currently using pause to handle AudioOutput.Stop,
                // per the AudioOutput.Stop API, Stop is a stop+flush request for the media item and not meant to be
                // resumed again.
                Log.e(TAG, "Can't resume from stopped state without preparing a new source");
                processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INVALID_REQUEST, "play request not expected");
                return;
            case STATE_NONE:
                Log.e(TAG, "resume requested before source was prepared");
                processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INVALID_REQUEST, "resume request not expected");
                return;
            case STATE_UNRECOVERABLE_ERROR:
            default:
                setCurrentState(STATE_UNRECOVERABLE_ERROR);
                processMediaError(
                        MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "already in an error state");
        }
    }

    /**
     * Pause the player and release focus
     */
    private void pausePlayerAndReleaseFocus() {
        Log.d(TAG, "pausePlayerAndReleaseFocus");
        mTargetMediaPlayer.requestPause();
        mAudioFocusController.relinquishAudioFocusIfCurrentlyAcquired();
    }

    private void handleGetDuration(@NonNull String messageId) throws JSONException {
        long duration = mTargetMediaPlayer.getDuration();
        String replyPayload =
                new JSONStringer().object().key(MediaConstants.DURATION).value(duration).endObject().toString();

        mAACSSender.sendReplyMessage(messageId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_DURATION, replyPayload);
    }

    private void handleGetPosition(@NonNull String messageId) throws JSONException {
        long positionMs = mTargetMediaPlayer.getPosition();
        String replyPayload =
                new JSONStringer().object().key(MediaConstants.POSITION).value(positionMs).endObject().toString();

        Log.d(TAG, "Dispatching current media position to AACS. Position:" + positionMs);
        mAACSSender.sendReplyMessage(messageId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_POSITION, replyPayload);
    }

    private void handleSetPosition(@NonNull String messageId, @Nullable JSONObject payloadJson)
            throws JSONException, NullPayloadException {
        Log.d(TAG, "handleSetPosition");
        if (payloadJson == null)
            throw new NullPayloadException("SetPosition");

        long position = payloadJson.getLong(AASBConstants.AudioOutput.POSITION);
        Log.d(TAG, "Setting position:" + position);
        if (position > 0) {
            mTargetMediaPlayer.seekToPosition(position);
        }
    }

    private void handleChangeVolume(@NonNull String messageId, @Nullable JSONObject payloadJson)
            throws JSONException, NullPayloadException {
        Log.d(TAG, "handleChangeVolume");
        if (payloadJson == null)
            throw new NullPayloadException("ChangeVolume");

        double volume = payloadJson.getDouble(AASBConstants.AudioOutput.VOLUME);
        mMusicStreamAttributeUpdater.changeVolumeForMusicStream(volume);
    }

    private void handleSetMuteState(@NonNull String messageId, @Nullable JSONObject payloadJson)
            throws JSONException, NullPayloadException {
        Log.d(TAG, "handleSetMuteState");
        if (payloadJson == null)
            throw new NullPayloadException("SetMuteState");

        String state = payloadJson.getString(AASBConstants.AudioOutput.STATE);
        mMusicStreamAttributeUpdater.setMuteForMusicStream("MUTED".equals(state));
    }

    private void setCurrentState(@AudioPlayerState int state) {
        mCurrentState = state;
    }

    private @AudioPlayerState int getCurrentState() {
        return mCurrentState;
    }

    static String audioPlayerStateString(@AudioPlayerState int state) {
        switch (state) {
            case STATE_NONE:
                return "STATE_NONE";
            case STATE_PREPARED:
                return "STATE_PREPARED";
            case STATE_STARTING_PLAY:
                return "STATE_STARTING_PLAY";
            case STATE_PLAYING:
                return "STATE_PLAYING";
            case STATE_PAUSING:
                return "STATE_PAUSING";
            case STATE_PAUSED:
                return "STATE_PAUSED";
            case STATE_RESUMING:
                return "STATE_RESUMING";
            case STATE_STOPPING:
                return "STATE_STOPPING";
            case STATE_STOPPED:
                return "STATE_STOPPED";
            case STATE_FINISHED:
                return "STATE_FINISHED";
            case STATE_UNRECOVERABLE_ERROR:
                return "STATE_UNRECOVERABLE_ERROR";
            default:
                return "state not recognized";
        }
    }

    /**
     * Listener to the state of the player.
     */
    private class MediaPlayerEventListener implements Player.EventListener {
        @Player.State
        int mExoState;
        boolean mPlayWhenReady;

        MediaPlayerEventListener() {
            mExoState = mTargetMediaPlayer.getPlayer().getPlaybackState();
            mPlayWhenReady = mTargetMediaPlayer.getPlayer().getPlayWhenReady();
        }

        private String exoStateString(@Player.State int exoState) {
            switch (exoState) {
                case Player.STATE_ENDED:
                    return "STATE_ENDED";
                case Player.STATE_BUFFERING:
                    return "STATE_BUFFERING";
                case Player.STATE_READY:
                    return "STATE_READY";
                case Player.STATE_IDLE:
                    return "STATE_IDLE";
                default:
                    return "unknown state";
            }
        }

        private String exoPlayWhenReadyReasonString(@Player.PlayWhenReadyChangeReason int playWhenReadyReason) {
            switch (playWhenReadyReason) {
                case Player.PLAY_WHEN_READY_CHANGE_REASON_USER_REQUEST:
                    return "PLAY_WHEN_READY_CHANGE_REASON_USER_REQUEST";
                case Player.PLAY_WHEN_READY_CHANGE_REASON_AUDIO_FOCUS_LOSS:
                    return "PLAY_WHEN_READY_CHANGE_REASON_AUDIO_FOCUS_LOSS";
                case Player.PLAY_WHEN_READY_CHANGE_REASON_AUDIO_BECOMING_NOISY:
                    return "PLAY_WHEN_READY_CHANGE_REASON_AUDIO_BECOMING_NOISY";
                case Player.PLAY_WHEN_READY_CHANGE_REASON_REMOTE:
                    return "PLAY_WHEN_READY_CHANGE_REASON_REMOTE";
                case Player.PLAY_WHEN_READY_CHANGE_REASON_END_OF_MEDIA_ITEM:
                    return "PLAY_WHEN_READY_CHANGE_REASON_END_OF_MEDIA_ITEM";
                default:
                    return "unknown state";
            }
        }

        @Override
        public void onPlaybackStateChanged(int playbackState) {
            mExoState = playbackState;
            int trackedState = getCurrentState();
            String audioPlayerStateStr = audioPlayerStateString(trackedState);
            String exoStateStr = exoStateString(playbackState);
            Log.i(TAG,
                    String.format("onPlaybackStateChanged; playbackState: %s; currentTrackedState: %s", exoStateStr,
                            audioPlayerStateStr));
            switch (playbackState) {
                case Player.STATE_ENDED:
                    // Playback of current source finished. This is expected if we were playing or during an
                    // edge case while pausing or stopping (but not paused/stopped yet)
                    if (trackedState != STATE_PLAYING && trackedState != STATE_PAUSING
                            && trackedState != STATE_STOPPING) {
                        Log.w(TAG, "Reached finished in unexpected condition. Reporting state anyway");
                    }
                    setCurrentState(STATE_FINISHED);
                    processMediaStateChange(MediaConstants.MediaState.STOPPED);
                    break;
                case Player.STATE_BUFFERING:
                    if (mPlayWhenReady) {
                        // Buffer underrun reached while playing (or edge case while pausing or stopping but not
                        // paused/stopped yet)
                        processMediaStateChange(MediaConstants.MediaState.BUFFERING);
                    } else {
                        // Initial buffering during/after preparing source is expected.
                        if (trackedState != STATE_PREPARED && trackedState != STATE_NONE
                                && trackedState != STATE_FINISHED && trackedState != STATE_STOPPED) {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    }
                    break;
                case Player.STATE_READY:
                    if (mPlayWhenReady) {
                        // buffer was refilled after underrun while playing
                        if (trackedState == STATE_STARTING_PLAY || trackedState == STATE_RESUMING
                                || trackedState == STATE_PLAYING) {
                            setCurrentState(STATE_PLAYING);
                            processMediaStateChange(MediaConstants.MediaState.PLAYING);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    } else {
                        // indicates buffer refilled if paused/pausing while buffering
                        Log.v(TAG, "no change to track for current state");
                    }
                    break;
                case Player.STATE_IDLE:
                    if (mPlayWhenReady) {
                        Log.w(TAG, "Unexpected condition reached");
                    } else {
                        Log.v(TAG, "no change to track for current state");
                    }
                    break;
                default:
                    Log.w(TAG,
                            String.format("Unrecognized new player state %d. Current tracked state is %d",
                                    playbackState, trackedState));
                    break;
            }
        }

        @Override
        public void onPlayWhenReadyChanged​(boolean playWhenReady, int reason) {
            mPlayWhenReady = playWhenReady;
            int trackedState = getCurrentState();
            String audioPlayerStateStr = audioPlayerStateString(trackedState);
            String exoStateStr = exoStateString(mExoState);
            Log.d(TAG,
                    String.format(
                            "onPlayWhenReadyChanged​; playWhenReady: %b; reason: %d; currentTrackedState: %s; currentExoState: %s",
                            playWhenReady, reason, audioPlayerStateStr, exoStateStr));

            switch (mExoState) {
                case Player.STATE_ENDED:
                    if (playWhenReady) {
                        // Player was asked to play while already finished. This is not expected
                        if (trackedState != STATE_FINISHED) {
                            Log.w(TAG, "Reporting finished state to Engine in unexpected condition");
                            setCurrentState(STATE_FINISHED);
                            processMediaStateChange(MediaConstants.MediaState.STOPPED);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    } else {
                        // Player was asked to pause while already finished, or it was set to pause state before
                        // preparing a new item. these are normal conditions.
                        if (trackedState == STATE_FINISHED || trackedState == STATE_NONE
                                || trackedState == STATE_PREPARED) {
                            break;
                        }
                        if (trackedState == STATE_PAUSING || trackedState == STATE_STOPPING) {
                            setCurrentState(STATE_FINISHED);
                            processMediaStateChange(MediaConstants.MediaState.STOPPED);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    }
                    break;
                case Player.STATE_BUFFERING:
                    if (playWhenReady) {
                        // Player entered buffer underrun when asked to play. This is normal
                        if (trackedState == STATE_STARTING_PLAY || trackedState == STATE_RESUMING) {
                            processMediaStateChange(MediaConstants.MediaState.BUFFERING);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    } else {
                        // Player was asked to pause while buffering. This is normal
                        if (trackedState == STATE_PAUSING) {
                            setCurrentState(STATE_PAUSED);
                            processMediaStateChange(MediaConstants.MediaState.STOPPED);
                        } else if (trackedState == STATE_STOPPING) {
                            setCurrentState(STATE_STOPPED);
                            processMediaStateChange(MediaConstants.MediaState.STOPPED);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    }
                    break;
                case Player.STATE_READY:
                    if (playWhenReady) {
                        // Player started playing when requested
                        if (trackedState == STATE_STARTING_PLAY || trackedState == STATE_RESUMING) {
                            setCurrentState(STATE_PLAYING);
                            processMediaStateChange(MediaConstants.MediaState.PLAYING);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    } else {
                        // Player paused when requested
                        if (trackedState == STATE_PAUSING) {
                            setCurrentState(STATE_PAUSED);
                            processMediaStateChange(MediaConstants.MediaState.STOPPED);
                        } else if (trackedState == STATE_STOPPING) {
                            setCurrentState(STATE_STOPPED);
                            processMediaStateChange(MediaConstants.MediaState.STOPPED);
                        } else {
                            Log.w(TAG, "Unexpected condition reached");
                        }
                    }
                    break;
                case Player.STATE_IDLE:
                    // No source was set yet. Changes to onPlayWhenReady are not expected
                    Log.w(TAG, "Unexpected condition reached. Source not set yet");
                    break;
                default:
                    Log.e(TAG, "Unrecognized state reached in switch");
                    break;
            }
        }

        @Override
        public void onPlayerError(PlaybackException error) {
            int trackedState = getCurrentState();
            String audioPlayerStateStr = audioPlayerStateString(trackedState);
            Log.i(TAG,
                    String.format("onPlayerError; error: %s; currentTrackedState: %s", error.toString(),
                            audioPlayerStateStr));
            setCurrentState(STATE_UNRECOVERABLE_ERROR);
            processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, error.toString());
        }
    }

    /**
     * PlayerController for Audio Focus.
     */
    private class AudioFocusPlayerController implements MediaPlayerAudioFocusController.PlayerController {
        @Override
        public void startPlaybackNow() {
            int currentState = getCurrentState();
            if (currentState == STATE_RESUMING || currentState == STATE_STARTING_PLAY) {
                mTargetMediaPlayer.requestPlay();
            } else {
                Log.w(TAG, "Doing nothing. Starting playback is not expected in current state: " + currentState);
            }
        }

        @Override
        public void requestResumingPlayback() {
            int currentState = getCurrentState();
            // If focus manager paused playback, it did so with PlaybackController.ButtonPressed AASB message with state
            // PAUSE (since Auto SDK does not yet have an AASB message to report an external focus event taking focus
            // transiently).
            // Note that requestResumingPlayback() is only expected while stopping/stopped, not pausing/paused strictly
            // because the response to PlaybackController.ButtonPressed PAUSE is AudioOutput.Stop, not AudioOutput.Pause
            if (currentState == STATE_STOPPING || currentState == STATE_STOPPED) {
                mPlaybackControlMessages.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PLAY);
            } else {
                Log.i(TAG, "Ignoring requestResumingPlayback. Not expected in current state " + currentState);
            }
        }

        @Override
        public void requestPausePlayback() {
            int currentState = getCurrentState();
            // Use PlaybackController.ButtonPressed AASB message since Auto SDK does not yet have an AASB message to
            // report an external focus event taking focus transiently
            if (currentState == STATE_RESUMING || currentState == STATE_STARTING_PLAY || currentState == STATE_PLAYING
                    || currentState == STATE_PREPARED) {
                mPlaybackControlMessages.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PAUSE);
            } else {
                Log.i(TAG, "Ignoring requestPausePlayback. Not expected in current state " + currentState);
            }
        }

        @Override
        public void requestStopPlayback() {
            int currentState = getCurrentState();
            // Use PlaybackController.ButtonPressed AASB message since Auto SDK does not yet have an AASB message to
            // report an external focus event taking focus permanently
            if (currentState == STATE_RESUMING || currentState == STATE_STARTING_PLAY || currentState == STATE_PLAYING
                    || currentState == STATE_PREPARED) {
                mPlaybackControlMessages.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PAUSE);
            } else {
                Log.i(TAG, "Ignoring requestStopPlayback. Not playing. Current state: " + currentState);
            }
        }

        @Override
        public void adjustPlaybackVolume(float multiplier) {
            mTargetMediaPlayer.setVolume(multiplier);
        }

        @Override
        public void failedToAcquireFocus() {
            String errorMsg = "Failed to acquire audio focus";
            int currentState = getCurrentState();
            if (currentState == STATE_RESUMING || currentState == STATE_STARTING_PLAY) {
                processMediaError(MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, errorMsg);
            } else {
                Log.w(TAG,
                        "Doing nothing. Audio focus was not expected to be needed in current state: " + currentState);
            }
        }
    }

    private class WrongTargetChannelException extends Exception {
        public WrongTargetChannelException(String channel) {
            super(String.format("Message is for channel %s, but this class only handles AudioPlayer", channel));
        }
    }

    private class NullPayloadException extends Exception {
        public NullPayloadException(String messageName) {
            super(String.format("Expected payload for %s message was null or empty", messageName));
        }
    }
}
