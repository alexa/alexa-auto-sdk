/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 */
package com.amazon.maccandroid.model;

import android.support.v4.media.MediaMetadataCompat;
import android.support.v4.media.session.PlaybackStateCompat;

/**
 * Class representing the playback state ({@link PlaybackStateCompat}, {@link MediaMetadataCompat},
 * etc) of a player.
 */
public class PlayerPlaybackInfo {
    private PlaybackStateCompat mPlaybackState;
    private MediaMetadataCompat mMetadata;
    int mRepeatMode;
    int mShuffleMode;

    public PlayerPlaybackInfo(
            PlaybackStateCompat playbackState, MediaMetadataCompat metadata, int repeatMode, int shuffleMode) {
        mPlaybackState = playbackState;
        mMetadata = metadata;
        mRepeatMode = repeatMode;
        mShuffleMode = shuffleMode;
    }

    public PlaybackStateCompat getPlaybackState() {
        return mPlaybackState;
    }

    public MediaMetadataCompat getMetadata() {
        return mMetadata;
    }

    public int getRepeatMode() {
        return mRepeatMode;
    }

    public int getShuffleMode() {
        return mShuffleMode;
    }

    /**
     * Builder class for {@link PlayerPlaybackInfo} to encapsulate the default values for each field and
     * make it easy to create new {@link PlayerPlaybackInfo} objects from existing {@link PlayerPlaybackInfo}
     * objects with one or more values changed.
     */
    public static class Builder {
        private static final PlaybackStateCompat DEFAULT_PLAYBACK_STATE = null;
        private static final MediaMetadataCompat DEFAULT_METADATA = null;
        private static final int DEFAULT_REPEAT_MODE = PlaybackStateCompat.REPEAT_MODE_NONE;
        private static final int DEFAULT_SHUFFLE_MODE = PlaybackStateCompat.SHUFFLE_MODE_NONE;

        private PlaybackStateCompat playbackState = DEFAULT_PLAYBACK_STATE;
        private MediaMetadataCompat metadata = DEFAULT_METADATA;
        private int repeatMode = DEFAULT_REPEAT_MODE;
        private int shuffleMode = DEFAULT_SHUFFLE_MODE;

        /**
         * Build a new {@link PlayerPlaybackInfo} using the values provided to this {@link Builder}.
         */
        public PlayerPlaybackInfo build() {
            return new PlayerPlaybackInfo(playbackState, metadata, repeatMode, shuffleMode);
        }

        /**
         * Use the values from the other {@link PlayerPlaybackInfo}.
         * <p>
         * This overwrites the values of all the fields of this {@link Builder}, so this shouldn't
         * be called after providing any other values to this {@link Builder}; call this method
         * first, instead!
         */
        public Builder playerPlaybackState(final PlayerPlaybackInfo playerPlaybackInfo) {
            if (playerPlaybackInfo == null) {
                return this;
            }
            playbackState = playerPlaybackInfo.mPlaybackState;
            metadata = playerPlaybackInfo.mMetadata;
            repeatMode = playerPlaybackInfo.mRepeatMode;
            shuffleMode = playerPlaybackInfo.mShuffleMode;
            return this;
        }

        /**
         * Set the {@link PlaybackStateCompat}.
         */
        public Builder playbackState(final PlaybackStateCompat playbackState) {
            this.playbackState = playbackState;
            return this;
        }

        /**
         * Set the {@link MediaMetadataCompat}.
         */
        public Builder metadata(final MediaMetadataCompat metadata) {
            this.metadata = metadata;
            return this;
        }

        /**
         * Set the repeat mode.
         */
        public Builder repeatMode(final int repeatMode) {
            this.repeatMode = repeatMode;
            return this;
        }

        /**
         * Set the shuffle mode.
         */
        public Builder shuffleMode(final int shuffleMode) {
            this.shuffleMode = shuffleMode;
            return this;
        }
    }
}
