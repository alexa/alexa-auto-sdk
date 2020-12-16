package com.amazon.alexa.auto.media.session;

import android.os.Bundle;
import android.support.v4.media.session.PlaybackStateCompat;

import androidx.annotation.Nullable;

/**
 * Interface for facilitating integration of custom controls with Media Session.
 */
public interface CustomActionProvider {
    /**
     * Provide the {@link PlaybackStateCompat.CustomAction} object that encompass
     * the icon to be displayed for custom control as well as the action name that
     * is passed to {@link CustomActionProvider#onCustomAction} when user interacts
     * with the custom control.
     *
     * @return An {@link PlaybackStateCompat.CustomAction} object representing the
     * custom action.
     */
    PlaybackStateCompat.CustomAction getCustomAction();

    /**
     * Callback that is invoked when user interacts with the custom control.
     *
     * @param action Name of the action (that is passed from
     * {@link CustomActionProvider#getCustomAction}).
     * @param extras Additional parameters.
     */
    void onCustomAction(String action, @Nullable Bundle extras);
}
