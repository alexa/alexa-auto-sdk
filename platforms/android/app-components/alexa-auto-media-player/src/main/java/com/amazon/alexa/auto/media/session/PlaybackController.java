package com.amazon.alexa.auto.media.session;

import static com.google.android.exoplayer2.Player.REPEAT_MODE_OFF;

import android.os.Bundle;
import android.support.v4.media.RatingCompat;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;
import com.google.android.exoplayer2.Player;

/**
 * Playback controller to implement playback commands such as play/pause/seek
 * etc.
 */
public class PlaybackController {
    @NonNull
    private final PlaybackControlMessages mMessageSender;

    /**
     * Construct an instance of @c ControlDispatcherImpl.
     *
     * @param sender Message sender for playback control.
     */
    public PlaybackController(@NonNull PlaybackControlMessages sender) {
        this.mMessageSender = sender;
    }

    /**
     * Call this method to play/pause the media.
     *
     * @param play true for playing the media, false for pausing it.
     */
    public void setPlay(boolean play) {
        mMessageSender.sendButtonCommandToAACS(
                play ? PlaybackConstants.PlaybackButton.PLAY : PlaybackConstants.PlaybackButton.PAUSE);
    }

    /**
     * Set repeat mode for the playback.
     *
     * @param repeatMode Repeat mode.
     */
    public void setRepeatMode(@Player.RepeatMode int repeatMode) {
        mMessageSender.sendToggleCommandToAACS(PlaybackConstants.ToggleButton.REPEAT, repeatMode != REPEAT_MODE_OFF);
    }

    /**
     * Enable/Disable shuffle mode for the playback.
     *
     * @param shuffleModeEnabled Whether to enable/disable shuffle mode.
     */
    public void setShuffleModeEnabled(boolean shuffleModeEnabled) {
        mMessageSender.sendToggleCommandToAACS(PlaybackConstants.ToggleButton.SHUFFLE, shuffleModeEnabled);
    }

    /**
     * Stop the playback.
     */
    public void stop() {
        mMessageSender.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PAUSE);
    }

    /**
     * Skip to previous media item.
     */
    public void skipToPrevious() {
        mMessageSender.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PREVIOUS);
    }

    /**
     * Skip to next media item.
     */
    public void skipToNext() {
        mMessageSender.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.NEXT);
    }

    /**
     * Set the rating for currently played media item.
     *
     * @param rating Rating to set.
     */
    public void setRating(RatingCompat rating) {
        if (rating.isRated()) {
            mMessageSender.sendToggleCommandToAACS(rating.isThumbUp() ? PlaybackConstants.ToggleButton.THUMBS_UP
                                                                      : PlaybackConstants.ToggleButton.THUMBS_DOWN,
                    true);
        } else {
            mMessageSender.sendToggleCommandToAACS(PlaybackConstants.ToggleButton.THUMBS_UP, false);
            mMessageSender.sendToggleCommandToAACS(PlaybackConstants.ToggleButton.THUMBS_DOWN, false);
        }
    }

    /**
     * Seek to different position in the playback.
     *
     * @param player Player object where seek will be applied.
     * @param windowIndex Window index for seek.
     * @param positionMs Position in milliseconds.
     */
    public void seekTo(Player player, int windowIndex, long positionMs) {
        player.seekTo(windowIndex, positionMs);
    }
}
