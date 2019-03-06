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

package com.amazon.sampleapp.impl.PlaybackController;

import android.app.Activity;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.amazon.aace.alexa.PlaybackController;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.MediaPlayer.MediaPlayerHandler;

import java.lang.ref.WeakReference;
import java.util.Formatter;
import java.util.Locale;
import java.util.TimerTask;

public class PlaybackControllerHandler extends PlaybackController {

    private static final String sTag = "PlaybackController";
    private static final int SHOW_PROGRESS = 0;

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final ProgressHandler mProgressHandler;
    private final StringBuilder mStringBuilder;
    private final Formatter mFormatter;
    private MediaPlayerHandler mMediaPlayer;
    private ImageButton mControlPrev, mControlNext, mControlSkipForward, mControlSkipBackward;
    private ToggleButton mControlPlayPause, mShuffleToggle, mLoopToggle, mRepeatToggle, mThumbsUpToggle, mThumbsDownToggle;
    private TextView mProgressTime, mEndTime, mTitle, mArtist, mProvider;
    private ProgressBar mProgress;
    private String mCurrentProvider = "";

    public PlaybackControllerHandler( Activity activity, LoggerHandler logger ) {
        mActivity = activity;
        mLogger = logger;
        mProgressHandler = new ProgressHandler( this );
        mStringBuilder = new StringBuilder();
        mFormatter = new Formatter( mStringBuilder, Locale.US );
        setupGUI();
    }

    public void previousButtonPressed() {
        mLogger.postVerbose( sTag, "Calling PREVIOUS buttonPressed" );
        buttonPressed( PlaybackButton.PREVIOUS );
    }

    public void playButtonPressed() {
        mLogger.postVerbose( sTag, "Calling PLAY buttonPressed" );
        buttonPressed( PlaybackButton.PLAY );
    }

    public void pauseButtonPressed() {
        mLogger.postVerbose( sTag, "Calling PAUSE buttonPressed" );
        buttonPressed( PlaybackButton.PAUSE );
    }

    public void nextButtonPressed() {
        mLogger.postVerbose( sTag, "Calling NEXT buttonPressed" );
        buttonPressed( PlaybackButton.NEXT );
    }

    public void skipForwardButtonPressed() {
        mLogger.postVerbose( sTag, "Calling SKIP_FORWARD buttonPressed" );
        buttonPressed( PlaybackButton.SKIP_FORWARD );
    }

    public void skipBackwardButtonPressed() {
        mLogger.postVerbose( sTag, "Calling SKIP_BACKWARD buttonPressed" );
        buttonPressed( PlaybackButton.SKIP_BACKWARD );
    }

    public void shuffleTogglePressed( boolean action ) {
        mLogger.postVerbose( sTag, String.format( "Calling SHUFFLE togglePressed %s", action ? "selected" : "deselected" ) );
        togglePressed( PlaybackToggle.SHUFFLE, action );
    }

    public void loopTogglePressed( boolean action ) {
        mLogger.postVerbose( sTag, String.format( "Calling LOOP togglePressed %s", action ? "selected" : "deselected" ) );
        togglePressed( PlaybackToggle.LOOP, action );
    }

    public void repeatTogglePressed( boolean action ) {
        mLogger.postVerbose( sTag, String.format( "Calling REPEAT togglePressed %s", action ? "selected" : "deselected" ) );
        togglePressed( PlaybackToggle.REPEAT, action );
    }

    public void thumbsUpTogglePressed( boolean action ) {
        mLogger.postVerbose( sTag, String.format( "Calling THUMBS_UP togglePressed %s", action ? "selected" : "deselected" ) );
        togglePressed( PlaybackToggle.THUMBS_UP, action );
    }

    public void thumbsDownTogglePressed( boolean action ) {
        mLogger.postVerbose( sTag, String.format( "Calling THUMBS_DOWN togglePressed %s", action ? "selected" : "deselected" ) );
        togglePressed( PlaybackToggle.THUMBS_DOWN, action );
    }

    public void setMediaPlayer( MediaPlayerHandler mediaPlayer ) { mMediaPlayer = mediaPlayer; }

    MediaPlayerHandler getMediaPlayer() { return mMediaPlayer; }

    //
    // GUI updates
    //

