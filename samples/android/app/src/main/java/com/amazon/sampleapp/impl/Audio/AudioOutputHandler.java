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

package com.amazon.sampleapp.impl.Audio;

import android.app.Activity;
import android.content.Context;
import android.net.Uri;

import com.amazon.aace.alexa.AuthProvider.AuthState;
import com.amazon.aace.alexa.AuthProvider.AuthError;

import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioStream;
import com.amazon.sampleapp.impl.AuthProvider.AuthStateObserver;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;

import java.io.FileOutputStream;
import java.io.IOException;

public class AudioOutputHandler extends AudioOutput implements AuthStateObserver
{
    private static final String sTag = AudioOutputHandler.class.getSimpleName();
    private static final String sFileName = "alexa_media"; // Note: not thread safe

    private final Activity mActivity;
    private final Context mContext;
    private final LoggerHandler mLogger;
    private final String mName;
    private final MediaSourceFactory mMediaSourceFactory;
    private SimpleExoPlayer mPlayer;
    private boolean mRepeating;

    private float mVolume = 0.5f;
    private MutedState mMutedState = MutedState.UNMUTED;

    private Timeline.Period mPeriod;
    private long mPosition;
    private long mLivePausedPosition;
    private int mSavedPeriodIndex;
    private long mLivePausedOffset;
    private long mLiveResumedOffset;
    private boolean mNewPlayReceieved;

    public AudioOutputHandler( Activity activity,
                               LoggerHandler logger,
                               String name ) {
        mActivity = activity;
        mContext = activity.getApplicationContext();
        mLogger = logger;
        mName = name;
        mMediaSourceFactory = new MediaSourceFactory( mContext, mLogger, mName );
        mRepeating = false;
        mPeriod = new Timeline.Period();

        initializePlayer();
    }

    private void initializePlayer() {
        mPlayer = ExoPlayerFactory.newSimpleInstance( mContext, new DefaultTrackSelector() );
        mPlayer.addListener( new PlayerEventListener() );
        mPlayer.setPlayWhenReady( false );
    }

    private void resetPlayer() {
        mPlayer.setRepeatMode( Player.REPEAT_MODE_OFF );
        mPlayer.setPlayWhenReady( false );
        mPlayer.stop( true );
        // reset live station offsets
        mLiveResumedOffset = 0;
        mLivePausedPosition = 0;
    }

    public boolean isPlaying() {
        return mPlayer != null && mPlayer.getPlayWhenReady()
            && ( mPlayer.getPlaybackState() == Player.STATE_BUFFERING
            || mPlayer.getPlaybackState() == Player.STATE_READY );
    }

    //
    // Handle playback directives from Engine
    //

    @Override
    public boolean prepare( AudioStream stream, boolean repeating ) {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling prepare()", mName ) );
        resetPlayer();
        mRepeating = repeating;
        try ( FileOutputStream os = mContext.openFileOutput( sFileName, Context.MODE_PRIVATE ) ) {
            byte[] buffer = new byte[4096];
            int size;
            while ( !stream.isClosed() ) {
                while ( ( size = stream.read( buffer ) ) > 0 ) os.write( buffer, 0, size );
            }
        } catch ( IOException e ) {
            mLogger.postError( sTag, e );
            return false;
        }

        try {
            Uri uri = Uri.fromFile( mContext.getFileStreamPath( sFileName ) );
            MediaSource mediaSource = mMediaSourceFactory.createFileMediaSource( uri );
            mPlayer.prepare( mediaSource, true, false );
            return true;
        } catch ( Exception e ) {
            mLogger.postError( sTag, e.getMessage() );
            String message = e.getMessage() != null ? e.getMessage() : "";
            mediaError( MediaError.MEDIA_ERROR_UNKNOWN, message );
            return false;
        }
    }

    @Override
    public boolean prepare( String url, boolean repeating ) {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling prepare(url)", mName ) );
        resetPlayer();
        mRepeating = repeating;
        Uri uri = Uri.parse( url );
        try {
            MediaSource mediaSource = mMediaSourceFactory.createHttpMediaSource( uri );
            mPlayer.prepare( mediaSource, true, false );
            return true;
        } catch ( Exception e ) {
            String message = e.getMessage() != null ? e.getMessage() : "";
            mLogger.postError( sTag, message );
            mediaError( MediaError.MEDIA_ERROR_UNKNOWN, message );
            return false;
        }
    }

