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

package com.amazon.sampleapp.impl.SpeechRecognizer;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.support.v4.app.ActivityCompat;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.amazon.aace.alexa.SpeechRecognizer;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.Observable;
import java.util.Observer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;

public class SpeechRecognizerHandler extends SpeechRecognizer {

    private static final String sTag = "SpeechRecognizer";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private AudioCueObservable mAudioCueObservable = new AudioCueObservable();
    private final ExecutorService mExecutor = Executors.newFixedThreadPool( 2 );
    private AudioRecord mAudioInput;
    private AudioReader mReader;
    private boolean mWakeWordEnabled;
    private boolean mAllowStopCapture = false; // Only true if holdToTalk() returned true

    public SpeechRecognizerHandler( Activity activity,
                                    LoggerHandler logger,
                                    boolean wakeWordSupported,
                                    boolean wakeWordEnabled ) {
        super( wakeWordSupported && wakeWordEnabled );
        mActivity = activity;
        mLogger = logger;
        mWakeWordEnabled = wakeWordEnabled;
        mAudioInput = createAudioInput();

        setupGUI( wakeWordSupported );
    }

    private AudioRecord createAudioInput() {
        AudioRecord audioRecord = null;
        try {
            audioRecord = new AudioRecord( MediaRecorder.AudioSource.MIC, 16000,
                    AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT,
                    1024 );
        } catch ( IllegalArgumentException e ) {
            mLogger.postError( sTag, "Cannot create audio input. Error: "
                    + e.getMessage() );
        }
        return audioRecord;
    }

    @Override
    public boolean startAudioInput() {
        if ( mAudioInput == null ) {
            mLogger.postWarn( sTag,
                    "Cannot start audio input. AudioRecord could not be created" );
            return false;
        }

        if ( mAudioInput.getState() != AudioRecord.STATE_INITIALIZED ) {
            if ( ActivityCompat.checkSelfPermission( mActivity, Manifest.permission.RECORD_AUDIO )
                    == PackageManager.PERMISSION_DENIED ) {
                mLogger.postWarn( sTag,
                        "Cannot start audio input. Microphone permission not granted" );
                return false;
            } else {
                // Retry AudioRecord initialization. Microphone permission may have been granted
                mAudioInput = createAudioInput();
                if ( mAudioInput.getState() != AudioRecord.STATE_INITIALIZED ) {
                    mLogger.postWarn( sTag, "Cannot initialize AudioRecord" );
                    return false;
                }
            }
        }
        return startRecording();
    }

    private boolean startRecording() {
        if ( mReader != null && mReader.isRunning() ) {
            mLogger.postWarn( sTag,
                    "startRecording() called but AudioRecorder thread is already running" );
            return false;
        } else {
            // Start audio recording
            try {
                mAudioInput.startRecording();
            } catch ( IllegalStateException e ) {
                mLogger.postError( sTag, "AudioRecord cannot start recording. Error: "
                        + e.getMessage() );
                return false;
            }

            // Read recorded audio samples and pass to engine
            try {
                mExecutor.submit( mReader = new AudioReader() ); // Submit the audio reader thread
            } catch ( RejectedExecutionException e ) {
                mLogger.postError( sTag,
                        "Audio reader task cannot be scheduled for execution. Error: "
                                + e.getMessage() );
                return false;
            }
            return true;
        }
    }

    @Override
    public boolean stopAudioInput() {
        if ( mAudioInput == null ) {
            mLogger.postWarn( sTag,
                    "stopAudioInput() called but AudioRecord was never initialized" );
            return false;
        }

        // Cancel the audio reader and stop recording
        if ( mReader != null ) mReader.cancel();
        try {
            mAudioInput.stop();
        } catch ( IllegalStateException e ) {
            mLogger.postError( sTag, "AudioRecord cannot stop recording. Error: "
                    + e.getMessage() );
            return false;
        }

        return true;
    }

    @Override
    public boolean wakewordDetected( String wakeWord ) {
        mAudioCueObservable.playAudioCue( AudioCueState.START_VOICE );
        return true;
    }

    @Override
    public void endOfSpeechDetected() {
        mAudioCueObservable.playAudioCue( AudioCueState.END );
    }

    public void onTapToTalk() {
        if ( tapToTalk() ) mAudioCueObservable.playAudioCue( AudioCueState.START_TOUCH );
    }

    public void onHoldToTalk() {
        mAllowStopCapture = false;
        if ( holdToTalk() ) {
            mAllowStopCapture = true;
            mAudioCueObservable.playAudioCue( AudioCueState.START_TOUCH );
        }
    }

    public void onReleaseHoldToTalk() {
        if ( mAllowStopCapture ) stopCapture();
        mAllowStopCapture = false;
    }

    //
    // AudioReader class
    //

    private class AudioReader implements Runnable {

        private boolean mRunning = true;
        private byte[] mBuffer = new byte[ 300 ];

        void cancel() { mRunning = false; }

        boolean isRunning() { return mRunning; }

        @Override
        public void run() {
            int size ;

            while ( mRunning ) {
                size = mAudioInput.read( mBuffer, 0, mBuffer.length );
                if ( size > 0 && mRunning ) write( mBuffer, size ); // Write audio samples to engine
            }
        }
    }

    private void setupGUI( boolean wakeWordSupported ) {
        // Toggle Wake Word switch
        final View toggleItem = mActivity.findViewById( R.id.toggleWakeWord );
        ( ( TextView ) toggleItem.findViewById( R.id.text ) ).setText( R.string.wake_word_enabled );

        // Wake Word not supported message
        final View message = mActivity.findViewById( R.id.wakeWordNotSupportedMessage );

        // Show toggle Wake Word option only if Wake Word supported
        if ( wakeWordSupported ) {

            final SwitchCompat wakeWordSwitch = toggleItem.findViewById( R.id.drawerSwitch );

            mActivity.runOnUiThread( new Runnable() {
                @Override
                public void run() {
                    toggleItem.setVisibility( View.VISIBLE );
                    message.setVisibility( View.GONE );
                    wakeWordSwitch.setChecked( mWakeWordEnabled );
                }
            } );

            wakeWordSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener
                    () {
                @Override
                public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                    if ( isChecked ) {
                        mLogger.postInfo( sTag, "Enabling Wake Word" );
                        mExecutor.submit( new Runnable() {
                            @Override
                            public void run() {
                                enableWakewordDetection();
                            }
                        } );
                    } else {
                        mLogger.postInfo( sTag, "Disabling Wake Word" );
                        mExecutor.submit( new Runnable() {
                            @Override
                            public void run() {
                                disableWakewordDetection();
                            }
                        } );
                    }
                    mWakeWordEnabled = isChecked;
                }
            } );
        } else {
            mActivity.runOnUiThread( new Runnable() {
                @Override
                public void run() {
                    toggleItem.setVisibility( View.GONE );
                    message.setVisibility( View.VISIBLE );
                }
            } );
        }
    }

    /* For playing speech recognition audio cues */

    public enum AudioCueState { START_TOUCH, START_VOICE, END }

    public static class AudioCueObservable extends Observable {

        void playAudioCue( AudioCueState state ) {
            setChanged();
            notifyObservers( state );
        }
    }

    public void addObserver( Observer observer ) {
        if ( mAudioCueObservable == null ) mAudioCueObservable = new AudioCueObservable();
        mAudioCueObservable.addObserver( observer );
    }
}
