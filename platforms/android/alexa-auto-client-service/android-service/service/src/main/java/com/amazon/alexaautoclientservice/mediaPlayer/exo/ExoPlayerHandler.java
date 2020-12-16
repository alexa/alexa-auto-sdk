/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.mediaPlayer.exo;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.amazon.aace.aasb.AASBStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.AASBConstants.AudioOutput.MutedState;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexaautoclientservice.mediaPlayer.AACSMediaPlayer;
import com.amazon.alexaautoclientservice.mediaPlayer.AudioFocusAttributes;
import com.amazon.alexaautoclientservice.mediaPlayer.EventReceiver;
import com.amazon.alexaautoclientservice.modules.alexaClient.AuthStateObserver;
import com.amazon.alexaautoclientservice.util.MediaPlayerUtil;
import com.google.android.exoplayer2.*;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;

import org.json.JSONObject;

import java.io.FileOutputStream;
import java.io.IOException;

@RequiresApi(api = Build.VERSION_CODES.O)
public class ExoPlayerHandler implements AACSMediaPlayer, AudioManager.OnAudioFocusChangeListener, AuthStateObserver {
    private static final String TAG = AACSConstants.AACS + "-" + ExoPlayerHandler.class.getSimpleName();
    private static final String ALEXA_MEDIA_FILE = "alexa_media";

    private static int WRITE_BUFFER_SIZE = 4096;

    private static float AUDIO_DUCK_LEVEL = 0.2f;
    private static float AUDIO_NORMAL_LEVEL = 1f;

    private final Context mContext;
    private final String mChannel;
    private final String mType;

    private final MediaSourceFactory mMediaSourceFactory;

    private boolean mRepeating;
    private boolean mPlaying;
    private boolean mPaused;
    private final EventReceiver mEventReceiver;

    private final AudioManager mAudioManager;
    private final AudioFocusAttributes mAudioFocusAttributes;
    private final AudioFocusRequest mExoPlayerAudioFocusRequest;

    private SimpleExoPlayer mPlayer;

    private float mVolume = 0.5f;
    private String mMutedState = MutedState.UNMUTED;
    private long mPosition;
    private long mLivePlaybackStartedTime;
    private long mLivePreviousTotalTimePlayed;
    private String mCurrentToken = "";
    private Object focusLock;
    private boolean playbackDelayed = false;
    private boolean resumeOnFocusGain = false;

    @RequiresApi(api = Build.VERSION_CODES.O)
    public ExoPlayerHandler(@NonNull Context context, @NonNull String channel, @NonNull String type,
            @NonNull EventReceiver eventReceiver) {
        mContext = context;
        mChannel = channel;
        mType = type;

        Handler handler = new Handler(Looper.getMainLooper());
        mMediaSourceFactory = new MediaSourceFactory(mContext, mChannel, handler);

        mRepeating = false;
        mPlaying = false;
        mPaused = false;
        mEventReceiver = eventReceiver;
        focusLock = new Object();

        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        mAudioFocusAttributes = new AudioFocusAttributes(type);
        AudioAttributes attributes = new AudioAttributes.Builder()
                                             .setUsage(mAudioFocusAttributes.mUsage)
                                             .setContentType(mAudioFocusAttributes.mContentType)
                                             .build();

        mExoPlayerAudioFocusRequest =
                new AudioFocusRequest.Builder(mAudioFocusAttributes.mFocusGain)
                        .setAudioAttributes(attributes)
                        .setOnAudioFocusChangeListener(this)
                        .setWillPauseWhenDucked(mAudioFocusAttributes.mWillPauseWhenDucked)
                        .setAcceptsDelayedFocusGain(mAudioFocusAttributes.mAcceptsDelayedFocusGain)
                        .build();

        initializePlayer();
    }