    private void setupGUI() {
        mControlPrev = mActivity.findViewById( R.id.prevControlButton );
        mControlPlayPause = mActivity.findViewById( R.id.playControlButton );
        mControlNext = mActivity.findViewById( R.id.nextControlButton );
        mControlSkipForward = mActivity.findViewById( R.id.skipForwardControlButton );
        mControlSkipBackward = mActivity.findViewById( R.id.skipBackwardControlButton );

        mShuffleToggle = mActivity.findViewById( R.id.shuffleToggle );
        mLoopToggle = mActivity.findViewById( R.id.loopToggle );
        mRepeatToggle = mActivity.findViewById( R.id.repeatToggle );
        mThumbsUpToggle = mActivity.findViewById( R.id.thumbsUpToggle );
        mThumbsDownToggle = mActivity.findViewById( R.id.thumbsDownToggle );

        mProgress = mActivity.findViewById( R.id.mediaProgressBar );
        mProgressTime = mActivity.findViewById( R.id.mediaProgressTime );
        mEndTime = mActivity.findViewById( R.id.mediaEndTime );
        mTitle = mActivity.findViewById( R.id.mediaTitle );
        mArtist = mActivity.findViewById( R.id.mediaArtist );
        mProvider = mActivity.findViewById( R.id.mediaProvider );

        // playback button listeners
        mControlPrev.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) { previousButtonPressed(); }
        });

        mControlPlayPause.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
