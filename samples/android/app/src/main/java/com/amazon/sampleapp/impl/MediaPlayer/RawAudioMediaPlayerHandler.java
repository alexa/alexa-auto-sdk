/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.MediaPlayer;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.support.annotation.Nullable;

import com.amazon.aace.alexa.Speaker;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

/**
 * A @c MediaPlayer capable to play raw PCM 16 bit data @ 16 KHZ.
 */
public class RawAudioMediaPlayerHandler extends com.amazon.aace.alexa.MediaPlayer {

    private static final String sTag = "RawAudioMediaPlayerHandler";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final String mName;
    private final SpeakerHandler mSpeaker;
    private AudioTrack mAudioTrack;
    private Thread mAudioPlaybackThread;

    public RawAudioMediaPlayerHandler(
            Activity activity,
            LoggerHandler logger,
            String name,
            @Nullable Speaker.Type speakerType) {
        mActivity = activity;
        mLogger = logger;
        mName = name;
        mSpeaker = new SpeakerHandler( speakerType );

        initializePlayer();
    }

    private void initializePlayer() {
        int audioBufferSize = AudioTrack.getMinBufferSize(
                16000,
                AudioFormat.CHANNEL_OUT_MONO,
                AudioFormat.ENCODING_PCM_16BIT);
        mAudioTrack = new AudioTrack(
                AudioManager.STREAM_VOICE_CALL,
                16000,
                AudioFormat.CHANNEL_OUT_MONO,
                AudioFormat.ENCODING_PCM_16BIT,
                audioBufferSize,
                AudioTrack.MODE_STREAM);
        if (mAudioTrack.getState() == AudioTrack.STATE_UNINITIALIZED) {
            throw new RuntimeException("Failed to create AudioTrack");
        }
    }

    private void resetPlayer() {
        mAudioTrack.flush();
    }

    public boolean isPlaying() {
        return mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING;
    }

    public Speaker getSpeaker() { return mSpeaker; }

    //
    // Handle playback directives from Engine
    //

    @Override
    public boolean prepare() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling prepare()", mName ) );
        resetPlayer();
        return true;
    }

    @Override
    public boolean prepare( String url ) {
        throw new RuntimeException("URL based playback not supported " + url);
    }

    @Override
    public boolean play() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling play()", mName ) );
        mAudioTrack.play();
        mAudioPlaybackThread = new Thread(new AudioSampleReadWriteRunnable());
        mAudioPlaybackThread.start();
        return true;
    }

    @Override
    public boolean stop() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling stop()", mName ) );
        mAudioTrack.stop();
        return true;
    }

    @Override
    public boolean pause() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling pause()", mName ) );
        mAudioTrack.pause();
        return true;
    }

    @Override
    public boolean resume() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling resume()", mName ) );
        mAudioTrack.play();
        return true;
    }

    @Override
    public boolean setPosition( long position ) {
        mLogger.postVerbose( sTag, String.format( "(%s) Seek is not supported for Raw Audio") );
        return true;
    }

    @Override
    public long getPosition() { return Math.abs( mAudioTrack.getPlaybackHeadPosition() ); }

    //
    // Handle state changes and notify Engine
    //

    private void onPlaybackStarted () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: PLAYING", mName ) );
        mediaStateChanged( MediaState.PLAYING );
    }

    private void onPlaybackStopped () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: STOPPED", mName ) );
        mediaStateChanged( MediaState.STOPPED );
    }

    private class AudioSampleReadWriteRunnable implements Runnable {
        @Override
        public void run() {
            onPlaybackStarted();
            try {
                mLogger.postVerbose(sTag, String.format("(%s) Audio Playback loop started", mName));
                byte[] audioBuffer = new byte[640];
                while (isPlaying() && !isClosed()) {
                    int dataRead = read(audioBuffer);
                    if (dataRead > 0) {
                        mAudioTrack.write(audioBuffer, 0, dataRead);
                    }
                }
            } catch (Exception exp) {
                mLogger.postError( sTag, exp.getMessage() );
                String message = exp.getMessage() != null ? exp.getMessage() : "";
                mediaError(MediaError.MEDIA_ERROR_UNKNOWN, message);
            } finally {
                onPlaybackStopped();
            }
            mLogger.postVerbose(sTag, String.format("(%s) Audio Playback loop exited", mName));
        }
    }

    //
    // SpeakerHandler for Raw audio media player.
    //

    public class SpeakerHandler extends Speaker {

        private byte mVolume = 50;
        private boolean mIsMuted = false;

        SpeakerHandler( @Nullable Speaker.Type type ) {
            super();
        }

        @Override
        public boolean setVolume( byte volume ) {
            if(mVolume == volume)
                return true;
            mLogger.postInfo( sTag, String.format( "(%s) Handling setVolume(%s)", mName, volume ) );
            mVolume = volume;
            if ( mIsMuted ) {
                mAudioTrack.setVolume( 0 );
            } else {
                float channelVolume = volume / 100f;
                mAudioTrack.setVolume( channelVolume );
            }
            return true;
        }

        @Override
        public boolean adjustVolume( byte value ) {
            return setVolume( ( byte ) ( mVolume + value ) );
        }

        @Override
        public byte getVolume() {
            if ( mIsMuted ) return 0;
            else return mVolume;
        }

        @Override
        public boolean setMute( boolean mute ) {
            if ( mute && !mIsMuted ) {
                mLogger.postInfo( sTag, String.format( "Handling mute (%s)", mName ) );
            } else if ( !mute && mIsMuted ) {
                mLogger.postInfo( sTag, String.format( "Handling unmute (%s)", mName ) );
            }

            mIsMuted = mute;
            if ( mute ) {
                mAudioTrack.setVolume( 0 );
            } else {
                mAudioTrack.setVolume( mVolume / 100f );
            }
            return true;
        }

        @Override
        public boolean isMuted() {
            return mIsMuted;
        }
    }
}
