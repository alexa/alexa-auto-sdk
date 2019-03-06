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
import android.content.Context;
import android.net.Uri;
import android.support.annotation.Nullable;
import android.view.MotionEvent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.Speaker;

import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class MediaPlayerHandler extends com.amazon.aace.alexa.MediaPlayer {

    private static final String sTag = "MediaPlayer";
    private static final String sFileName = "alexa_media"; // Note: not thread safe

    private final Activity mActivity;
    private final Context mContext;
    private final LoggerHandler mLogger;
    private final String mName;
    private final SpeakerHandler mSpeaker;
    private final MediaSourceFactory mMediaSourceFactory;
    private PlaybackControllerHandler mPlaybackController;
    private SimpleExoPlayer mPlayer;

    public MediaPlayerHandler( Activity activity,
                               LoggerHandler logger,
                               String name,
                               @Nullable Speaker.Type speakerType,
                               @Nullable PlaybackControllerHandler controller ) {
        mActivity = activity;
        mContext = activity.getApplicationContext();
        mLogger = logger;
        mName = name;
        mSpeaker = new SpeakerHandler( speakerType );
        mMediaSourceFactory = new MediaSourceFactory( mContext, mLogger, mName );

        if ( controller != null ) {
            mPlaybackController = controller;
            mPlaybackController.setMediaPlayer( this );
        }
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
    }

    public boolean isPlaying() {
        return mPlayer != null && mPlayer.getPlayWhenReady()
                && ( mPlayer.getPlaybackState() == Player.STATE_BUFFERING
                || mPlayer.getPlaybackState() == Player.STATE_READY );
    }

    public long getDuration() {
        long duration = mPlayer.getDuration();
        return duration != C.TIME_UNSET ? duration : 0;
    }

    public Speaker getSpeaker() { return mSpeaker; }

    //
    // Handle playback directives from Engine
    //

    @Override
    public boolean prepare() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling prepare()", mName ) );
        resetPlayer();

        try ( FileOutputStream os = mContext.openFileOutput( sFileName, Context.MODE_PRIVATE ) ) {
            byte[] buffer = new byte[4096];
            int size;
            while ( !isClosed() ) {
                while ( ( size = read( buffer ) ) > 0 ) os.write( buffer, 0, size );
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
    public boolean prepare( String url ) {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling prepare(url)", mName ) );
        resetPlayer();
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
        mPlayer.setPlayWhenReady( false );
        return true;
    }

    @Override
    public boolean resume() {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling resume()", mName ) );
        mPlayer.setPlayWhenReady( true );
        return true;
    }

    @Override
    public boolean setPosition( long position ) {
        mLogger.postVerbose( sTag, String.format( "(%s) Handling setPosition(%s)", mName, position ) );
        mPlayer.seekTo( position );
        return true;
    }

    @Override
    public long getPosition() { return Math.abs( mPlayer.getCurrentPosition() ); }

    //
    // Handle ExoPlayer state changes and notify Engine
    //

    private void onPlaybackStarted () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: PLAYING", mName ) );
        mediaStateChanged( MediaState.PLAYING );
        if ( mPlaybackController != null ) { mPlaybackController.start(); }
    }

    private void onPlaybackStopped () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: STOPPED", mName ) );
        mediaStateChanged( MediaState.STOPPED );
        if ( mPlaybackController != null ) { mPlaybackController.stop(); }
    }

    private void onPlaybackFinished() {
        if ( isRepeating() ) {
            mPlayer.seekTo( 0 );
            mPlayer.setRepeatMode( Player.REPEAT_MODE_ONE );
        } else {
            mPlayer.setRepeatMode( Player.REPEAT_MODE_OFF );
            if ( mPlaybackController != null ) { mPlaybackController.reset(); }
            mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: STOPPED", mName ) );
            mediaStateChanged( MediaState.STOPPED );
        }
    }

    private void onPlaybackBuffering () {
        mLogger.postVerbose( sTag, String.format( "(%s) Media State Changed. STATE: BUFFERING", mName ) );
        mediaStateChanged( MediaState.BUFFERING );
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

    //
    // SpeakerHandler
    //

    public class SpeakerHandler extends Speaker {

        private SeekBar mVolumeControl;
        private TextView mMuteButton;
        private byte mVolume = 50;
        private boolean mIsMuted = false;

        SpeakerHandler( @Nullable Speaker.Type type ) {
            super();
            if ( type == Speaker.Type.AVS_SPEAKER  ) {
                // Link mute button to synced speakers only
                mMuteButton = mActivity.findViewById( R.id.muteSpeakerButton );
            }
            setupUIVolumeControls( type );
        }

        @Override
        public boolean setVolume( byte volume ) {
            if(mVolume == volume)
                return true;
            mLogger.postInfo( sTag, String.format( "(%s) Handling setVolume(%s)", mName, volume ) );
            mVolume = volume;
            if ( mIsMuted ) {
                mPlayer.setVolume( 0 );
                updateUIVolume( ( byte ) 0 );
            } else {
                float channelVolume = volume / 100f;
                mPlayer.setVolume( channelVolume );
                updateUIVolume( volume );
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
                updateMuteButton( mute );
            } else if ( !mute && mIsMuted ) {
                mLogger.postInfo( sTag, String.format( "Handling unmute (%s)", mName ) );
                updateMuteButton( mute );
            }

            mIsMuted = mute;
            if ( mute ) {
                mPlayer.setVolume( 0 );
                updateUIVolume( ( byte ) 0 );
            } else {
                mPlayer.setVolume( mVolume / 100f );
                updateUIVolume( mVolume );
            }
            return true;
        }

        @Override
        public boolean isMuted() {
            return mIsMuted;
        }

        private void setupUIVolumeControls( @Nullable final  Speaker.Type type ) {
            if ( type == Speaker.Type.AVS_SPEAKER ) {
                mVolumeControl = mActivity.findViewById( R.id.speakerVolume );
            } else {
                mVolumeControl = mActivity.findViewById( R.id.alertsVolume );
            }

            updateUIVolume( mVolume );
            updateMuteButton( mIsMuted );

            mVolumeControl.setOnSeekBarChangeListener( new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {}

                @Override
                public void onStartTrackingTouch( SeekBar seekBar ) {}

                @Override
                public void onStopTrackingTouch( SeekBar seekBar ) {
                    int progress = seekBar.getProgress();
                    if ( type != Speaker.Type.AVS_SPEAKER ) {
                        localVolumeSet( ( byte ) progress );
                    } else {
                        // Unmute before setting volume
                        if ( mIsMuted ) {
                            updateMuteButton( false );
                            localMuteSet( false );
                        }
                        localVolumeSet( ( byte ) progress );
                    }
                }
            });

            // Prevent parent view from intercepting touch events
            mVolumeControl.setOnTouchListener( new View.OnTouchListener() {
                @Override
                public boolean onTouch( View v, MotionEvent event ) {
                    int action = event.getAction();
                    switch( action ) {
                        case MotionEvent.ACTION_DOWN:
                            v.getParent().requestDisallowInterceptTouchEvent( true );
                            break;

                        case MotionEvent.ACTION_UP:
                            v.getParent().requestDisallowInterceptTouchEvent( false );
                            break;
                    }

                    // Handle SeekBar touch events
                    v.onTouchEvent( event );
                    return true;
                }
            });

            if ( mMuteButton != null ) {
                mMuteButton.setOnClickListener( new View.OnClickListener() {
                    @Override
                    public void onClick( View v ) {
                        updateMuteButton( !mIsMuted );
                        mLogger.postVerbose( sTag, String.format( "Calling localMuteSet(%s)", !mIsMuted ) );
                        localMuteSet( !mIsMuted );
                    }
                });
            }
        }

        private void updateMuteButton( final boolean isMuted ) {
            if ( mMuteButton != null ) {
                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        if ( !isMuted ) mMuteButton.setText( R.string.volume_mute );
                        else mMuteButton.setText( R.string.volume_unmute );
                    }
                });
            }
        }

        private void updateUIVolume( final byte vol ) {
            mActivity.runOnUiThread( new Runnable() {
                @Override
                public void run() { mVolumeControl.setProgress( vol ); }
            });
        }
    }
}
