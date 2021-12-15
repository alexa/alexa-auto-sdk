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

package com.amazon.maccandroid.model.state;

import android.os.SystemClock;
import android.support.v4.media.MediaMetadataCompat;
import android.support.v4.media.session.PlaybackStateCompat;

import com.amazon.maccandroid.MediaApp;
import com.amazon.maccandroid.model.PlayBackStateFields;
import com.amazon.maccandroid.model.SupportedOperations;

import java.util.HashSet;
import java.util.Set;

public class MediaAppPlaybackState {
    private static final String EXTERNAL_MEDIA_PLAYER_TYPE = "ExternalMediaPlayerMusicItem";
    private static final String MEDIA_TYPE_TRACK = "TRACK";
    private static final String MEDIA_TYPE_AD = "AD";

    private final Set<String> mSupportedOperations;
    private final PlayBackStateFields.State mPlaybackState;
    private final long mPositionMilliseconds;
    private final PlayBackStateFields.shuffle mShuffleMode;
    private final PlayBackStateFields.repeat mRepeatMode;
    private final PlayBackStateFields.favorite mFavourite;
    private final MediaAppMetaData mMediaAppMetaData;

    public MediaAppPlaybackState(MediaApp app, SupportedOperations supportedOperations) {
        String type = EXTERNAL_MEDIA_PLAYER_TYPE;

        if (app.getPlayerPlaybackInfo() == null || app.getPlayerPlaybackInfo().getPlaybackState() == null) {
            mSupportedOperations = new HashSet<>(0);
            mPlaybackState = null;
            mPositionMilliseconds = 0;
            mShuffleMode = null;
            mRepeatMode = null;
            mFavourite = null;
            mMediaAppMetaData = new MediaAppMetaData();
            return;
        }
        PlaybackStateCompat playBackState = app.getPlayerPlaybackInfo().getPlaybackState();

        mPlaybackState = getExternalMediaPlayerState(app.getPlayerPlaybackInfo().getPlaybackState().getState());
        mSupportedOperations = supportedOperations.getSupportedOperations(app);
        mPositionMilliseconds = getPositionMilliseconds(playBackState);
        mShuffleMode = getShuffleEnabled(app.getPlayerPlaybackInfo().getShuffleMode());
        mRepeatMode = getRepeatEnabled(app.getPlayerPlaybackInfo().getRepeatMode());
        mFavourite = getFavorite(app.getPlayerPlaybackInfo().getMetadata());
        MediaMetadataCompat metadata = app.getPlayerPlaybackInfo().getMetadata();
        mMediaAppMetaData = metadata == null ? new MediaAppMetaData() : new MediaAppMetaData(metadata);
    }

    private PlayBackStateFields.favorite getFavorite(MediaMetadataCompat metadata) {
        if (metadata == null) {
            return null;
        }
        if (metadata.getRating(MediaMetadataCompat.METADATA_KEY_RATING) == null) {
            return null;
        }
        if (!metadata.getRating(MediaMetadataCompat.METADATA_KEY_RATING).isRated()) {
            return PlayBackStateFields.favorite.NOT_RATED;
        } else if (metadata.getRating(MediaMetadataCompat.METADATA_KEY_RATING).isThumbUp()) {
            return PlayBackStateFields.favorite.FAVORITED;
        } else {
            return PlayBackStateFields.favorite.UNFAVORITED;
        }
    }

    private PlayBackStateFields.repeat getRepeatEnabled(int repeatMode) {
        switch (repeatMode) {
            case PlaybackStateCompat.REPEAT_MODE_ALL:
            case PlaybackStateCompat.REPEAT_MODE_GROUP:
                return PlayBackStateFields.repeat.REPEATED;
            case PlaybackStateCompat.REPEAT_MODE_ONE:
                return PlayBackStateFields.repeat.ONE_REPEATED;
            case PlaybackStateCompat.REPEAT_MODE_NONE:
                //            case PlaybackStateCompat.REPEAT_MODE_INVALID:
                //                return PlayBackStateFields.repeat.NOT_REPEATED;
        }
        return PlayBackStateFields.repeat.NOT_REPEATED;
    }

    private PlayBackStateFields.shuffle getShuffleEnabled(int shuffleMode) {
        if (shuffleMode == PlaybackStateCompat.SHUFFLE_MODE_ALL
                || shuffleMode == PlaybackStateCompat.SHUFFLE_MODE_GROUP) {
            return PlayBackStateFields.shuffle.SHUFFLED;
        } else {
            return PlayBackStateFields.shuffle.NOT_SHUFFLED;
        }
    }

    private long getPositionMilliseconds(PlaybackStateCompat playbackState) {
        if (playbackState.getLastPositionUpdateTime() > 0) {
            return (long) (playbackState.getPosition()
                    + playbackState.getPlaybackSpeed()
                            * (SystemClock.elapsedRealtime() - playbackState.getLastPositionUpdateTime()));
        }
        return 0;
    }

    private PlayBackStateFields.State getExternalMediaPlayerState(int state) {
        switch (state) {
            case PlaybackStateCompat.STATE_NONE:
                return PlayBackStateFields.State.IDLE;
            case PlaybackStateCompat.STATE_PLAYING:
                return PlayBackStateFields.State.PLAYING;
            case PlaybackStateCompat.STATE_PAUSED:
                return PlayBackStateFields.State.PAUSED;
            case PlaybackStateCompat.STATE_STOPPED:
                return PlayBackStateFields.State.STOPPED;
            case PlaybackStateCompat.STATE_FAST_FORWARDING:
                return PlayBackStateFields.State.FAST_FORWARDING;
            case PlaybackStateCompat.STATE_REWINDING:
                return PlayBackStateFields.State.REWINDING;
            case PlaybackStateCompat.STATE_BUFFERING:
                return PlayBackStateFields.State.BUFFER_UNDERRUN;
        }
        return null;
    }

    public Set<String> getSupportedOperations() {
        return mSupportedOperations;
    }

    public PlayBackStateFields.State getPlaybackState() {
        return mPlaybackState;
    }

    public long getPositionMilliseconds() {
        return mPositionMilliseconds;
    }

    public PlayBackStateFields.shuffle isShuffleEnabled() {
        return mShuffleMode;
    }

    public PlayBackStateFields.repeat getRepeatMode() {
        return mRepeatMode;
    }

    public PlayBackStateFields.favorite getFavourite() {
        return mFavourite;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }

        if (!MediaAppPlaybackState.class.isAssignableFrom(obj.getClass())) {
            return false;
        }

        final MediaAppPlaybackState otherPlayBackState = (MediaAppPlaybackState) obj;

        if (!mPlaybackState.equals(otherPlayBackState.mPlaybackState)) {
            return false;
        }

        if (!mSupportedOperations.equals(otherPlayBackState.mSupportedOperations)) {
            return false;
        }

        if (!(mPositionMilliseconds == otherPlayBackState.mPositionMilliseconds)) {
            return false;
        }

        if (!mShuffleMode.equals(otherPlayBackState.mShuffleMode)) {
            return false;
        }

        if (!mRepeatMode.equals(otherPlayBackState.mRepeatMode)) {
            return false;
        }

        if (!mFavourite.equals(otherPlayBackState.mFavourite)) {
            return false;
        }

        return true;
    }

    public MediaAppMetaData getMediaAppMetaData() {
        return mMediaAppMetaData;
    }
}
