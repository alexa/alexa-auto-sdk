package com.amazon.alexa.auto.media.session;

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

    public CustomActionProviders(@NonNull Context context, @NonNull PlaybackControlMessages messageSender) {
        mSkipForwardProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_skip_forward_selected, context.getString(R.string.playback_control_skip_forward),
                PlaybackConstants.PlaybackButton.SKIP_FORWARD, false, false);
        mSkipBackwardProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_skip_backward_selected, context.getString(R.string.playback_control_skip_backward),
                PlaybackConstants.PlaybackButton.SKIP_BACKWARD, false, false);
        mThumbsUpSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_like_selected, context.getString(R.string.playback_control_thumbsup),
                PlaybackConstants.ToggleButton.THUMBS_UP, true,
                false); // On click un-select thumbs up
        mThumbsUpNotSelectedProvider = new PlaybackControlButtonActionProvider(messageSender, R.drawable.media_like,
                context.getString(R.string.playback_control_thumbsup), PlaybackConstants.ToggleButton.THUMBS_UP, true,
                true); // On click select thumbs up
        mThumbsDownSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_dislike_selected, context.getString(R.string.playback_control_thumbsdown),
                PlaybackConstants.ToggleButton.THUMBS_DOWN, true,
                false); // On click un-select thumbs down
        mThumbsDownNotSelectedProvider = new PlaybackControlButtonActionProvider(messageSender,
                R.drawable.media_dislike, context.getString(R.string.playback_control_thumbsdown),
                PlaybackConstants.ToggleButton.THUMBS_DOWN, true,
                true); // On click select thumbs down
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

        return actionProviders;
    }
}
