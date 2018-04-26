/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.support.v4.content.ContextCompat;
import android.util.Log;

import com.amazon.aace.alexa.MediaPlayer;
import com.amazon.aace.alexa.Speaker;
import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.R;

import java.io.FileInputStream;
import java.io.FileOutputStream;

public class AndroidMediaPlayer extends com.amazon.aace.alexa.MediaPlayer implements android.media.MediaPlayer.OnCompletionListener, android.media.MediaPlayer.OnErrorListener
{
    private Context m_context;
    private android.media.MediaPlayer m_mediaPlayer;
    private boolean m_paused = false;
    private long m_currentPosition = 0;
    private AndroidMediaPlayerSpeaker m_speaker;
    private LoggerHandler m_logger;
    private PlaybackControllerHandler m_playbackController;
    private Activity m_activity;
    private int m_logColor;
    private String m_name;

    public AndroidMediaPlayer( Context context, LoggerHandler logger, String name,  Activity activity )
    {
        m_context = context;
        m_logger = logger;
        m_mediaPlayer = new android.media.MediaPlayer();
        m_mediaPlayer.setOnCompletionListener( this );
        m_mediaPlayer.setOnErrorListener( this );
        m_speaker = new AndroidMediaPlayerSpeaker();
        m_logColor = ContextCompat.getColor( context, R.color.logTextStatus) & 0x00FFFFFF;
        m_name = name;
        m_activity = activity;

    }

    public AndroidMediaPlayer( Context context, LoggerHandler logger, String name, Activity activity, PlaybackControllerHandler controller )
    {
        this( context, logger, name, activity );
        m_playbackController = controller;
    }

    public Speaker getSpeaker() {
        return m_speaker;
    }

    @Override
    public boolean prepare()
    {
        try
        {
            FileOutputStream os = m_context.openFileOutput( "alexa", Context.MODE_PRIVATE );

            byte[] buffer = new byte[4096];
            int size = 0;

            while( (size = read( buffer )) > 0 ) {
                os.write( buffer, 0, size );
            }

            os.close();

            FileInputStream is = m_context.openFileInput( "alexa" );

            m_mediaPlayer.reset();
            m_mediaPlayer.setDataSource( is.getFD() );
            m_currentPosition = 0;
            return true;
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return false;
        }
    }

    @Override
    public boolean prepare( String url )
    {
        try
        {
            m_mediaPlayer.reset();
            m_mediaPlayer.setDataSource( url );
            m_currentPosition = 0;
            return true;
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return true;
        }
    }

    @Override
    public boolean play()
    {
        try
        {
            m_mediaPlayer.prepareAsync();

            m_mediaPlayer.setOnPreparedListener(new android.media.MediaPlayer.OnPreparedListener() {
                @Override
                public void onPrepared(android.media.MediaPlayer mp) {
                    if( m_currentPosition <= mp.getDuration() )
                    {
                        mp.seekTo( ( int ) m_currentPosition );
                    }

                    mp.start();

                    m_paused = false;

                    m_activity.runOnUiThread( new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            playbackStarted();
                            // Set playback controller button to pause
                            if( m_playbackController != null ) {
                                m_playbackController.getControlPlayPause().setImageResource(R.drawable.control_selector_pause);
                            }
                        }
                    });
                }
            });
            return true;

        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return false;
        }
    }

    @Override
    public boolean pause()
    {
        try
        {
            if( m_mediaPlayer.isPlaying() )
            {
                m_mediaPlayer.pause();
                m_currentPosition = m_mediaPlayer.getCurrentPosition();
                m_paused = true;


                m_activity.runOnUiThread( new Runnable()
                {
                    @Override
                    public void run()
                    {
                        playbackPaused();
                        if( m_playbackController != null )
                        {
                            m_playbackController.getControlPlayPause().setImageResource(R.drawable.control_selector_play);
                        }
                    }
                });
            } else playbackPaused();

            return true;
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return false;
        }
    }

    @Override
    public boolean resume()
    {
        try
        {
            if( m_paused )
            {
                m_mediaPlayer.start();
                m_paused = false;


                m_activity.runOnUiThread( new Runnable()
                {
                    @Override
                    public void run()
                    {
                    playbackResumed();
                    // Set playback controller button to pause
                    if( m_playbackController != null ) {
                        m_playbackController.getControlPlayPause().setImageResource(R.drawable.control_selector_pause);
                    }
                    }
                });
            } else playbackResumed();

            return true;
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return false;
        }
    }

    @Override
    public boolean stop()
    {
        if( m_mediaPlayer.isPlaying() )
        {
            m_mediaPlayer.stop();
            m_currentPosition = m_mediaPlayer.getCurrentPosition();


            m_activity.runOnUiThread( new Runnable()
            {
                @Override
                public void run()
                {

                playbackStopped();
                // Set playback controller button to play
                if( m_playbackController != null )
                {
                    m_playbackController.getControlPlayPause().setImageResource(R.drawable.control_selector_play);
                }
                }
            });
        } else playbackStopped();

        return true;

    }

    @Override
    public long getPosition()
    {
        try
        {
            return m_mediaPlayer.isPlaying() ? m_mediaPlayer.getCurrentPosition() : m_currentPosition;
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return -1;
        }
    }

    @Override
    public boolean setPosition( long position )
    {
        try
        {
            if( m_mediaPlayer.isPlaying() )
            {
                m_mediaPlayer.seekTo( (int) position );
            }
            else
            {
                m_currentPosition = position;
            }

            return true;
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
            return false;
        }
    }

    @Override
    public void onCompletion( android.media.MediaPlayer mediaPlayer )
    {
        try
        {
            if( isRepeating() )
            {
                m_mediaPlayer.start();
            }
            else
            {
                playbackFinished();
                m_paused = false;
            }
        }
        catch( Throwable ex )
        {
            m_logger.post( ex );
        }

    }

    @Override
    public boolean onError( android.media.MediaPlayer mediaPlayer, int what, int extra ) {
        playbackError(ErrorType.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, "Android Media Player Error");
        return true;
    }

    //
    // AndroidMediaPlayerSpeaker
    //

    private class AndroidMediaPlayerSpeaker extends Speaker
    {
        private byte m_volume = 50;
        private boolean m_muted = false;

        @Override
        public boolean setVolume( byte volume )
        {
            try
            {
                m_logger.post(  "Volume (" + m_name+"): " + volume, Logger.Level.INFO, m_logColor);

                if( m_muted )
                {
                    m_mediaPlayer.setVolume( 0, 0 );
                }
                else
                {
                    float channelVolume = volume / 100f;
                    m_mediaPlayer.setVolume( channelVolume, channelVolume );
                }

                m_volume = volume;

                return true;
            }
            catch( Throwable ex )
            {
                m_logger.post( ex );

                return false;
            }
        }

        @Override
        public boolean adjustVolume( byte value ) {
            return setVolume( (byte) (m_volume + value) );
        }

        @Override
        public byte getVolume() {
            return m_volume;
        }

        @Override
        public boolean setMute( boolean mute )
        {
            m_mediaPlayer.setVolume( 0, 0 );
            m_muted = mute;
            return true;
        }

        @Override
        public boolean isMuted() {
            return m_muted;
        }
    }
}
