package com.amazon.alexa.auto.voiceinteraction.earcon;

import android.content.Context;
import android.media.MediaPlayer;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.alexaCustomAssistant.EarconProvider;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.voiceinteraction.R;

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

    public void playAudioCueStartTouch() {
        if (mAudioCueStartTouch != null) {
            Log.d(TAG, "Start playing touch-initiated listening audio cue...");
            mAudioCueStartTouch.start();
        }
    }

    public void playAudioCueEnd() {
        playAudioCueEnd(false);
    }

    public void playAudioCueEnd(boolean alternative) {
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
