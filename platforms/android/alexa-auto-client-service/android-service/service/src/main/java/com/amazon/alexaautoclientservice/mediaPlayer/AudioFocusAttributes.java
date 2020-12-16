package com.amazon.alexaautoclientservice.mediaPlayer;

import android.media.AudioAttributes;
import android.media.AudioManager;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants.AudioOutput.AudioType;

public class AudioFocusAttributes {
    private static final String TAG = AACSConstants.AACS + "-" + AudioFocusAttributes.class.getSimpleName();
    public final String mType;
    public int mFocusGain;
    public int mUsage;
    public int mContentType;
    public boolean mWillPauseWhenDucked;
    public boolean mAcceptsDelayedFocusGain;

    public AudioFocusAttributes(@NonNull String type) {
        mType = type;
        switch (type) {
            case AudioType.TTS:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,
                        AudioAttributes.USAGE_ASSISTANT, AudioAttributes.CONTENT_TYPE_SPEECH, false, false);
                break;
            case AudioType.NOTIFICATION:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT,
                        AudioAttributes.USAGE_NOTIFICATION, AudioAttributes.CONTENT_TYPE_SPEECH, false, false);
                break;
            case AudioType.ALARM:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,
                        AudioAttributes.USAGE_ALARM, AudioAttributes.CONTENT_TYPE_SONIFICATION, false, true);
                break;
            case AudioType.EARCON:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT,
                        AudioAttributes.USAGE_NOTIFICATION_EVENT, AudioAttributes.CONTENT_TYPE_SONIFICATION, false,
                        false);
                break;
            case AudioType.RINGTONE:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT,
                        AudioAttributes.USAGE_NOTIFICATION_RINGTONE, AudioAttributes.CONTENT_TYPE_SONIFICATION, false,
                        false);
                break;
            case AudioType.COMMUNICATION:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_EXCLUSIVE,
                        AudioAttributes.USAGE_VOICE_COMMUNICATION, AudioAttributes.CONTENT_TYPE_SPEECH, false, false);
                break;
            default:
                Log.w(TAG, String.format("audioType=%s is not supported in audio focus management.", type));
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN, AudioAttributes.USAGE_UNKNOWN,
                        AudioAttributes.CONTENT_TYPE_UNKNOWN, false, false);
        }
    }

    public void setAudioFocusAttributesForType(
            int focusGain, int usage, int contentType, boolean willPauseWhenDucked, boolean acceptsDelayedFocusGain) {
        Log.i(TAG,
                String.format(
                        "setAudioFocusAttributesForType=%s: focusGain=%s, usage=%s, contentType=%s, willPauseWhenDucked=%s, acceptsDelayedFocusGain=%s",
                        mType, focusGain, usage, contentType, willPauseWhenDucked, acceptsDelayedFocusGain));
        mFocusGain = focusGain;
        mUsage = usage;
        mContentType = contentType;
        mWillPauseWhenDucked = willPauseWhenDucked;
        mAcceptsDelayedFocusGain = acceptsDelayedFocusGain;
    }
}