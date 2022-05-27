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

package com.amazon.apl.android.render.tts;

import android.content.Context;
import android.media.AudioManager;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.apl.android.render.audio.AudioFocusController;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;
import com.amazon.apl.player.audio.AudioPlayer;
import com.amazon.apl.player.tts.TtsPlayer;

/**
 * Provides a wrapper of the TTS player that can report activity events
 * and request Android audio focus.
 */
public class APLTtsPlayer extends TtsPlayer implements AudioFocusController.PlaybackController {
    private static final String TAG = APLTtsPlayer.class.getSimpleName();

    private final IAPLEventSender mAplEventSender;
    private final IAPLTokenProvider mAplTokenProvider;
    private final AudioFocusController mAudioFocusController;

    public APLTtsPlayer(@NonNull Context context, @NonNull IAPLEventSender aplEventSender,
            @NonNull IAPLTokenProvider aplTokenProvider) {
        super(context);
        mAplEventSender = aplEventSender;
        mAplTokenProvider = aplTokenProvider;
        AudioManager audioManager =
                (AudioManager) context.getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
        mAudioFocusController = new AudioFocusController(audioManager, this);
    }

    @Override
    public void play() {
        Log.v(TAG, "play: intercepted for audio focus");
        mAudioFocusController.startPlaybackAfterAcquiringFocus();
    }

    @Override
    public void onPlayerStateChanged(boolean playWhenReady, @NonNull AudioPlayer.AudioPlayerState state) {
        Log.v(TAG, playWhenReady + " state: " + state);

        if (state == AudioPlayer.AudioPlayerState.STATE_PREPARING) {
            Log.v(TAG, "Acquire APL focus");
        } else if (state == AudioPlayer.AudioPlayerState.STATE_ENDED
                || state == AudioPlayer.AudioPlayerState.STATE_IDLE) {
            mAplEventSender.sendActivityEventRequest(
                    mAplTokenProvider.getToken(), IAPLEventSender.ActivityEvent.DEACTIVATED);
            Log.v(TAG, "Release APL focus");
            mAudioFocusController.relinquishAudioFocusIfCurrentlyAcquired();
        } else if (state == AudioPlayer.AudioPlayerState.STATE_READY) {
            mAplEventSender.sendActivityEventRequest(
                    mAplTokenProvider.getToken(), IAPLEventSender.ActivityEvent.ACTIVATED);
        }

        super.onPlayerStateChanged(playWhenReady, state);
    }

    @Override
    public void startPlaybackNow() {
        Log.v(TAG, "startPlaybackNow: ");
        super.play();
    }

    @Override
    public void requestResumingPlayback() {
        Log.v(TAG, "requestResumingPlayback: ");
        super.play();
    }

    @Override
    public void requestPausePlayback() {
        Log.v(TAG, "requestPausePlayback: ");
        super.stop();
    }

    @Override
    public void requestStopPlayback() {
        Log.v(TAG, "requestStopPlayback: ");
        super.stop();
    }

    @Override
    public void adjustPlaybackVolume(float volumeMultiplier) {
        Log.v(TAG, "adjustPlaybackVolume: " + volumeMultiplier);
    }

    @Override
    public void failedToAcquireFocus() {
        Log.e(TAG, "failedToAcquireFocus: ");
        super.stop();
    }
}