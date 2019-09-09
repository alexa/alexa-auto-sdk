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

package com.amazon.sampleapp.impl.SpeechRecognizer;

import android.app.Activity;
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
// AutoVoiceChrome imports

public class SpeechRecognizerHandler extends SpeechRecognizer {

    private static final String TAG = SpeechRecognizerHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private AudioCueObservable mAudioCueObservable = new AudioCueObservable();
    private final ExecutorService mExecutor = Executors.newFixedThreadPool( 1 );
    private boolean mWakeWordEnabled;
    private boolean mAllowStopCapture = false; // Only true if holdToTalk() returned true
    // AutoVoiceChrome controller

    public SpeechRecognizerHandler( Activity activity,
                                    LoggerHandler logger,
                                    boolean wakeWordSupported,
                                    boolean wakeWordEnabled ) {
        super( wakeWordSupported && wakeWordEnabled );
        mActivity = activity;
        mLogger = logger;
        mWakeWordEnabled = wakeWordEnabled;

        setupGUI( wakeWordSupported );
    }

    @Override
    public boolean wakewordDetected( String wakeWord ) {
        mAudioCueObservable.playAudioCue( AudioCueState.START_VOICE );

        // Notify Error state to AutoVoiceChrome if disconnected with Alexa

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

    private void setupGUI( boolean wakeWordSupported ) {
        // Toggle Wake Word switch
        final View toggleItem = mActivity.findViewById( R.id.toggleWakeWord );
        ( ( TextView ) toggleItem.findViewById( R.id.text ) ).setText( R.string.wake_word_enabled );

        // Wake Word not supported message
        final View message = mActivity.findViewById( R.id.wakeWordNotSupportedMessage );

        // Wakeword locale switching Message
        final TextView localeMessage = mActivity.findViewById(R.id.wakeWordLocaleChangeMessage);

        // Show toggle Wake Word option only if Wake Word supported
        if ( wakeWordSupported ) {

            final SwitchCompat wakeWordSwitch = toggleItem.findViewById( R.id.drawerSwitch );

            toggleItem.setVisibility( View.VISIBLE );
            message.setVisibility( View.GONE );
            wakeWordSwitch.setChecked( mWakeWordEnabled );
            if ( mWakeWordEnabled ) {
                localeMessage.setVisibility( View.VISIBLE );
            }

            wakeWordSwitch.setOnCheckedChangeListener( new CompoundButton.OnCheckedChangeListener
                    () {
                @Override
                public void onCheckedChanged( CompoundButton buttonView, boolean isChecked ) {
                    if ( isChecked ) {
                        mLogger.postInfo( TAG, "Enabling Wake Word" );
                        mExecutor.submit( new Runnable() {
                            @Override
                            public void run() {
                                enableWakewordDetection();
                            }
                        } );

                        localeMessage.setVisibility( View.VISIBLE );
                    } else {
                        mLogger.postInfo( TAG, "Disabling Wake Word" );
                        mExecutor.submit( new Runnable() {
                            @Override
                            public void run() {
                                disableWakewordDetection();
                            }
                        } );
                        localeMessage.setVisibility( View.GONE );
                    }
                    // Notify wake word changes to AutoVoiceChrome

                    mWakeWordEnabled = isChecked;
                }
            } );
        } else {
            toggleItem.setVisibility( View.GONE );
            message.setVisibility( View.VISIBLE );
            localeMessage.setVisibility( View.GONE );
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

    // AutoVoiceChrome related functions
}
