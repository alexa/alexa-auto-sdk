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

package com.amazon.sampleapp.impl.AudioPlayer;

import android.os.Handler;
import android.os.Message;

import com.amazon.aace.alexa.AudioPlayer;
import com.amazon.aace.audio.AudioOutput;
import com.amazon.sampleapp.impl.Audio.AudioOutputProviderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class AudioPlayerHandler extends AudioPlayer
{
    private static String TAG = AudioPlayerHandler.class.getSimpleName();
    private LoggerHandler mLogger = null;
    private AudioOutputProviderHandler mAudioOutputProvider = null;
    private PlaybackControllerHandler mPlaybackController = null;
    private AudioPlayerStateHandler mAudioPlayerStateHandler = null;

    public AudioPlayerHandler( LoggerHandler logger, AudioOutputProviderHandler audioOutputProvider, PlaybackControllerHandler playbackController ) {
        mLogger = logger;
        mPlaybackController = playbackController;
        mAudioOutputProvider = audioOutputProvider;
        mAudioPlayerStateHandler = new AudioPlayerStateHandler();
    }

    @Override
    public void playerActivityChanged(AudioPlayer.PlayerActivity state) {
        mLogger.postInfo( TAG, String.format( "playerActivityChanged: %s", state.toString() ) );
        mAudioPlayerStateHandler.sendEmptyMessage( state.ordinal() );
    }

    //
    // ProgressHandler
    //

    static private int UPDATE_PROGRESS = Integer.MAX_VALUE;

    private class AudioPlayerStateHandler extends Handler
    {
        AudioPlayerStateHandler() {
        }

        @Override
        public void handleMessage( Message msg )
        {
            if( msg.what == UPDATE_PROGRESS )
            {
                AudioOutput audioOutput = mAudioOutputProvider.getOutputChannel( "AudioPlayer" );

                if( audioOutput != null )
                {
                    long position = audioOutput.getPosition();

                    if(audioOutput.getDuration() == AudioOutput.TIME_UNKNOWN) {
                        position = AudioOutput.TIME_UNKNOWN;
                    }

                    mPlaybackController.setTime( position, audioOutput.getDuration() );

                    sendEmptyMessageDelayed( UPDATE_PROGRESS, 1000 - (position % 1000) );
                }
            }
            else if( msg.what == PlayerActivity.PLAYING.ordinal() ) {
                mPlaybackController.start();
                sendEmptyMessage( UPDATE_PROGRESS );
            }
            else if( msg.what == PlayerActivity.STOPPED.ordinal() ) {
                mPlaybackController.stop();
                removeMessages( UPDATE_PROGRESS );
            }
            else if( msg.what == PlayerActivity.FINISHED.ordinal() ) {
                mPlaybackController.reset();
                removeMessages( UPDATE_PROGRESS );
            }
        }
    }
}