    @Override
    public boolean play() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling play()", mName ) );
        mNewPlayReceieved = true; // remember new play received
        mSavedPeriodIndex = mPlayer.getCurrentPeriodIndex(); // remember period index
        mPlayer.setPlayWhenReady( true );
        return true;
    }

    @Override
    public boolean stop() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling stop()", mName ) );
        if ( !mPlayer.getPlayWhenReady() ) {
            // Player is already not playing. Notify Engine of stop
            onPlaybackStopped();
        } else mPlayer.setPlayWhenReady( false );
        return true;
    }

    @Override
    public boolean pause() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling pause()", mName ) );

        Timeline currentTimeline = mPlayer.getCurrentTimeline();
        if( !currentTimeline.isEmpty() && mPlayer.isCurrentWindowDynamic() ) { // If pausing live station.
            mLivePausedOffset = 0;
            mLivePausedPosition = mPosition; // save paused position
        }

        mPlayer.setPlayWhenReady( false );
        return true;
    }

    @Override
    public boolean resume() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling resume()", mName ) );

        Timeline currentTimeline = mPlayer.getCurrentTimeline();
        if ( !currentTimeline.isEmpty() && mPlayer.isCurrentWindowDynamic() ) {  // If resuming live station reset to 0.
            mPlayer.seekToDefaultPosition(); // reset player position to its default
            mLivePausedOffset = Math.abs( mPlayer.getCurrentPosition() ); // get the new position
            mLivePausedOffset -= currentTimeline.getPeriod(mSavedPeriodIndex, mPeriod).getPositionInWindowMs(); // adjust for window
            mLivePausedOffset -= mLiveResumedOffset; // adjust for stopped offset
            mLivePausedOffset -= mLivePausedPosition; // adjust for paused offset

            mLivePausedPosition = 0; // reset paused position
        }

        mPlayer.setPlayWhenReady( true );
        return true;
    }

    @Override
    public boolean setPosition( long position ) {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling setPosition(%s)", mName, position ) );
        mPlayer.seekTo( position );
        mLiveResumedOffset -= position;
        return true;
    }

    @Override
    public long getPosition() {
        Timeline currentTimeline = mPlayer.getCurrentTimeline();
        mPosition = Math.abs( mPlayer.getCurrentPosition() );
        if ( !currentTimeline.isEmpty() && mPlayer.isCurrentWindowDynamic() ) {
            if ( mLivePausedPosition == 0 ) { // not during pause
                mPosition -= currentTimeline.getPeriod(mSavedPeriodIndex, mPeriod).getPositionInWindowMs(); // Adjust position to be relative to start of period rather than window.
                mPosition -= mLiveResumedOffset; // Offset saved for live station stopped / played
                mPosition -= mLivePausedOffset; // Offset saved for live station paused / resumed
            } else{
                mLogger.postVerbose( sTag, String.format( "(%s) Handling livePaused getPosition(%s)", mName, mLivePausedPosition ) );
                return mLivePausedPosition; // the saved position during a live station paused state
            }
        }
        mLogger.postVerbose( sTag, String.format( "(%s) Handling getPosition(%s)", mName, mPosition ) );
        return mPosition;
    }

    @Override
    public long getDuration() {
        long duration = mPlayer.getDuration();
        return duration != C.TIME_UNSET ? duration : TIME_UNKNOWN;
    }

    @Override
    public boolean volumeChanged( float volume ) {
        if( mVolume != volume ) {
            mLogger.postInfo( sTag, String.format( "(%s) Handling volumeChanged(%s)", mName, volume ) );
            mVolume = volume;
            if ( mMutedState == MutedState.MUTED ) {
                mPlayer.setVolume( 0 );
            } else {
                mPlayer.setVolume( volume );
            }
        }
        return true;
    }

    @Override
    public boolean mutedStateChanged( MutedState state ) {
        if( state != mMutedState ) {
            mLogger.postInfo( sTag, String.format( "Muted state changed (%s) to %s.", mName, state ) );
            mPlayer.setVolume( state == MutedState.MUTED ? 0 : mVolume );
            mMutedState = state;
        }
        return true;
    }

    //
    // Handle ExoPlayer state changes and notify Engine
    //

    private void onPlaybackStarted () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: PLAYING", mName ) );
        mediaStateChanged( MediaState.PLAYING );

        if ( mNewPlayReceieved && mPlayer.isCurrentWindowDynamic() ) { // remember offset if new play for live station
            mPlayer.seekToDefaultPosition();
            mLiveResumedOffset += Math.abs( mPlayer.getCurrentPosition() );
            mNewPlayReceieved = false;
        }
    }

    private void onPlaybackStopped () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: STOPPED", mName ) );
        mediaStateChanged( MediaState.STOPPED );
    }

    private void onPlaybackFinished() {
        if ( mRepeating ) {
            mPlayer.seekTo( 0 );
            mPlayer.setRepeatMode( Player.REPEAT_MODE_ONE );
        } else {
            mPlayer.setRepeatMode( Player.REPEAT_MODE_OFF );
            mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: STOPPED", mName ) );
            mediaStateChanged( MediaState.STOPPED );
        }
    }

    private void onPlaybackBuffering () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: BUFFERING", mName ) );
        mediaStateChanged( MediaState.BUFFERING );
    }

    @Override
    public void onAuthStateChanged(AuthState state, AuthError error, String token) {
        if ( state == AuthState.UNINITIALIZED ) {
            mLogger.postInfo( sTag, String.format( "(%s) Auth state is uninitialized. Stopping media player", mName ) );
            // Stop playing media if user logs out
            stop();
        }
    }

    //
    // ExoPlayer event listener
    //
    private class PlayerEventListener extends Player.DefaultEventListener {

        @Override
        public void onPlayerStateChanged( boolean playWhenReady, int playbackState ) {
            switch ( playbackState ) {
                case Player.STATE_ENDED:
                    if ( playWhenReady ) onPlaybackFinished();
                    break;
                case Player.STATE_READY:
                    if ( playWhenReady ) onPlaybackStarted();
                    else onPlaybackStopped();
                    break;
                case Player.STATE_BUFFERING:
                    if ( playWhenReady ) onPlaybackBuffering();
                    break;
                default:
                    // Disregard other states
                    break;
            }
        }

        @Override
        public void onPlayerError( ExoPlaybackException e ) {
            String message;
            if ( e.type == ExoPlaybackException.TYPE_SOURCE ) {
                message = "ExoPlayer Source Error: " + e.getSourceException().getMessage();
            } else if ( e.type == ExoPlaybackException.TYPE_RENDERER ) {
                message = "ExoPlayer Renderer Error: " + e.getRendererException().getMessage();
            } else if ( e.type == ExoPlaybackException.TYPE_UNEXPECTED ) {
                message = "ExoPlayer Unexpected Error: " + e.getUnexpectedException().getMessage();
            } else {
                message = e.getMessage();
            }
            mLogger.postError( sTag, "PLAYER ERROR: " + message );
            mediaError( MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, message );
        }
    }
}
