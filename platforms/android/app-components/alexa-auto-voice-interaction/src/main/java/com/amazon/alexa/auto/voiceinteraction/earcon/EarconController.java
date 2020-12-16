package com.amazon.alexa.auto.voiceinteraction.earcon;

import android.content.Context;
import android.media.MediaPlayer;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.voiceinteraction.R;

/**
 * Alexa Auto Earcon Controller.
 */
public class EarconController {
    private static final String TAG = EarconController.class.getSimpleName();

    @NonNull
    private final Context mContext;

    private MediaPlayer mAudioCueStartVoice; // Voice-initiated listening audio cue
    private MediaPlayer mAudioCueStartTouch; // Touch-initiated listening audio cue
    private MediaPlayer mAudioCueEnd; // End of listening audio cue

    public EarconController(@NonNull Context content) {
        mContext = content;
    }

    public void initEarcon() {
        Log.d(TAG, "Initialize Alexa Auto Earcon...");
        mAudioCueStartVoice = MediaPlayer.create(mContext, R.raw.med_ui_wakesound);
        mAudioCueStartTouch = MediaPlayer.create(mContext, R.raw.med_ui_wakesound_touch);
        mAudioCueEnd = MediaPlayer.create(mContext, R.raw.med_ui_endpointing);
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
    }

    public void playAudioCueStartVoice() {
        if (mAudioCueStartVoice != null) {
            Log.d(TAG, "Start playing voice-initiated listening audio cue...");
            mAudioCueStartVoice.start();
        }
    }

    public void playAudioCueStartTouch() {
        if (mAudioCueStartTouch != null) {
            Log.d(TAG, "Start playing touch-initiated listening audio cue...");
            mAudioCueStartTouch.start();
        }
    }

    public void playAudioCueEnd() {
        if (mAudioCueEnd != null) {
            Log.d(TAG, "Start playing end audio cue...");
            mAudioCueEnd.start();
        }
    }
}