    private void initializePlayer() {
        mPlayer = ExoPlayerFactory.newSimpleInstance(mContext, new DefaultTrackSelector());
        mPlayer.addListener(new PlayerEventListener());
        mPlayer.setPlayWhenReady(false);
    }

    private void resetPlayer() {
        mPlayer.setRepeatMode(Player.REPEAT_MODE_OFF);
        mPlayer.setPlayWhenReady(false);
        mPlayer.stop(true);
        // reset live station offsets
        mLivePreviousTotalTimePlayed = 0;
    }

    public boolean isPlaying() {
        return mPlayer != null && mPlayer.getPlayWhenReady()
                && (mPlayer.getPlaybackState() == Player.STATE_BUFFERING
                        || mPlayer.getPlaybackState() == Player.STATE_READY);
    }

    //
    // Handle playback directives from Engine
    //

    @Override
    public void prepare(AASBStream stream, boolean repeating, String token) {
        Log.v(TAG, String.format("(%s) Handling prepare() given AASBStream.", mChannel));
        resetPlayer();
        mRepeating = repeating;
        mCurrentToken = token;
        try (FileOutputStream os = mContext.openFileOutput(ALEXA_MEDIA_FILE, Context.MODE_PRIVATE)) {
            byte[] buffer = new byte[WRITE_BUFFER_SIZE];
            int size;
            while (!stream.isClosed()) {
                while ((size = stream.read(buffer)) > 0) os.write(buffer, 0, size);
            }
        } catch (IOException e) {
            Log.e(TAG, String.format("Error occurred while writing to output stream. Error=%s", e.getMessage()));
        }

        try {
            Uri uri = Uri.fromFile(mContext.getFileStreamPath(ALEXA_MEDIA_FILE));
            MediaSource mediaSource = mMediaSourceFactory.createFileMediaSource(uri);
            mPlayer.prepare(mediaSource, true, false);
        } catch (Exception e) {
            Log.e(TAG, String.format("Error occurred while preparing media source. Error=%s", e.getMessage()));
            String message = e.getMessage() != null ? e.getMessage() : "";
            MediaPlayerUtil.sendMediaErrorMessage(
                    mEventReceiver, mCurrentToken, MediaConstants.MediaError.MEDIA_ERROR_UNKNOWN, message, mChannel);
        }
    }

    @Override
    public void prepare(String url, boolean repeating, String token) {
        Log.v(TAG, String.format("(%s) Handling prepare() given a URL.", mChannel));
        resetPlayer();
        mRepeating = repeating;
        mCurrentToken = token;
        Uri uri = Uri.parse(url);
        try {
            MediaSource mediaSource = mMediaSourceFactory.createHttpMediaSource(uri);
            mPlayer.prepare(mediaSource, true, false);
        } catch (Exception e) {
            String message = e.getMessage() != null ? e.getMessage() : "";
            Log.e(TAG, String.format("Error occurred while preparing mediaSource. Error=%s", message));
            MediaPlayerUtil.sendMediaErrorMessage(
                    mEventReceiver, mCurrentToken, MediaConstants.MediaError.MEDIA_ERROR_UNKNOWN, message, mChannel);
        }
    }