//                if ( mMediaPlayer.isPlaying() ) pauseButtonPressed();
//                else playButtonPressed();
                if ( mControlPlayPause.isChecked() ){
                    playButtonPressed();
                } else {
                    pauseButtonPressed();
                }

            }
        });

        mControlNext.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) { nextButtonPressed(); }
        });

        mControlSkipForward.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                skipForwardButtonPressed();

                // block UI input until state is synced by callback
                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        mControlSkipForward.setEnabled( false );
                    }
                });
            }
        });

        mControlSkipBackward.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                skipBackwardButtonPressed();

                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        mControlSkipBackward.setEnabled( false );
                    }
                });
            }
        });

        mShuffleToggle.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                // Send UI toggle state
                shuffleTogglePressed( mShuffleToggle.isChecked() );

                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        mShuffleToggle.setEnabled( false );
                    }
                });
            }
        });

        mLoopToggle.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                loopTogglePressed( mLoopToggle.isChecked() );

                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        mLoopToggle.setEnabled( false );
                    }
                });
            }
        });

        mRepeatToggle.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                repeatTogglePressed( mRepeatToggle.isChecked() );

                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        mRepeatToggle.setEnabled( false );
                    }
                });
            }
        });

        mThumbsUpToggle.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                // ignore un-checking thumbs_up, cloud behavior does not support it
                if ( mThumbsUpToggle.isChecked() ) {
                    thumbsUpTogglePressed( true );
                    mActivity.runOnUiThread( new Runnable() {
                        @Override
                        public void run() {
                            mThumbsUpToggle.setEnabled( false );
                        }
                    });
                } else mThumbsUpToggle.setChecked( true );
            }
        });

        mThumbsDownToggle.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick( View v ) {
                // ignore un-checking thumbs_down, cloud behavior does not support it
                if ( mThumbsDownToggle.isChecked() ) {
                    thumbsDownTogglePressed(mThumbsDownToggle.isChecked());

                    mActivity.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mThumbsDownToggle.setEnabled(false);
                        }
                    });
                } else mThumbsDownToggle.setChecked(true);
            }
        });

        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mControlPrev.setEnabled( false );
                mControlPlayPause.setChecked(false);
                mControlPlayPause.setEnabled( false );
                mControlNext.setEnabled( false );

                // MSP-specific controls from the RenderPlayerInfo template are hidden by default
                mControlSkipForward.setVisibility( View.GONE );
                mControlSkipBackward.setVisibility( View.GONE );

                mShuffleToggle.setVisibility( View.GONE );
                mLoopToggle.setVisibility( View.GONE );
                mRepeatToggle.setVisibility( View.GONE );
                mThumbsUpToggle.setVisibility( View.GONE );
                mThumbsDownToggle.setVisibility( View.GONE );
            }
        });
    }

    public void setPlayerInfo( final String title, final String artist, final String provider ) {
        mCurrentProvider = provider;
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mTitle.setText( title );
                mArtist.setText( artist );
                mProvider.setText( provider );
            }
        });
    }

    public String getProvider(){
        return mCurrentProvider;
    }

    public void start() {
        if ( mMediaPlayer == null ) { return; }
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                //mControlPlayPause.setImageResource( R.drawable.control_selector_pause );
                mControlPrev.setEnabled( true );
                mControlPlayPause.setEnabled( true );
                mControlPlayPause.setChecked(true);
                mControlNext.setEnabled( true );
                mProgress.setMax( 1000 );
                mProgressHandler.sendEmptyMessage( SHOW_PROGRESS );
            }
        });
    }

    public void stop() {
        if ( mMediaPlayer == null ) { return; }
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                //mControlPlayPause.setImageResource( R.drawable.control_selector_play );
                mProgressHandler.removeMessages( SHOW_PROGRESS );
                mControlPlayPause.setChecked(false);
            }
        });
    }

    public void reset() {
        if ( mMediaPlayer == null ) { return; }
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                //mControlPlayPause.setImageResource( R.drawable.control_selector_play );
                mControlPlayPause.setChecked(false);
                mControlPrev.setEnabled( false );
                mControlPlayPause.setEnabled( false );
                mControlNext.setEnabled( false );
                mProgressHandler.removeMessages( SHOW_PROGRESS );
                resetProgress();
                setPlayerInfo( "", "", "" );
            }
        });
    }

    // Updates Control Button's states
    public void updateControlButton( final String name, final boolean enabled ) {
        if ( mMediaPlayer == null ) { return; }
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                switch ( name ) {
                    case "PREVIOUS":
                        mControlPrev.setEnabled( enabled );
                        break;
                    case "PLAY_PAUSE":
                        mControlPlayPause.setEnabled( enabled );
                        break;
                    case "NEXT":
                        mControlNext.setEnabled( enabled );
                        break;
                    case "SKIP_FORWARD":
                        mControlSkipForward.setVisibility( View.VISIBLE );
                        mControlSkipForward.setEnabled( enabled );
                        break;
                    case "SKIP_BACKWARD":
                        mControlSkipBackward.setVisibility( View.VISIBLE );
                        mControlSkipBackward.setEnabled( enabled );
                        break;
                }
            }
        });
    }

    // Updates Toggle's display states
    // NOTE: Disabled controls not hidden here for development visibility.
    public void updateControlToggle( final String name, final boolean enabled, final boolean selected ) {
        if ( mMediaPlayer == null ) { return; }
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                switch ( name ) {
                    case "SHUFFLE":
                        mShuffleToggle.setVisibility( View.VISIBLE );
                        mShuffleToggle.setEnabled( enabled );
                        mShuffleToggle.setChecked( selected );
                        break;
                    case "LOOP":
                        mLoopToggle.setVisibility( View.VISIBLE );
                        mLoopToggle.setEnabled( enabled );
                        mLoopToggle.setChecked( selected );
                        break;
                    case "REPEAT":
                        mRepeatToggle.setVisibility( View.VISIBLE );
                        mRepeatToggle.setEnabled( enabled );
                        mRepeatToggle.setChecked( selected );
                        break;
                    case "THUMBS_UP":
                        mThumbsUpToggle.setVisibility( View.VISIBLE );
                        mThumbsUpToggle.setEnabled( enabled );
                        mThumbsUpToggle.setChecked( selected );
                        break;
                    case "THUMBS_DOWN":
                        mThumbsDownToggle.setVisibility( View.VISIBLE );
                        mThumbsDownToggle.setEnabled( enabled );
                        mThumbsDownToggle.setChecked( selected );
                        break;
                }
            }
        });
    }

    public void hidePlayerInfoControls(){
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mControlSkipForward.setVisibility( View.GONE );
                mControlSkipBackward.setVisibility( View.GONE );

                mShuffleToggle.setVisibility( View.GONE );
                mLoopToggle.setVisibility( View.GONE );
                mRepeatToggle.setVisibility( View.GONE );
                mThumbsUpToggle.setVisibility( View.GONE );
                mThumbsDownToggle.setVisibility( View.GONE );
            }
        });
    }

    private void resetProgress() {
        mActivity.runOnUiThread( new Runnable() {
            @Override
            public void run() {
                mProgress.setProgress( 0 );
                mProgressTime.setText( "0:00" );
                mEndTime.setText( "0:00" );
            }
        });
    }

    private long setProgress() {
        if ( mMediaPlayer == null ) return 0;

        long position = mMediaPlayer.getPosition();
        long duration = mMediaPlayer.getDuration();
        if ( mProgress != null ) {
            if ( duration > 0 ) {
                long pos = 1000L * position / duration;
                mProgress.setProgress( ( int ) pos);
            }
        }

        mEndTime.setText( stringForTime( ( int ) duration ) );
        mProgressTime.setText( stringForTime( ( int ) position ) );

        return position;
    }

    private String stringForTime( int timeMs ) {
        int totalSeconds = timeMs / 1000;
        int seconds = totalSeconds % 60;
        int minutes = ( totalSeconds / 60 ) % 60;
        int hours   = totalSeconds / 3600;

        mStringBuilder.setLength( 0 );
        if ( hours > 0 ) {
            return mFormatter.format( "%d:%02d:%02d", hours, minutes, seconds ).toString();
        } else {
            return mFormatter.format( "%02d:%02d", minutes, seconds ).toString();
        }
    }

    private static class ProgressHandler extends Handler {

        private final WeakReference<PlaybackControllerHandler> mController;

        ProgressHandler( PlaybackControllerHandler controller ) {
            mController = new WeakReference<>( controller );
        }

        @Override
        public void handleMessage( Message msg ) {
            PlaybackControllerHandler controller = mController.get();

            long pos;
            switch ( msg.what ) {
                case SHOW_PROGRESS:
                    pos = controller.setProgress();
                    if ( controller.getMediaPlayer().isPlaying() ) {
                        msg = obtainMessage( SHOW_PROGRESS );
                        sendMessageDelayed( msg, 1000 - ( pos % 1000 ) );
                    }
                    break;
            }
        }
    }
}
