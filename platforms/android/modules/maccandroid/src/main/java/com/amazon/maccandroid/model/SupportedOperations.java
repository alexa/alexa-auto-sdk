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

package com.amazon.maccandroid.model;

import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_DISABLE_REPEAT;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_DISABLE_SHUFFLE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_REPEAT;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_REPEAT_ONE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_SHUFFLE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_FASTFORWARD;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_FAVORITE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_NEXT;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_PAUSE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_PLAY;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_PREVIOUS;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_REWIND;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_STARTOVER;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_STOP;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_UNFAVORITE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.SEEK_CONTROL_SET_SEEK_POSITION;

import android.media.Rating;
import android.support.v4.media.session.PlaybackStateCompat;

import com.amazon.maccandroid.Log;
import com.amazon.maccandroid.MediaApp;

import java.util.HashSet;
import java.util.Set;

public class SupportedOperations {
    private static final String TAG = SupportedOperations.class.getSimpleName();

    public Set<String> getSupportedOperations(MediaApp app) {
        Set<String> operations = new HashSet<>();
        PlaybackStateCompat playbackState = app.getPlayerPlaybackInfo().getPlaybackState();
        if (playbackState == null) {
            return operations;
        }
        long actions = playbackState.getActions();
        int currentPlaybackState = playbackState.getState();
        Log.i(TAG, "supportedOperations: " + actions);
        if (((actions & PlaybackStateCompat.ACTION_PLAY) != 0)) {
            if (currentPlaybackState != PlaybackStateCompat.STATE_PLAYING)
                operations.add(PLAY_CONTROL_PLAY);
        }
        if (((actions & PlaybackStateCompat.ACTION_PAUSE) != 0)) {
            if (currentPlaybackState == PlaybackStateCompat.STATE_PLAYING)
                operations.add(PLAY_CONTROL_PAUSE);
        }
        if ((actions & PlaybackStateCompat.ACTION_STOP) != 0) {
            if (currentPlaybackState == PlaybackStateCompat.STATE_PLAYING)
                operations.add(PLAY_CONTROL_STOP);
        }
        if ((actions & PlaybackStateCompat.ACTION_SEEK_TO) != 0) {
            operations.add(PLAY_CONTROL_STARTOVER);
            operations.add(SEEK_CONTROL_SET_SEEK_POSITION);
        }
        if ((actions & PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS) != 0) {
            operations.add(PLAY_CONTROL_PREVIOUS);
        }
        if ((actions & PlaybackStateCompat.ACTION_SKIP_TO_NEXT) != 0) {
            operations.add(PLAY_CONTROL_NEXT);
        }
        if ((actions & PlaybackStateCompat.ACTION_REWIND) != 0) {
            operations.add(PLAY_CONTROL_REWIND);
        }
        if ((actions & PlaybackStateCompat.ACTION_FAST_FORWARD) != 0) {
            operations.add(PLAY_CONTROL_FASTFORWARD);
        }
        if ((actions & PlaybackStateCompat.ACTION_SET_RATING) != 0
                && app.getMediaController().getRatingType() == Rating.RATING_THUMB_UP_DOWN) {
            operations.add(PLAY_CONTROL_FAVORITE);
            operations.add(PLAY_CONTROL_UNFAVORITE);
        }
        if ((actions & PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE) != 0
                || (actions & PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE_ENABLED) != 0) {
            operations.add(PLAY_CONTROL_ENABLE_SHUFFLE);
            operations.add(PLAY_CONTROL_DISABLE_SHUFFLE);
        }
        if ((actions & PlaybackStateCompat.ACTION_SET_REPEAT_MODE) != 0) {
            operations.add(PLAY_CONTROL_ENABLE_REPEAT);
            operations.add(PLAY_CONTROL_ENABLE_REPEAT_ONE);
            operations.add(PLAY_CONTROL_DISABLE_REPEAT);
        }

        Log.i(TAG, "supportedOperations: " + operations);
        return operations;
    }
}
