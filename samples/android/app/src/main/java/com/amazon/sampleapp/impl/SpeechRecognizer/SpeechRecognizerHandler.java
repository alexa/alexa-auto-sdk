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

package com.amazon.sampleapp.impl.SpeechRecognizer;

import android.app.Activity;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.amazon.aace.alexa.AlexaProperties;
import com.amazon.aace.alexa.SpeechRecognizer;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PropertyManager.PropertyManagerHandler;

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
    private final ExecutorService mExecutor = Executors.newFixedThreadPool(1);
    private boolean mAllowStopCapture = false; // Only true if holdToTalk() returned true
    private final View mToggleItem;
    private final View mMessage;
    private final TextView mLocaleMessage;
    private PropertyManagerHandler mPropertyManager;
    // AutoVoiceChrome controller

    public SpeechRecognizerHandler(Activity activity, LoggerHandler logger, PropertyManagerHandler propertyManager) {
        mActivity = activity;
        mLogger = logger;

        // Toggle Wake Word switch
        mToggleItem = mActivity.findViewById(R.id.toggleWakeWord);
        ((TextView) mToggleItem.findViewById(R.id.text)).setText(R.string.wake_word_enabled);

        // Wake Word not supported message
        mMessage = mActivity.findViewById(R.id.wakeWordNotSupportedMessage);

        // Wakeword locale switching Message
        mLocaleMessage = mActivity.findViewById(R.id.wakeWordLocaleChangeMessage);

        mPropertyManager = propertyManager;

        disableWakeWordUI();
    }

    @Override
    public boolean wakewordDetected(String wakeWord) {
        mAudioCueObservable.playAudioCue(AudioCueState.START_VOICE);

        // Notify Error state to AutoVoiceChrome if disconnected with Alexa

        return true;
    }

    @Override
    public void endOfSpeechDetected() {
        mAudioCueObservable.playAudioCue(AudioCueState.END);
    }

    public void onTapToTalk() {
        if (tapToTalk())
            mAudioCueObservable.playAudioCue(AudioCueState.START_TOUCH);
    }

    public void onHoldToTalk() {
        mAllowStopCapture = false;
        if (holdToTalk()) {
            mAllowStopCapture = true;
            mAudioCueObservable.playAudioCue(AudioCueState.START_TOUCH);
        }
    }

    public void onReleaseHoldToTalk() {
        if (mAllowStopCapture)
            stopCapture();
        mAllowStopCapture = false;
    }

    private void disableWakeWordUI() {
        mToggleItem.setVisibility(View.GONE);
        mMessage.setVisibility(View.VISIBLE);
        mLocaleMessage.setVisibility(View.GONE);
    }

    public void enableWakeWordUI() {
        // Show toggle Wake Word option only if Wake Word supported
        final SwitchCompat wakeWordSwitch = mToggleItem.findViewById(R.id.drawerSwitch);
        mToggleItem.setVisibility(View.VISIBLE);
        mMessage.setVisibility(View.GONE);
        boolean wakeWordEnabled = mPropertyManager.getProperty(AlexaProperties.WAKEWORD_ENABLED).equals("true");
        wakeWordSwitch.setChecked(wakeWordEnabled);
        mLocaleMessage.setVisibility(View.VISIBLE);

        wakeWordSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    mLogger.postInfo(TAG, "Enabling Wake Word");
                    mExecutor.submit(new Runnable() {
                        @Override
                        public void run() {
                            mPropertyManager.setProperty(AlexaProperties.WAKEWORD_ENABLED, "true");
                        }
                    });

                    mLocaleMessage.setVisibility(View.VISIBLE);
                } else {
                    mLogger.postInfo(TAG, "Disabling Wake Word");
                    mExecutor.submit(new Runnable() {
                        @Override
                        public void run() {
                            mPropertyManager.setProperty(AlexaProperties.WAKEWORD_ENABLED, "false");
                        }
                    });
                    mLocaleMessage.setVisibility(View.GONE);
                }
            }
        });
    }

    /* For playing speech recognition audio cues */

    public enum AudioCueState { START_TOUCH, START_VOICE, END }

    public static class AudioCueObservable extends Observable {
        void playAudioCue(AudioCueState state) {
            setChanged();
            notifyObservers(state);
        }
    }

    public void addObserver(Observer observer) {
        if (mAudioCueObservable == null)
            mAudioCueObservable = new AudioCueObservable();
        mAudioCueObservable.addObserver(observer);
    }

    // AutoVoiceChrome related functions
}
