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

package com.amazon.maccandroid;

import android.os.Bundle;
import android.support.v4.media.MediaMetadataCompat;
import android.support.v4.media.RatingCompat;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.media.session.PlaybackStateCompat;

import com.amazon.maccandroid.model.APIConstants;
import com.amazon.maccandroid.model.PlayerEvents;
import com.amazon.maccandroid.model.PlayerPlaybackInfo;
import com.amazon.maccandroid.model.errors.MediaAppPlayerError;
import com.amazon.maccandroid.model.state.MediaAppMetaData;
import com.amazon.maccandroid.model.state.MediaAppPlaybackState;

import java.util.HashSet;
import java.util.Set;
import java.util.UUID;

public class MediaControllerCallback extends MediaControllerCompat.Callback {
    private static final String TAG = MediaControllerCallback.class.getSimpleName();

    private final String mMediaAppPackageName;
    private PlaybackStateCompat mPrevState;
    private MediaApp mMediaApp;
    private Set<PlayerEvents> mEventsToSendQueue;
    private boolean mReportUnauthorized = false;

    MediaControllerCallback(String packageName, MediaApp mediaApp) {
        mMediaAppPackageName = packageName;
        mMediaApp = mediaApp;
        mEventsToSendQueue = new HashSet<>();
    }

    //    @Override
    //    public void onSessionReady() {
    //        super.onSessionReady();
    //        mMediaApp.setSessionReady(true);
    //    }

    @Override
    public void onSessionDestroyed() {
        super.onSessionDestroyed();
        MediaAppsRepository.getInstance().removeMediaApp(mMediaAppPackageName);
    }

    @Override
    public void onSessionEvent(String event, Bundle extras) {
        if (extras == null) {
            Log.e(TAG, "onSessionEvent extras is null");
            return;
        }
        if (APIConstants.Events.SESSION_EVENT_REQUEST_TOKEN.equals(event)) {
            MediaAppsStateReporter.getInstance().requestToken(mMediaAppPackageName);
        }
    }

    @Override
    public void onPlaybackStateChanged(PlaybackStateCompat state) {
        if (state == null) {
            Log.e(TAG, "onPlaybackStateChanged state is null");
            return;
        }
        Log.i(TAG, "onPlaybackStateChanged : " + state);
        int playbackState = state.getState();

        mMediaApp.setPlayerPlaybackInfo(new PlayerPlaybackInfo.Builder()
                                                .playerPlaybackState(mMediaApp.getPlayerPlaybackInfo())
                                                .playbackState(state)
                                                .build());

        super.onPlaybackStateChanged(state);
        if (mPrevState == null || mPrevState.getActions() != state.getActions()) {
            mEventsToSendQueue.add(PlayerEvents.PlayModeChanged);
        }

        // if only current state is differrent from previous state or there is an error
        if (mPrevState == null || mPrevState.getState() != playbackState
                || state.getState() == PlaybackStateCompat.STATE_ERROR) {
            // State changed from STATE_NONE to anything else
            if ((mPrevState == null || mPrevState.getState() == PlaybackStateCompat.STATE_NONE)
                    && playbackState != PlaybackStateCompat.STATE_NONE) {
                mEventsToSendQueue.add(PlayerEvents.PlaybackSessionStarted);
            }

            // State changed from STATE_PLAYING to anything else
            if ((mPrevState != null && mPrevState.getState() == PlaybackStateCompat.STATE_PLAYING)
                    && playbackState != PlaybackStateCompat.STATE_PLAYING) {
                mEventsToSendQueue.add(PlayerEvents.PlaybackStopped);
            }

            // If previous state is None and current state is not none then we need to create a
            // new playbacksesisonid
            if (mPrevState != null && (mPrevState.getState() == (PlaybackStateCompat.STATE_NONE))
                    && !(PlaybackStateCompat.STATE_NONE == playbackState)) {
                if (mMediaApp.getPlaybackSessionId() == null) {
                    mMediaApp.setPlaybackSessionId(UUID.randomUUID());
                }
            }

            // send error if either state had error code, or if there was an error message
            // for case where state is temporary error
            if (state.getErrorMessage() != null || playbackState == PlaybackStateCompat.STATE_ERROR) {
                handlePlaybackError(state.getErrorCode());
            }

            switch (playbackState) {
                case PlaybackStateCompat.STATE_NONE:
                    // Don't count this as the playback session ending if there was no previous
                    // state
                    if (mPrevState != null) {
                        mEventsToSendQueue.add(PlayerEvents.PlaybackSessionEnded);
                    }
                    // Playback session ended setting to null
                    mMediaApp.setPlaybackSessionId(null);
                    break;
                case PlaybackStateCompat.STATE_PLAYING:
                    mEventsToSendQueue.add(PlayerEvents.PlaybackStarted);
                    break;
                case PlaybackStateCompat.STATE_SKIPPING_TO_NEXT:
                    mEventsToSendQueue.add(PlayerEvents.PlaybackNext);
                    break;
                case PlaybackStateCompat.STATE_SKIPPING_TO_PREVIOUS:
                    mEventsToSendQueue.add(PlayerEvents.PlaybackPrevious);
                    break;
                case PlaybackStateCompat.STATE_REWINDING:
                case PlaybackStateCompat.STATE_FAST_FORWARDING:
                    mEventsToSendQueue.add(PlayerEvents.PlayModeChanged);
                    break;
                default:
                    break;
            }
        }

        mPrevState = state;
        if (!mEventsToSendQueue.isEmpty() && MediaAppsRepository.getInstance().isAuthorizedApp(mMediaAppPackageName)) {
            MediaAppsStateReporter.getInstance().reportPlayerEvent(mMediaApp.getLocalPlayerId(), mEventsToSendQueue);
            mEventsToSendQueue.clear();
        }
    }

