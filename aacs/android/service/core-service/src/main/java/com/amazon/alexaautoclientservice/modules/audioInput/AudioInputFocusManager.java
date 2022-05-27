/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.alexaautoclientservice.modules.audioInput;

import static com.amazon.alexaautoclientservice.util.FileUtil.handleAudioInputFocus;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioManager;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.alexaautoclientservice.modules.alexaClient.AlexaClientMessageHandler;
import com.amazon.alexaautoclientservice.modules.alexaClient.DialogStateObserver;
import com.amazon.alexaautoclientservice.modules.audioOutput.mediaPlayer.AudioFocusAttributes;

public class AudioInputFocusManager implements DialogStateObserver, AudioManager.OnAudioFocusChangeListener {
    public static final String TAG = AACSConstants.AACS + "-" + AudioInputFocusManager.class.getSimpleName();
    private Context mContext;
    private AlexaClientMessageHandler mAlexaClient;
    private AudioManager mAudioManager;
    private AudioAttributes mAttributes;
    private AudioFocusAttributes mAudioFocusAttributes;
    private AudioFocusRequest mPreviousAudioFocusRequest;
    private boolean hasAudioFocus;
    private ConfigurationStates mHandleAudioInputFocus = ConfigurationStates.UNINITIALIZED;
    private boolean mIsMusicPlaying = false;

    public AudioInputFocusManager(@NonNull Context context, @NonNull AlexaClientMessageHandler alexaClient) {
        mContext = context;
        mAlexaClient = alexaClient;
        mAlexaClient.registerDialogStateObserver(this);
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        mAudioFocusAttributes = new AudioFocusAttributes(AASBConstants.AudioOutput.AudioType.TTS);
        mAttributes = new AudioAttributes.Builder()
                              .setUsage(mAudioFocusAttributes.mUsage)
                              .setContentType(mAudioFocusAttributes.mContentType)
                              .build();
    }

    private AudioFocusRequest getAudioFocusRequest() {
        int focusType = mHandleAudioInputFocus == ConfigurationStates.HANDLE_AUDIO_FOCUS && !mIsMusicPlaying
                ? AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK
                : AudioManager.AUDIOFOCUS_NONE;
        Log.d(TAG, "Focus Type " + focusType);
        if (mPreviousAudioFocusRequest != null && focusType == mPreviousAudioFocusRequest.getFocusGain())
            return mPreviousAudioFocusRequest;

        if (focusType != AudioManager.AUDIOFOCUS_NONE) {
            return new AudioFocusRequest.Builder(focusType)
                    .setAudioAttributes(mAttributes)
                    .setOnAudioFocusChangeListener(this)
                    .setWillPauseWhenDucked(mAudioFocusAttributes.mWillPauseWhenDucked)
                    .setAcceptsDelayedFocusGain(mAudioFocusAttributes.mAcceptsDelayedFocusGain)
                    .build();
        } else {
            Log.v(TAG, "Input audio focus is not handled");
            return null;
        }
    }

    @Override
    public void onDialogStateChanged(@NonNull String dialogState) {
        Log.v(TAG, "onDialogStateChanged " + dialogState);
        if (mHandleAudioInputFocus == ConfigurationStates.UNINITIALIZED) {
            boolean handleAudioInputFocus = handleAudioInputFocus("VOICE");
            mHandleAudioInputFocus = handleAudioInputFocus ? ConfigurationStates.HANDLE_AUDIO_FOCUS
                                                           : ConfigurationStates.IGNORE_AUDIO_FOCUS;
        }
        switch (dialogState) {
            case AASBConstants.AlexaClient.DIALOG_STATE_IDLE:
                if (hasAudioFocus) {
                    int result = mAudioManager.abandonAudioFocusRequest(mPreviousAudioFocusRequest);
                    if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                        Log.v(TAG, "Abandoned audio focus");
                        hasAudioFocus = false;
                    }
                }
                break;
            case AASBConstants.AlexaClient.DIALOG_STATE_LISTENING:
            case AASBConstants.AlexaClient.DIALOG_STATE_EXPECTING:
            case AASBConstants.AlexaClient.DIALOG_STATE_THINKING:
                if (!hasAudioFocus) {
                    mPreviousAudioFocusRequest = getAudioFocusRequest();
                    if (mPreviousAudioFocusRequest == null)
                        break;
                    else {
                        hasAudioFocus = AudioManager.AUDIOFOCUS_REQUEST_GRANTED
                                == mAudioManager.requestAudioFocus(mPreviousAudioFocusRequest);
                    }
                }
                break;
            // No Need to act in "SPEAKING" state since audioOutput handles the TTS audio focus
            default:
                break;
        }
    }

    @Override
    public void onAudioFocusChange(int focusChange) {
        switch (focusChange) {
            case AudioManager.AUDIOFOCUS_GAIN:
            case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                hasAudioFocus = true;
                break;
            case AudioManager.AUDIOFOCUS_LOSS:
                hasAudioFocus = false;
                if (mPreviousAudioFocusRequest != null)
                    mAudioManager.abandonAudioFocusRequest(mPreviousAudioFocusRequest);
                break;
            default:
                break;
        }
    }

    public void setMediaState(String channel, String state) {
        mIsMusicPlaying = AASBConstants.AudioOutput.Channel.AUDIO_PLAYER.equals(channel)
                && MediaConstants.MediaState.PLAYING.equals(state);
        Log.d(TAG, "setMediaState isMusicPlaying " + mIsMusicPlaying);
    }

    /**
     * This enumeration used to maintain the configuration reading states
     */
    private enum ConfigurationStates { UNINITIALIZED, HANDLE_AUDIO_FOCUS, IGNORE_AUDIO_FOCUS }
}