    @Override
    public void play() {
        Log.v(TAG, String.format("(%s) Handling play()", mChannel));
        int result = mAudioManager.requestAudioFocus(mExoPlayerAudioFocusRequest);
        synchronized (focusLock) {
            if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                mPlayer.setPlayWhenReady(true);
            } else if (result == AudioManager.AUDIOFOCUS_REQUEST_DELAYED) {
                playbackDelayed = true;
            } else if (result == AudioManager.AUDIOFOCUS_REQUEST_FAILED) {
                String message = "Audio focus request failed.";
                MediaPlayerUtil.sendMediaErrorMessage(mEventReceiver, mCurrentToken,
                        MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, message, mChannel);
            }
        }
    }

    @Override
    public boolean stop() {
        Log.v(TAG, String.format("(%s) Handling stop()", mChannel));
        synchronized (focusLock) {
            playbackDelayed = false;
            resumeOnFocusGain = false;
        }
        mAudioManager.abandonAudioFocusRequest(mExoPlayerAudioFocusRequest);
        mPaused = false;
        if (!mPlayer.getPlayWhenReady()) {
            // Player is already not playing. Notify Engine of stop
            onPlaybackStopped();
        } else
            mPlayer.setPlayWhenReady(false);
        return true;
    }

    @Override
    public boolean pause() {
        Log.v(TAG, String.format("(%s) Handling pause()", mChannel));
        mPaused = true;
        mPlayer.setPlayWhenReady(false);
        return true;
    }

    @Override
    public boolean resume() {
        Log.v(TAG, String.format("(%s) Handling resume()", mChannel));
        mPlayer.setPlayWhenReady(true);
        return true;
    }

    @Override
    public boolean setPosition(long position) {
        Log.v(TAG, String.format("(%s) Handling setPosition(%s)", mChannel, position));
        mPlayer.seekTo(position);
        return true;
    }

    @Override
    public long getPosition(String messageId) {
        mPosition = Math.abs(mPlayer.getCurrentPosition());

        if (mPlayer.isCurrentWindowDynamic()) {
            if (mPlaying) {
                mPosition = System.currentTimeMillis() - mLivePlaybackStartedTime + mLivePreviousTotalTimePlayed;
            } else {
                mPosition = mLivePreviousTotalTimePlayed;
            }
        }
        JSONObject payload = new JSONObject();
        try {
            payload.put("position", mPosition);
        } catch (Exception e) {
            Log.e(TAG, "failed to create getPositionReply JSON payload.");
        }
        Log.v(TAG, String.format("(%s) Handling getPosition(%s)", mChannel, mPosition));
        MediaPlayerUtil.sendEvent(mEventReceiver, messageId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_POSITION,
                payload.toString(), mChannel);
        return mPosition;
    }

    public long getDuration(String messageId) {
        long duration = mPlayer.getDuration();
        duration = (duration != C.TIME_UNSET) ? duration : -1;

        JSONObject payload = new JSONObject();
        try {
            payload.put("duration", duration);
        } catch (Exception e) {
            Log.e(TAG, "failed to create getDurationReply JSON payload.");
        }
        Log.v(TAG, String.format("(%s) Handling getDuration(%s)", mChannel, duration));
        MediaPlayerUtil.sendEvent(mEventReceiver, messageId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_DURATION,
                payload.toString(), mChannel);
        return duration;
    }

    @Override
    public void volumeChanged(float volume) {
        if (mVolume != volume) {
            Log.i(TAG, String.format("(%s) Handling volumeChanged(%s)", mChannel, volume));
            mVolume = volume;
            if (mMutedState.equals(MutedState.MUTED)) {
                mPlayer.setVolume(0);
            } else {
                mPlayer.setVolume(volume);
            }
        }
    }

    @Override
    public void mutedStateChanged(String state) {
        if (state != null && !state.equals(mMutedState)) {
            Log.i(TAG, String.format("(%s) Muted state changed to %s.", mChannel, state));
            mPlayer.setVolume(state.equals(MutedState.MUTED) ? 0 : mVolume);
            mMutedState = state;
        }
    }

    //
    // Handle ExoPlayer state changes and notify Engine
    //

    private void onPlaybackStarted() {
        Log.v(TAG, String.format("(%s) Media State Changed. STATE: PLAYING", mChannel));
        mLivePlaybackStartedTime = System.currentTimeMillis();
        mPlaying = true;
        mPaused = false;
        MediaPlayerUtil.sendMediaStateChangedMessage(
                mEventReceiver, mChannel, mCurrentToken, MediaConstants.MediaState.PLAYING);
    }

    private void onPlaybackStopped() {
        Log.v(TAG, String.format("(%s) Media State Changed. STATE: STOPPED", mChannel));
        // if triggered via pause, keep previous offset
        if (mPaused) {
            mLivePreviousTotalTimePlayed += System.currentTimeMillis() - mLivePlaybackStartedTime;
        }
        mPlaying = false;
        MediaPlayerUtil.sendMediaStateChangedMessage(
                mEventReceiver, mChannel, mCurrentToken, MediaConstants.MediaState.STOPPED);
    }

    private void onPlaybackFinished() {
        if (mRepeating) {
            mPlayer.seekTo(0);
            mPlayer.setRepeatMode(Player.REPEAT_MODE_ONE);

        } else {
            mAudioManager.abandonAudioFocusRequest(mExoPlayerAudioFocusRequest);
            mPlayer.setRepeatMode(Player.REPEAT_MODE_OFF);
            Log.v(TAG, String.format("(%s) Media State Changed. STATE: STOPPED", mChannel));
            mPlaying = false;
            MediaPlayerUtil.sendMediaStateChangedMessage(
                    mEventReceiver, mChannel, mCurrentToken, MediaConstants.MediaState.STOPPED);
        }
    }

    private void onPlaybackBuffering() {
        Log.v(TAG, String.format("(%s) Media State Changed. STATE: BUFFERING", mChannel));
        MediaPlayerUtil.sendMediaStateChangedMessage(
                mEventReceiver, mChannel, mCurrentToken, MediaConstants.MediaState.BUFFERING);
    }

    @Override
    public void cleanUp() {
        mPlayer.release();
        mPlayer = null;
    }

    private class PlayerEventListener extends Player.DefaultEventListener {
        @Override
        public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
            switch (playbackState) {
                case Player.STATE_ENDED:
                    if (playWhenReady)
                        onPlaybackFinished();
                    break;
                case Player.STATE_READY:
                    if (playWhenReady)
                        onPlaybackStarted();
                    else
                        onPlaybackStopped();
                    break;
                case Player.STATE_BUFFERING:
                    if (playWhenReady)
                        onPlaybackBuffering();
                    break;
                default:
                    // Disregard other states
                    break;
            }
        }

        @Override
        public void onPlayerError(ExoPlaybackException e) {
            String message;
            if (e.type == ExoPlaybackException.TYPE_SOURCE) {
                message = "ExoPlayer Source Error: " + e.getSourceException().getMessage();
            } else if (e.type == ExoPlaybackException.TYPE_RENDERER) {
                message = "ExoPlayer Renderer Error: " + e.getRendererException().getMessage();
            } else if (e.type == ExoPlaybackException.TYPE_UNEXPECTED) {
                message = "ExoPlayer Unexpected Error: " + e.getUnexpectedException().getMessage();
            } else {
                message = e.getMessage();
            }
            Log.e(TAG, "PLAYER ERROR: " + message);
            MediaPlayerUtil.sendMediaErrorMessage(mEventReceiver, mCurrentToken,
                    MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, message, mChannel);
        }
    }

    @Override
    public void onAudioFocusChange(int focusChange) {
        if (mPlayer == null) {
            Log.w(TAG, "Exoplayer out of scope");
            return;
        }
        Log.d(TAG, String.format("onAudioFocusChange for audioType=%s, new focus=%s", mType, focusChange));
        switch (focusChange) {
            case AudioManager.AUDIOFOCUS_GAIN:
                if (playbackDelayed || resumeOnFocusGain) {
                    synchronized (focusLock) {
                        playbackDelayed = false;
                        resumeOnFocusGain = false;
                    }
                    if (!mType.equals(AASBConstants.AudioOutput.AudioType.TTS)) {
                        mPlayer.setPlayWhenReady(true);
                        if (!mMutedState.equals(MutedState.MUTED)) {
                            mPlayer.setVolume(mVolume);
                        }
                    }
                }
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                if (mType.equals(AASBConstants.AudioOutput.AudioType.TTS)) {
                    MediaPlayerUtil.sendEvent(mEventReceiver, "", Topic.ALEXA_CLIENT,
                            Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "", mChannel);
                } else if (!mMutedState.equals(MutedState.MUTED)) {
                    mPlayer.setVolume(AUDIO_DUCK_LEVEL);
                }
                break;
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                synchronized (focusLock) {
                    resumeOnFocusGain = true;
                    playbackDelayed = false;
                }
                if (mType.equals(AASBConstants.AudioOutput.AudioType.TTS)) {
                    MediaPlayerUtil.sendEvent(mEventReceiver, "", Topic.ALEXA_CLIENT,
                            Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "", mChannel);
                } else {
                    mPaused = true;
                    mPlayer.setPlayWhenReady(false);
                }
                break;
            case AudioManager.AUDIOFOCUS_LOSS:
                synchronized (focusLock) {
                    resumeOnFocusGain = false;
                    playbackDelayed = false;
                }
                if (mType.equals(AASBConstants.AudioOutput.AudioType.ALARM)) {
                    MediaPlayerUtil.sendEvent(mEventReceiver, "", Topic.ALERTS, Action.Alerts.LOCAL_STOP, "", mChannel);
                } else if (mType.equals(AASBConstants.AudioOutput.AudioType.TTS)) {
                    MediaPlayerUtil.sendEvent(mEventReceiver, "", Topic.ALEXA_CLIENT,
                            Action.AlexaClient.STOP_FOREGROUND_ACTIVITY, "", mChannel);
                } else {
                    mPaused = false;
                    if (!mPlayer.getPlayWhenReady()) {
                        // Player is already not playing. Notify Engine of stop
                        onPlaybackStopped();
                    } else
                        mPlayer.setPlayWhenReady(false);
                }
                break;
            default:
                Log.w(TAG, "onAudioFocusChange, unknown focus gain type.");
        }
    }

    @Override
    public void onAuthStateChanged(@NonNull String authState) {
        if (AASBConstants.AlexaClient.AUTH_STATE_UNINITIALIZED.equals(authState)) {
            if (mType.equals(AASBConstants.AudioOutput.AudioType.ALARM)) {
                MediaPlayerUtil.sendEvent(mEventReceiver, "", Topic.ALERTS, Action.Alerts.LOCAL_STOP, "", mChannel);
            } else if (!mType.equals(AASBConstants.AudioOutput.AudioType.EARCON)) {
                if (mPlayer.getPlayWhenReady()) {
                    Log.i(TAG, String.format("(%s) Auth state is uninitialized. Stopping media player", mChannel));
                    mPlayer.setPlayWhenReady(false);
                }
            }
        }
    }

    @Override
    public long getNumBytesBuffered(String messageId) {
        long bufferedBytes = 0;
        Format audioFormat = mPlayer.getAudioFormat();
        if (audioFormat != null && audioFormat.bitrate == Format.NO_VALUE) {
            bufferedBytes = 0;
        } else {
            long bufferMs = mPlayer.getBufferedPosition() - mPlayer.getCurrentPosition();
            if (bufferMs >= 0) {
                bufferedBytes = bufferMs * audioFormat.bitrate / 1000 / 8;
            }
        }

        JSONObject payload = new JSONObject();
        try {
            payload.put("bufferedBytes", bufferedBytes);
        } catch (Exception e) {
            Log.e(TAG, "failed to create getNumBytesBufferedReply JSON payload.");
        }
        Log.v(TAG, String.format("(%s) Handling getNumBytesBuffered(%s)", mChannel, bufferedBytes));
        MediaPlayerUtil.sendEvent(mEventReceiver, messageId, Topic.AUDIO_OUTPUT,
                Action.AudioOutput.GET_NUM_BYTES_BUFFERED, payload.toString(), mChannel);
        return bufferedBytes;
    }
}
