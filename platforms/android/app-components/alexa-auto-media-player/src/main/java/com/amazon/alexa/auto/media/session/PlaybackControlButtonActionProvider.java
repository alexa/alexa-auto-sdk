package com.amazon.alexa.auto.media.session;

import android.os.Bundle;
import android.support.v4.media.session.PlaybackStateCompat;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;

/**
 * Custom action provider for custom button AACS playback control (like
 * skip forward, skip backward etc.).
 */
public class PlaybackControlButtonActionProvider implements CustomActionProvider {
    @NonNull
    private final PlaybackControlMessages mMessageSender;

    private final int mButtonResourceId;
    @NonNull
    private final String mButtonDisplayName;
    @NonNull
    private final String mButtonName;
    private boolean mIsToggleAction;
    private boolean mIsToggleSelected;

    private final PlaybackStateCompat.CustomAction mPlaybackAction;

    /**
     * Constructs an instance of PlaybackControlButtonActionProvider.
     *
     * @param aacsSender To send playback control commands to AACS.
     * @param buttonResourceId Resource id for image of button.
     * @param buttonDisplayName Name of the button as it should appear
     *                          in UI.
     * @param buttonName Name of the button sent with Playback control
     *                   command to AACS.
     * @param toggleAction true if it is a toggle action, else it would
     *                     be a button action.
     * @param toggleSelected selection value to send to AACS on clicking
     *                       the toggle button.
     */
    public PlaybackControlButtonActionProvider(@NonNull PlaybackControlMessages aacsSender, int buttonResourceId,
            @NonNull String buttonDisplayName, @NonNull String buttonName, boolean toggleAction,
            boolean toggleSelected) {
        this.mMessageSender = aacsSender;

        this.mButtonResourceId = buttonResourceId;
        this.mButtonDisplayName = buttonDisplayName;
        this.mButtonName = buttonName;
        this.mIsToggleAction = toggleAction;
        this.mIsToggleSelected = toggleSelected;

        mPlaybackAction =
                new PlaybackStateCompat.CustomAction.Builder(mButtonName, mButtonDisplayName, mButtonResourceId)
                        .build();
    }

    @Override
    public void onCustomAction(String action, @Nullable Bundle extras) {
        if (mIsToggleAction) {
            mMessageSender.sendToggleCommandToAACS(this.mButtonName, this.mIsToggleSelected);
        } else {
            mMessageSender.sendButtonCommandToAACS(this.mButtonName);
        }
    }

    @NonNull
    @Override
    public PlaybackStateCompat.CustomAction getCustomAction() {
        return mPlaybackAction;
    }
}
