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

import android.media.AudioFormat;
import android.media.AudioTrack;
import android.media.audiofx.Equalizer;

import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioStream;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.concurrent.atomic.AtomicLong;

/**
 * A @c AudioOutput capable to play raw PCM 16 bit data @ 16 KHZ.
 */
public class RawAudioOutputHandler
        extends AudioOutput implements Releasable, EqualizerControllerHandler.EqualizerProvider {
    private static final String sTag = "RawAudioAudioOutputHandler";

    private final LoggerHandler mLogger;
    private final String mName;
    private final int mStreamType;
    private AudioTrack mAudioTrack;
    private float mVolume = 0.5f;
    private MutedState mMutedState = MutedState.UNMUTED;
    private AudioStream mAudioStream;
    private AtomicLong mBytesWritten = new AtomicLong(0);
    private Equalizer mEqualizer = null;
    private int mAudioSessionId = 0;

    RawAudioOutputHandler(LoggerHandler logger, String name, int streamType) {
        mLogger = logger;
        mName = name;
        mStreamType = streamType;
    }

    private static AudioFormat intoAudioFormat(com.amazon.aace.audio.AudioFormat format) {
        AudioFormat.Builder builder = new AudioFormat.Builder();

        if (format.getEncoding() != com.amazon.aace.audio.AudioFormat.Encoding.LPCM) {
            throw new IllegalArgumentException("Unsupported encoding");
        }

        builder.setSampleRate(format.getSampleRate());

        if (format.getSampleSize() == 8) {
            builder.setEncoding(AudioFormat.ENCODING_PCM_8BIT);
        } else if (format.getSampleSize() == 16) {
            builder.setEncoding(AudioFormat.ENCODING_PCM_16BIT);
        } else {
            throw new IllegalArgumentException("Unsupported sample size");
        }

        switch (format.getNumChannels()) {
            case 1:
                builder.setChannelMask(AudioFormat.CHANNEL_OUT_MONO);
                break;
            case 2:
                builder.setChannelMask(AudioFormat.CHANNEL_OUT_STEREO);
                break;
            default:
                throw new IllegalArgumentException("Unsupported channel count");
        }

        return builder.build();
    }

    private void initializePlayer(AudioStream stream) {
        AudioFormat af = intoAudioFormat(stream.getAudioFormat());
        int audioBufferSize = AudioTrack.getMinBufferSize(af.getSampleRate(), af.getChannelMask(), af.getEncoding());
        mAudioTrack = new AudioTrack(mStreamType, af.getSampleRate(), af.getChannelMask(), af.getEncoding(),
                audioBufferSize, AudioTrack.MODE_STREAM);
        if (mAudioTrack.getState() == AudioTrack.STATE_UNINITIALIZED) {
            throw new RuntimeException("Failed to create AudioTrack");
        }
    }

    private void resetPlayer() {
        if (mAudioTrack != null) {
            mAudioTrack.flush();
            mBytesWritten.set(0);
        }
        initializePlayer(mAudioStream);
    }

    private boolean isPlaying() {
        return mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING;
    }

    //
    // Handle playback directives from Engine
    //

    @Override
    public boolean prepare(AudioStream stream, boolean repeating) {
        mLogger.postVerbose(sTag, String.format("(%s) Handling prepare()", mName));
        mAudioStream = stream;
        resetPlayer();
        return true;
    }

    @Override
    public boolean prepare(String url, boolean repeating) {
        throw new RuntimeException("URL based playback not supported " + url);
    }

    @Override
    public boolean play() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling play()", mName));
        startAudioTrack();
        return true;
    }

    private void startAudioTrack() {
        mAudioTrack.play();
        new Thread(new AudioSampleReadWriteRunnable()).start();
    }

    @Override
    public boolean stop() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling stop()", mName));
        mAudioTrack.stop();
        mBytesWritten.set(0);
        return true;
    }

    @Override
    public boolean pause() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling pause()", mName));
        mAudioTrack.pause();
        return true;
    }

    @Override
    public boolean resume() {
        mLogger.postVerbose(sTag, String.format("(%s) Handling resume()", mName));
        startAudioTrack();
        return true;
    }

    @Override
    public boolean setPosition(long position) {
        mLogger.postVerbose(sTag, String.format("(%s) Seek is not supported for Raw Audio", mName));
        return true;
    }

    @Override
    public long getPosition() {
        return Math.abs(mAudioTrack.getPlaybackHeadPosition());
    }

    @Override
    public long getNumBytesBuffered() {
        if (mAudioTrack != null) {
            long playbackHead = mAudioTrack.getPlaybackHeadPosition() & 0x00000000ffffffffL;
            int sampleSize = (mAudioTrack.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT) ? 2 : 1;
            int channelCount = mAudioTrack.getChannelCount();
            long writtenFrames = mBytesWritten.get() / channelCount / sampleSize;
            if (writtenFrames < playbackHead) {
                return 0;
            }
            return (writtenFrames - playbackHead) * sampleSize * channelCount;
        }
        return super.getNumBytesBuffered();
    }

    //
    // Handle state changes and notify Engine
    //

    private void onPlaybackStarted() {
        mLogger.postVerbose(sTag, String.format("(%s) Media State Changed. STATE: PLAYING", mName));
        mediaStateChanged(MediaState.PLAYING);
    }

    private void onPlaybackStopped() {
        mLogger.postVerbose(sTag, String.format("(%s) Media State Changed. STATE: STOPPED", mName));
        mediaStateChanged(MediaState.STOPPED);
    }

    @Override
    public void release() {
        if (mAudioTrack != null) {
            mAudioTrack.release();
        }
        dispose();
    }

    @Override
    public Equalizer getEqualizer() {
        int audioSessionId = mAudioTrack.getAudioSessionId();
        if (mEqualizer == null || (mAudioSessionId != audioSessionId)) {
            mEqualizer = new Equalizer(1000, audioSessionId);
            mEqualizer.setEnabled(true);
            mAudioSessionId = audioSessionId;
        }

        return mEqualizer;
    }

    private class AudioSampleReadWriteRunnable implements Runnable {
        @Override
        public void run() {
            onPlaybackStarted();

            try {
                mLogger.postVerbose(sTag, String.format("(%s) Audio Playback loop started", mName));
                byte[] audioBuffer = new byte[640];
                while (isPlaying() && !mAudioStream.isClosed()) {
                    int dataRead = mAudioStream.read(audioBuffer);
                    if (dataRead > 0) {
                        int byteWritten = mAudioTrack.write(audioBuffer, 0, dataRead);
                        if (byteWritten > 0) {
                            mBytesWritten.getAndAdd(dataRead);
                        }
                    }
                }
            } catch (Exception exp) {
                mLogger.postError(sTag, exp.getMessage());
                String message = exp.getMessage() != null ? exp.getMessage() : "";
                mediaError(MediaError.MEDIA_ERROR_UNKNOWN, message);
            } finally {
                onPlaybackStopped();
            }

            mLogger.postVerbose(sTag, String.format("(%s) Audio Playback loop exited", mName));
        }
    }

    @Override
    public boolean volumeChanged(float volume) {
        if (mVolume == volume)
            return true;
        mLogger.postInfo(sTag, String.format("(%s) Handling setVolume(%s)", mName, volume));
        mVolume = volume;
        if (mMutedState == MutedState.MUTED) {
            mAudioTrack.setVolume(0);
        } else {
            mAudioTrack.setVolume(mVolume);
        }
        return true;
    }

    @Override
    public boolean mutedStateChanged(MutedState state) {
        if (state != mMutedState) {
            mLogger.postInfo(sTag, String.format("Muted state changed (%s) to %s.", mName, state));
            mAudioTrack.setVolume(state == MutedState.MUTED ? 0 : mVolume);
            mMutedState = state;
        }

        return true;
    }
}
