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
package com.amazon.alexaautoclientservice.modules.audioOutput.mediaPlayer;

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
                // when asked to duck, TTS should pause rather than decrease its volume.
                // set willPauseWhenDucked to true for TTS type to turn off the automatic ducking.
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,
                        AudioAttributes.USAGE_ASSISTANT, AudioAttributes.CONTENT_TYPE_SPEECH, true, false);
                break;
            case AudioType.NOTIFICATION:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,
                        AudioAttributes.USAGE_NOTIFICATION, AudioAttributes.CONTENT_TYPE_SONIFICATION, false, false);
                break;
            case AudioType.ALARM:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,
                        AudioAttributes.USAGE_ALARM, AudioAttributes.CONTENT_TYPE_SONIFICATION, false, true);
                break;
            case AudioType.EARCON:
                setAudioFocusAttributesForType(AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK,
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
                        AudioAttributes.CONTENT_TYPE_UNKNOWN, true, false);
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