    private void handlePlaybackError(int errorCode) {
        Log.i(TAG, "handlePlaybackError | onError: " + errorCode);
        switch (errorCode) {
            case PlaybackStateCompat.ERROR_CODE_ACTION_ABORTED:
                reportErrorEvent(MediaAppPlayerError.UNPLAYABLE_BY_ACCOUNT);
                break;

            case PlaybackStateCompat.ERROR_CODE_APP_ERROR:
                reportErrorEvent(MediaAppPlayerError.INTERNAL_ERROR);
                break;

            case PlaybackStateCompat.ERROR_CODE_AUTHENTICATION_EXPIRED:
                // block multiple extra calls for unauthorized
                if (mReportUnauthorized == true) {
                    reportErrorEvent(MediaAppPlayerError.UNPLAYABLE_BY_AUTHORIZATION);
                    mReportUnauthorized = false;
                }
                break;

            case PlaybackStateCompat.ERROR_CODE_CONCURRENT_STREAM_LIMIT:
                reportErrorEvent(MediaAppPlayerError.UNPLAYABLE_BY_STREAM_CONCURRENCY);
                break;

            case PlaybackStateCompat.ERROR_CODE_CONTENT_ALREADY_PLAYING:
                reportErrorEvent(MediaAppPlayerError.OPERATION_REJECTED_UNINTERRUPTIBLE);
                break;

            case PlaybackStateCompat.ERROR_CODE_END_OF_QUEUE:
                reportErrorEvent(MediaAppPlayerError.OPERATION_REJECTED_END_OF_QUEUE);
                break;

            case PlaybackStateCompat.ERROR_CODE_NOT_AVAILABLE_IN_REGION:
                reportErrorEvent(MediaAppPlayerError.UNPLAYABLE_BY_REGION);
                break;

            case PlaybackStateCompat.ERROR_CODE_NOT_SUPPORTED:
                reportErrorEvent(MediaAppPlayerError.OPERATION_UNSUPPORTED);
                break;

            case PlaybackStateCompat.ERROR_CODE_PARENTAL_CONTROL_RESTRICTED:
                reportErrorEvent(MediaAppPlayerError.UNPLAYABLE_BY_PARENTAL_CONTROL);
                break;

            case PlaybackStateCompat.ERROR_CODE_PREMIUM_ACCOUNT_REQUIRED:
                reportErrorEvent(MediaAppPlayerError.UNPLAYABLE_BY_SUBSCRIPTION);
                break;

            case PlaybackStateCompat.ERROR_CODE_SKIP_LIMIT_REACHED:
                reportErrorEvent(MediaAppPlayerError.OPERATION_REJECTED_SKIP_LIMIT);
                break;

            case PlaybackStateCompat.ERROR_CODE_UNKNOWN_ERROR:
                reportErrorEvent(MediaAppPlayerError.UNKNOWN_ERROR);
                break;

            default:
                Log.e(TAG, "Media App reported unknown error");
                break;
        }
    }

