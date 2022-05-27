/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.voice.ui.earcon;

import android.content.Context;
import android.media.MediaPlayer;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.EarconProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apps.common.util.EarconSoundSettingsProvider;
import com.amazon.alexa.auto.voice.ui.R;

/**
 * Alexa Auto Earcon Controller.
 */
public class EarconController {
    private static final String TAG = EarconController.class.getCanonicalName();

    @NonNull
    private final Context mContext;

    private MediaPlayer mAudioCueStartVoice; // Voice-initiated listening audio cue
    private MediaPlayer mAudioCueStartTouch; // Touch-initiated listening audio cue
    private MediaPlayer mAudioCueEnd; // End of listening audio cue

    private MediaPlayer mAlternativeAudioCueStartVoice; // Alternative Voice-initiated listening audio cue
    private MediaPlayer mAlternativeAudioCueEnd; // Alternative End of listening audio cue

    public EarconController(@NonNull Context context) {
        mContext = context;
    }

    public void initEarcon() {
        Log.d(TAG, "Initialize Alexa Auto Earcon...");
        mAudioCueStartVoice = MediaPlayer.create(mContext, R.raw.med_ui_wakesound);
        mAudioCueStartTouch = MediaPlayer.create(mContext, R.raw.med_ui_wakesound_touch);
        mAudioCueEnd = MediaPlayer.create(mContext, R.raw.med_ui_endpointing);

        if (mContext != null) {
            AlexaApp app = AlexaApp.from(mContext);
            if (app.getRootComponent().getComponent(EarconProvider.class).isPresent()) {
                Log.d(TAG, "Initialize Alternative Alexa Auto Earcon...");
                int alternativeAudioCueStartVoiceRes =
                        app.getRootComponent().getComponent(EarconProvider.class).get().getAudioCueStartVoice();
                mAlternativeAudioCueStartVoice = MediaPlayer.create(mContext, alternativeAudioCueStartVoiceRes);
                int alternativeAudioCueEndRes =
                        app.getRootComponent().getComponent(EarconProvider.class).get().getAudioCueEnd();
                mAlternativeAudioCueEnd = MediaPlayer.create(mContext, alternativeAudioCueEndRes);
            }
        }
    }

    public void uninitEarcon() {
        Log.d(TAG, "Uninitialize Alexa Auto Earcon...");
        if (mAudioCueStartVoice != null) {
            mAudioCueStartVoice.release();
            mAudioCueStartVoice = null;
        }
        if (mAudioCueStartTouch != null) {
            mAudioCueStartTouch.release();
            mAudioCueStartTouch = null;
        }
        if (mAudioCueEnd != null) {
            mAudioCueEnd.release();
            mAudioCueEnd = null;
        }
        AlexaApp app = AlexaApp.from(mContext);
        if (app.getRootComponent().getComponent(EarconProvider.class).isPresent()) {
            mAlternativeAudioCueStartVoice.release();
            mAlternativeAudioCueStartVoice = null;
            mAlternativeAudioCueEnd.release();
            mAlternativeAudioCueEnd = null;
        }
    }

    public void playAudioCueStartVoice() {
        playAudioCueStartVoice(false);
    }

    public void playAudioCueStartVoice(boolean alternative) {
        if (EarconSoundSettingsProvider.isStartEarconSettingEnabled(mContext)) {
            if (alternative) {
                if (mAlternativeAudioCueStartVoice != null) {
                    mAlternativeAudioCueStartVoice.start();
                }
            } else {
                if (mAudioCueStartVoice != null) {
                    Log.d(TAG, "Start playing voice-initiated listening audio cue...");
                    mAudioCueStartVoice.start();
                }
            }
        }
    }

    public void playAudioCueStartTouch() {
        if (mAudioCueStartTouch != null && EarconSoundSettingsProvider.isStartEarconSettingEnabled(mContext)) {
            Log.d(TAG, "Start playing touch-initiated listening audio cue...");
            mAudioCueStartTouch.start();
        }
    }

    public void playAudioCueEnd() {
        playAudioCueEnd(false);
    }

    public void playAudioCueEnd(boolean alternative) {
        if (EarconSoundSettingsProvider.isEndEarconSettingEnabled(mContext)) {
            if (alternative) {
                if (mAlternativeAudioCueEnd != null) {
                    mAlternativeAudioCueEnd.start();
                }
            } else {
                if (mAudioCueEnd != null) {
                    Log.d(TAG, "Start playing end audio cue...");
                    mAudioCueEnd.start();
                }
            }
        }
    }
}
