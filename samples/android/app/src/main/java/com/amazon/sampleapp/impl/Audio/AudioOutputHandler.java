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

package com.amazon.sampleapp.impl.Audio;

import android.content.Context;
import android.media.AudioManager;
import android.media.audiofx.Equalizer;
import android.net.Uri;
import android.os.Handler;

import com.amazon.aace.alexa.AlexaClient.AuthError;
import com.amazon.aace.alexa.AlexaClient.AuthState;
import com.amazon.aace.alexa.EqualizerController;
import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioStream;
import com.amazon.sampleapp.impl.AlexaClient.AuthStateObserver;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler.EqualizerProvider;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.audio.AudioRendererEventListener;
import com.google.android.exoplayer2.decoder.DecoderCounters;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Map;

public class AudioOutputHandler
        extends AudioOutput implements Releasable, EqualizerControllerHandler.EqualizerProvider {
    private static final String sTag = AudioOutputHandler.class.getSimpleName();
    private static final String sFileName = "alexa_media"; // Note: not thread safe
    private static final long sSkipThresholdInMS = 1500; // 1500 ms

    private final Context mContext;
    private final LoggerHandler mLogger;
    private final String mName;
    private final MediaSourceFactory mMediaSourceFactory;
    private SimpleExoPlayer mPlayer;
    private boolean mRepeating;

    private float mVolume = 0.5f;
    private MutedState mMutedState = MutedState.UNMUTED;

    private Timeline.Period mPeriod;
    private Timeline.Window mWindow;
    private long mPosition;

    private long mLivePlaybackStartedTime;
    private long mLivePreviousTotalTimePlayed;
    private boolean mPlaying;
    private boolean mPaused;
    private Equalizer mEqualizer = null;

    AudioOutputHandler(Context context, LoggerHandler logger, String name, Handler handler) {
        mContext = context;
        mLogger = logger;
        mName = name;
        mMediaSourceFactory = new MediaSourceFactory(mContext, mLogger, mName, handler);
        mRepeating = false;
        mPlaying = false;
        mPaused = false;
        mPeriod = new Timeline.Period();
        mWindow = new Timeline.Window();

        initializePlayer();
    }

    private void initializePlayer() {
        mPlayer = ExoPlayerFactory.newSimpleInstance(mContext, new DefaultTrackSelector());
        mPlayer.addListener(new PlayerEventListener());
        mPlayer.setPlayWhenReady(false);
        mPlayer.addAudioDebugListener(new AudioRendererListener());
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
    public boolean prepare(AudioStream stream, boolean repeating) {
        mLogger.postVerbose(sTag, String.format("(%s) Handling prepare()", mName));
        resetPlayer();
        mRepeating = repeating;
        try (FileOutputStream os = mContext.openFileOutput(sFileName, Context.MODE_PRIVATE)) {
            byte[] buffer = new byte[4096];
            int size;
            while (!stream.isClosed()) {
                while ((size = stream.read(buffer)) > 0) os.write(buffer, 0, size);
            }
        } catch (IOException e) {
            mLogger.postError(sTag, e);
            return false;
        }

        try {
            Uri uri = Uri.fromFile(mContext.getFileStreamPath(sFileName));
            MediaSource mediaSource = mMediaSourceFactory.createFileMediaSource(uri);
            mPlayer.prepare(mediaSource, true, false);
            return true;
        } catch (Exception e) {
            mLogger.postError(sTag, e.getMessage());
            String message = e.getMessage() != null ? e.getMessage() : "";
            mediaError(MediaError.MEDIA_ERROR_UNKNOWN, message);
            return false;
        }
    }

    @Override
    public boolean prepare(String url, boolean repeating) {
        mLogger.postVerbose(sTag, String.format("(%s) Handling prepare(url) (%s)", mName, url));
        resetPlayer();
        mRepeating = repeating;
        Uri uri = Uri.parse(url);
        try {
            MediaSource mediaSource = mMediaSourceFactory.createHttpMediaSource(uri);
            mPlayer.prepare(mediaSource, true, false);
            return true;
        } catch (Exception e) {
            String message = e.getMessage() != null ? e.getMessage() : "";
            mLogger.postError(sTag, message);
            mediaError(MediaError.MEDIA_ERROR_UNKNOWN, message);
            return false;
        }
    }

    @Override
    public boolean play() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling play()", mName));
        mPlayer.setPlayWhenReady(true);
        return true;
    }

    @Override
    public boolean stop() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling stop()", mName));
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
        mLogger.postVerbose(sTag, String.format("(%s) Handling pause()", mName));
        mPaused = true;
        mPlayer.setPlayWhenReady(false);
        return true;
    }

    @Override
    public boolean resume() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling resume()", mName));
        mPlayer.setPlayWhenReady(true);
        return true;
    }

    @Override
    public boolean setPosition(long position) {
        mLogger.postVerbose(sTag, String.format("(%s) Handling setPosition(%s)", mName, position));
        mPlayer.seekTo(position);
        return true;
    }

    @Override
    public long getPosition() {
        mPosition = Math.abs(mPlayer.getCurrentPosition());

        if (mPlayer.isCurrentWindowDynamic()) {
            if (mPlaying) { // not during pause
                mPosition = System.currentTimeMillis() - mLivePlaybackStartedTime + mLivePreviousTotalTimePlayed;
            } else {
                mPosition = mLivePreviousTotalTimePlayed;
            }
        }
        mLogger.postVerbose(sTag, String.format("(%s) Handling getPosition(%s)", mName, mPosition));
        return mPosition;
    }

    @Override
    public long getDuration() {
        long duration = mPlayer.getDuration();
        return duration != C.TIME_UNSET ? duration : TIME_UNKNOWN;
    }

    @Override
    public long getNumBytesBuffered() {
        Format audioFormat = mPlayer.getAudioFormat();
        if (audioFormat.bitrate == Format.NO_VALUE) {
            return 0;
        }
        long bufferMs = mPlayer.getBufferedPosition() - mPlayer.getCurrentPosition();
        if (bufferMs < 0) {
            return 0;
        }
        return bufferMs * audioFormat.bitrate / 1000 / 8;
    }

    @Override
    public boolean volumeChanged(float volume) {
        if (mVolume != volume) {
            mLogger.postInfo(sTag, String.format("(%s) Handling volumeChanged(%s)", mName, volume));
            mVolume = volume;
            if (mMutedState == MutedState.MUTED) {
                mPlayer.setVolume(0);
            } else {
                mPlayer.setVolume(volume);
            }
        }
        return true;
    }

    @Override
    public boolean mutedStateChanged(MutedState state) {
        if (state != mMutedState) {
            mLogger.postInfo(sTag, String.format("Muted state changed (%s) to %s.", mName, state));
            mPlayer.setVolume(state == MutedState.MUTED ? 0 : mVolume);
            mMutedState = state;
        }
        return true;
    }

    //
    // Handle ExoPlayer state changes and notify Engine
    //

    private void onPlaybackStarted() {
        mLogger.postVerbose(sTag, String.format("(%s) Media State Changed. STATE: PLAYING", mName));
        mLivePlaybackStartedTime = System.currentTimeMillis();
        mPlaying = true;
        mPaused = false;
        mediaStateChanged(MediaState.PLAYING);
    }

    private void onPlaybackStopped() {
        mLogger.postVerbose(sTag, String.format("(%s) Media State Changed. STATE: STOPPED", mName));
        // if triggered via pause, keep previous offset
        if (mPaused)
            mLivePreviousTotalTimePlayed += System.currentTimeMillis() - mLivePlaybackStartedTime;
        mPlaying = false;
        mediaStateChanged(MediaState.STOPPED);
    }

    private void onPlaybackFinished() {
        if (mRepeating) {
            mPlayer.seekTo(0);
            mPlayer.setRepeatMode(Player.REPEAT_MODE_ONE);
        } else {
            mPlayer.setRepeatMode(Player.REPEAT_MODE_OFF);
            mLogger.postVerbose(sTag, String.format("(%s) Media State Changed. STATE: STOPPED", mName));
            mPlaying = false;
            mediaStateChanged(MediaState.STOPPED);
        }
    }

    private void onPlaybackBuffering() {
        mLogger.postVerbose(sTag, String.format("(%s) Media State Changed. STATE: BUFFERING", mName));
        mediaStateChanged(MediaState.BUFFERING);
    }

    @Override
    public void release() {
        mPlayer.release();
        dispose();
    }

    @Override
    public Equalizer getEqualizer() {
        return mEqualizer;
    }

    //
    // ExoPlayer event listener
    //
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
            mLogger.postError(sTag, "PLAYER ERROR: " + message);
            mediaError(MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, message);
        }
    }

    //
    private class AudioRendererListener implements AudioRendererEventListener {
        @Override
        public void onAudioSessionId(int audioSessionId) {
            mEqualizer = new Equalizer(1000, audioSessionId);
            mEqualizer.setEnabled(true);
        }

        @Override
        public void onAudioEnabled(DecoderCounters counters) {}

        @Override
        public void onAudioDecoderInitialized(
                String decoderName, long initializedTimestampMs, long initializationDurationMs) {}

        @Override
        public void onAudioInputFormatChanged(Format format) {}

        @Override
        public void onAudioSinkUnderrun(int bufferSize, long bufferSizeMs, long elapsedSinceLastFeedMs) {}

        @Override
        public void onAudioDisabled(DecoderCounters counters) {}
    }
}