    private void reportErrorEvent(MediaAppPlayerError error) {
        MediaAppsStateReporter.getInstance().reportError(mMediaApp.getLocalPlayerId(), error);
    }

    @Override
    public void onMetadataChanged(MediaMetadataCompat metadata) {
        if (metadata == null) {
            Log.e(TAG, "onMetadataChanged metadata is null ");
            return;
        }
        Log.i(TAG, "onMetadataChanged: " + metadata);
        super.onMetadataChanged(metadata);
        final Set<PlayerEvents> eventsToSend = new HashSet<>();

        // Getting both current rating and previous rating
        RatingCompat currRating = metadata.getRating(MediaMetadataCompat.METADATA_KEY_RATING);
        RatingCompat prevRating = mMediaApp.getPlayerPlaybackInfo().getMetadata() == null
                ? null
                : mMediaApp.getPlayerPlaybackInfo().getMetadata().getRating(MediaMetadataCompat.METADATA_KEY_RATING);
        // If rating has changed then send a PlayModeChangedEvent
        if ((prevRating == null && currRating != null)
                || (prevRating != null && currRating != null && !currRating.equals(prevRating))) {
            eventsToSend.add(PlayerEvents.PlayModeChanged);
        }

        // any other metadata field in playbackstate changed then we need to send a track
        // changed
        if ((mMediaApp.getPlayerPlaybackInfo().getMetadata() == null)
                || (!new MediaAppMetaData(metadata).equals(
                        new MediaAppMetaData(mMediaApp.getPlayerPlaybackInfo().getMetadata())))) {
            eventsToSend.add(PlayerEvents.TrackChanged);
        }

        mMediaApp.setPlayerPlaybackInfo(new PlayerPlaybackInfo.Builder()
                                                .playerPlaybackState(mMediaApp.getPlayerPlaybackInfo())
                                                .metadata(metadata)
                                                .build());

        if (!eventsToSend.isEmpty()) {
            MediaAppsStateReporter.getInstance().reportPlayerEvent(mMediaApp.getLocalPlayerId(), eventsToSend);
        }
    }

    @Override
    public void onRepeatModeChanged(int repeatMode) {
        super.onRepeatModeChanged(repeatMode);
        mMediaApp.setPlayerPlaybackInfo(new PlayerPlaybackInfo.Builder()
                                                .playerPlaybackState(mMediaApp.getPlayerPlaybackInfo())
                                                .repeatMode(repeatMode)
                                                .build());
        sendPlayModeChanged();
    }

    @Override
    public void onShuffleModeChanged(int shuffleMode) {
        super.onShuffleModeChanged(shuffleMode);
        mMediaApp.setPlayerPlaybackInfo(new PlayerPlaybackInfo.Builder()
                                                .playerPlaybackState(mMediaApp.getPlayerPlaybackInfo())
                                                .shuffleMode(shuffleMode)
                                                .build());
        sendPlayModeChanged();
    }

    @Override
    public void onExtrasChanged(Bundle extras) {
        if (extras == null) {
            Log.e(TAG, "onExtrasChanged extras is null");
            return;
        }
        super.onExtrasChanged(extras);
        final String spiVersion = extras.getString(APIConstants.ExtrasKeys.SPI_VERSION_KEY);
        final String playerCookie = extras.getString(APIConstants.ExtrasKeys.PLAYER_COOKIE_KEY);
        if (spiVersion != null) {
            mMediaApp.setSpiVersion(spiVersion);
        }

        if (playerCookie != null) {
            mMediaApp.setPlayerCookie(playerCookie);
        }
    }

    private void sendPlayModeChanged() {
        final Set<PlayerEvents> eventsToSend = new HashSet<>();
        eventsToSend.add(PlayerEvents.PlayModeChanged);
        MediaAppsStateReporter.getInstance().reportPlayerEvent(mMediaApp.getLocalPlayerId(), eventsToSend);
    }

    // allow unauthorized to be reported again.
    public void resetUnauthorizedReported() {
        Log.i(TAG, "resetUnauthorizedReported for: " + mMediaAppPackageName);
        mReportUnauthorized = true;
    }
}
