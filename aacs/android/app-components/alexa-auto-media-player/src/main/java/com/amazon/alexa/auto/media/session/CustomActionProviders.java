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
package com.amazon.alexa.auto.media.session;

import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_LOOP;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_NEXT;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_PREVIOUS;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_SHUFFLE;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_SKIP_BACKWARD;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_SKIP_FORWARD;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_THUMBS_DOWN;
import static com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_THUMBS_UP;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.alexa.auto.aacs.common.PlaybackControl;
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo;
import com.amazon.alexa.auto.media.R;

import java.util.ArrayList;
import java.util.List;

/**
 * A collection of all custom action providers.
 */
public class CustomActionProviders {
    @NonNull
    @VisibleForTesting
    CustomActionProvider mSkipForwardProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mSkipBackwardProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mThumbsUpSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mThumbsUpNotSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mThumbsDownSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mThumbsDownNotSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mShuffleSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mShuffleNotSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mLoopSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mLoopNotSelectedProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mNextDisabledProvider;
    @NonNull
    @VisibleForTesting
    CustomActionProvider mPreviousDisabledProvider;

    public CustomActionProviders(@NonNull Context context, @NonNull PlaybackControlMessages messageSender) {
        mSkipForwardProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_skip_forward_selected, context.getString(R.string.playback_control_skip_forward),
                PlaybackConstants.PlaybackButton.SKIP_FORWARD, false, false, false);
        mSkipBackwardProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_skip_backward_selected, context.getString(R.string.playback_control_skip_backward),
                PlaybackConstants.PlaybackButton.SKIP_BACKWARD, false, false, false);
        mThumbsUpSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_like_selected, context.getString(R.string.playback_control_thumbsup),
                PlaybackConstants.ToggleButton.THUMBS_UP, true, false, false); // On click un-select thumbs up
        mThumbsUpNotSelectedProvider = new PlaybackControlButtonActionProvider(messageSender, R.drawable.media_like,
                context.getString(R.string.playback_control_thumbsup), PlaybackConstants.ToggleButton.THUMBS_UP, true,
                true, false); // On click select thumbs up
        mThumbsDownSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_dislike_selected, context.getString(R.string.playback_control_thumbsdown),
                PlaybackConstants.ToggleButton.THUMBS_DOWN, true, false, false); // On click un-select thumbs down
        mThumbsDownNotSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_dislike, context.getString(R.string.playback_control_thumbsdown),
                PlaybackConstants.ToggleButton.THUMBS_DOWN, true, true, false); // On click select thumbs down
        mShuffleSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_shuffle_selected, context.getString(R.string.playback_control_shuffle),
                PlaybackConstants.ToggleButton.SHUFFLE, true, false, false); // On click un-select shuffle
        mShuffleNotSelectedProvider = new PlaybackControlButtonActionProvider(messageSender, R.drawable.media_shuffle,
                context.getString(R.string.playback_control_shuffle), PlaybackConstants.ToggleButton.SHUFFLE, true,
                true, false); // On click select shuffle
        mLoopSelectedProvider = new PlaybackControlButtonActionProvider(messageSender, R.drawable.media_repeat_selected,
                context.getString(R.string.playback_control_loop), PlaybackConstants.ToggleButton.LOOP, true, false,
                false); // On click un-select loop
        mLoopNotSelectedProvider = new PlaybackControlButtonActionProvider(messageSender, R.drawable.media_repeat,
                context.getString(R.string.playback_control_loop), PlaybackConstants.ToggleButton.LOOP, true, true,
                false); // On click select loop
        mPreviousDisabledProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_skip_previous_disabled, context.getString(R.string.playback_control_previous),
                PlaybackConstants.PlaybackButton.NEXT, false, false, true);
        mNextDisabledProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_skip_next_disabled, context.getString(R.string.playback_control_next),
                PlaybackConstants.PlaybackButton.PREVIOUS, false, false, true);
    }

    /**
     * Compute and return the custom action providers for given render player
     * info.
     *
     * @param playerInfo Render player info.
     * @return List of custom action providers.
     */
    public List<CustomActionProvider> computeCustomActionProviders(@NonNull RenderPlayerInfo playerInfo) {
        // Custom actions appear in the UI in the order they are added in this
        // list.
        List<CustomActionProvider> actionProviders = new ArrayList<>();

        if (playerInfo.isControlEnabled(CONTROL_NAME_SKIP_BACKWARD)) {
            actionProviders.add(mSkipBackwardProvider);
        }

        if (playerInfo.isControlEnabled(CONTROL_NAME_SKIP_FORWARD)) {
            actionProviders.add(mSkipForwardProvider);
        }

        PlaybackControl thumbsUp = playerInfo.getControl(CONTROL_NAME_THUMBS_UP);
        if (thumbsUp != null) {
            actionProviders.add(thumbsUp.getSelected() ? mThumbsUpSelectedProvider : mThumbsUpNotSelectedProvider);
        }

        PlaybackControl thumbsDown = playerInfo.getControl(CONTROL_NAME_THUMBS_DOWN);
        if (thumbsDown != null) {
            actionProviders.add(
                    thumbsDown.getSelected() ? mThumbsDownSelectedProvider : mThumbsDownNotSelectedProvider);
        }

        PlaybackControl shuffle = playerInfo.getControl(CONTROL_NAME_SHUFFLE);
        if (shuffle != null) {
            actionProviders.add(shuffle.getSelected() ? mShuffleSelectedProvider : mShuffleNotSelectedProvider);
        }

        PlaybackControl loop = playerInfo.getControl(CONTROL_NAME_LOOP);
        if (loop != null) {
            actionProviders.add(loop.getSelected() ? mLoopSelectedProvider : mLoopNotSelectedProvider);
        }

        PlaybackControl previous = playerInfo.getControl(CONTROL_NAME_PREVIOUS);
        if (previous != null && !previous.getEnabled()) {
            actionProviders.add(mPreviousDisabledProvider);
        }

        PlaybackControl next = playerInfo.getControl(CONTROL_NAME_NEXT);
        if (next != null && !next.getEnabled()) {
            actionProviders.add(mNextDisabledProvider);
        }

        return actionProviders;
    }
}
