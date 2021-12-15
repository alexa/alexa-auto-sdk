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

package com.amazon.alexaautoclientservice.modules.mediaManager;

import android.content.Context;
import android.media.AudioManager;
import android.media.session.MediaController;
import android.os.RemoteException;
import android.support.v4.media.MediaBrowserCompat;
import android.support.v4.media.RatingCompat;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.media.session.MediaSessionCompat;
import android.support.v4.media.session.PlaybackStateCompat;
import android.util.Log;

import com.amazon.aace.alexa.LocalMediaSource;
import com.amazon.aace.audio.AudioOutput;
import com.amazon.aacsconstants.AACSConstants;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class LocalMediaSourceHandler extends LocalMediaSource {
    // Dynamic Tag based on Source type
    private String TAG;

    private final MediaSource mMediaSource;
    private MediaControllerCompat mMediaController;
    private boolean isSessionActive;
    private String mSessionId;
    private Context mContext;
    private boolean isBinderDied;
    private PlaybackStateCompat mPlaybackState;
    private MediaBrowserConnectionStates mMediaBrowserConnectionStatus = MediaBrowserConnectionStates.DISCONNECTED;
    private MediaBrowserCompat mMediaBrowserCompat;

    private static final String PLAYER_EVENT_SESSION_STARTED = "PlaybackSessionStarted";
    private static final String PLAYER_EVENT_SESSION_ENDED = "PlaybackSessionEnded";
    private static final String PLAYER_EVENT_PLAYBACK_STARTED = "PlaybackStarted";
    private static final String PLAYER_EVENT_PLAYBACK_STOPPED = "PlaybackStopped";

    private enum MediaBrowserConnectionStates { DISCONNECTED, CONNECTED, FAILED }

    private enum Repeat { REPEAT_ONE, REPEAT, DISABLE_REPEAT }

    private final MediaControllerCompat.Callback mCallback = new MediaControllerCompat.Callback() {
        public void onSessionDestroyed() {
            Log.v(TAG, "onSessionDestroyed");
            mediaSessionEnded();
            mMediaController = null;
            createMediaBrowserConnection(mContext, mMediaSource);
        }

        @Override
        public void binderDied() {
            super.binderDied();
            Log.v(TAG, "MediaController is disconnected");
            isBinderDied = true;
        }

        @Override
        public void onPlaybackStateChanged(PlaybackStateCompat state) {
            if (mPlaybackState == null || mPlaybackState.getState() != state.getState()) {
                Log.v(TAG, String.format("onPlaybackStateChanged %d", state.getState()));
                if (state.getState() == PlaybackStateCompat.STATE_PLAYING) {
                    playerEvent(PLAYER_EVENT_PLAYBACK_STARTED, mSessionId);
                } else if (state.getState() == PlaybackStateCompat.STATE_STOPPED
                        || state.getState() == PlaybackStateCompat.STATE_PAUSED) {
                    playerEvent(PLAYER_EVENT_PLAYBACK_STOPPED, mSessionId);
                } else if (state.getState() == PlaybackStateCompat.STATE_ERROR) {
                    playerError("INTERNAL_ERROR", state.getState(),
                            state.getErrorMessage() != null ? state.getErrorMessage().toString() : "", false,
                            mSessionId);
                }
            }
            mPlaybackState = state;
        }
    };

    private final MediaBrowserCompat.ConnectionCallback mMediaBrowserCallback =
            new MediaBrowserCompat.ConnectionCallback() {
                @Override
                public void onConnected() {
                    super.onConnected();
                    mMediaBrowserConnectionStatus = MediaBrowserConnectionStates.CONNECTED;
                    try {
                        createMediaController(mMediaBrowserCompat.getSessionToken());
                    } catch (IllegalStateException exe) {
                        // Sometimes app may crash with java.lang.IllegalStateException: getSessionToken()
                        // called while not connected, if local media app is crashed
                        Log.e(TAG, exe.getMessage());
                    }
                }

                @Override
                public void onConnectionFailed() {
                    super.onConnectionFailed();
                    mMediaBrowserConnectionStatus = MediaBrowserConnectionStates.FAILED;
                }

                @Override
                public void onConnectionSuspended() {
                    super.onConnectionSuspended();
                    mMediaBrowserConnectionStatus = MediaBrowserConnectionStates.DISCONNECTED;
                }
            };

    public LocalMediaSourceHandler(Context context, MediaSource mediaSource) {
        super(getSourceMapping(mediaSource.getSourceType()));
        TAG = AACSConstants.AACS + "-LMS-" + getSource().name();
        mSessionId = null;
        mMediaSource = mediaSource;
        mContext = context;
        createMediaBrowserConnection(context, mediaSource);
    }

    /**
     * Create MediaBrowserConnections with only LMS since their package name and class names are received in the config
     * file. For Default media sources this is not applicable. DMS takes the latest active session always This helps one
     * - one link between LMS and one - many link between DMS
     * @param context
     * @param mediaSource
     */
    private void createMediaBrowserConnection(Context context, MediaSource mediaSource) {
        if (mediaSource.getSourceType().equals(Source.DEFAULT.name())) {
            return;
        }
        Log.v(TAG,
                String.format("createMediaBrowserConnection : Creating a new media browser for %s",
                        mediaSource.getSourceType()));
        mMediaBrowserCompat =
                new MediaBrowserCompat(context, mediaSource.getComponentName(), mMediaBrowserCallback, null);
        mMediaBrowserCompat.connect();
    }

    public MediaSource getMediaSource() {
        return mMediaSource;
    }

    private static Source getSourceMapping(String sourceType) {
        switch (sourceType) {
            case "BLUETOOTH":
                return Source.BLUETOOTH;
            case "USB":
                return Source.USB;
            case "FM_RADIO":
                return Source.FM_RADIO;
            case "AM_RADIO":
                return Source.AM_RADIO;
            case "SATELLITE_RADIO":
                return Source.SATELLITE_RADIO;
            case "LINE_IN":
                return Source.LINE_IN;
            case "COMPACT_DISC":
                return Source.COMPACT_DISC;
            case "SIRIUS_XM":
                return Source.SIRIUS_XM;
            case "DAB":
                return Source.DAB;
            default:
                return Source.DEFAULT;
        }
    }

    public void mediaSessionEnded() {
        if (mMediaController == null) {
            Log.w(TAG, "MediaController is not initialized");
        }
        if (isSessionActive) {
            playerEvent(PLAYER_EVENT_SESSION_ENDED, mSessionId);
            isSessionActive = false;
            mSessionId = null;
        }
    }

    public void mediaSessionStarted(MediaController controller) {
        isSessionActive = true;
        Log.v(TAG,
                String.format(
                        "mediaSessionStarted for %s %s", controller.getPackageName(), mMediaSource.getSourceType()));
        if (mSessionId == null)
            mSessionId = UUID.randomUUID().toString();
        // Clean up previous controller if default media source
        // Default media source always maintains one class to many source relation
        if (mMediaController != null && mMediaSource.getSourceType().equals(Source.DEFAULT.name())) {
            mMediaController.unregisterCallback(mCallback);
        }
        // Create new controller of Default media source to replace existing media source with the newly
        // active source OR use the received controller if LMS connection failed if not allowed by MediaBrowserService
        // More Info:
        // https://developer.android.com/guide/topics/media-apps/audio-app/building-a-mediabrowserservice#client-connections
        if (mMediaSource.getSourceType().equals(Source.DEFAULT.name())
                || mMediaBrowserConnectionStatus == MediaBrowserConnectionStates.FAILED) {
            try {
                mMediaController = new MediaControllerCompat(
                        mContext, MediaSessionCompat.Token.fromToken(controller.getSessionToken()));
                mMediaController.registerCallback(mCallback);
            } catch (RemoteException e) {
                Log.e(TAG, e.getMessage());
            } catch (IllegalStateException exe) {
                // Sometimes app may crash with java.lang.IllegalStateException: getSessionToken()
                // called while not connected, if local media app is crashed
                Log.e(TAG, exe.getMessage());
            }
        }
        playerEvent(PLAYER_EVENT_SESSION_STARTED, mSessionId);
    }

    @Override
    public LocalMediaSourceState getState() {
        Log.v(TAG, "getState");
        LocalMediaSourceState stateToReturn = new LocalMediaSourceState();
        stateToReturn.playbackState = new PlaybackState();
        stateToReturn.playbackState.state = getConvertedState();
        stateToReturn.playbackState.trackOffset = getOffset();
        stateToReturn.playbackState.supportedOperations = getSupportedPlaybackOperations();

        stateToReturn.playbackState.trackName = getStringMetadata(mMediaSource.getMetadataTitleKey());
        stateToReturn.playbackState.trackId = getStringMetadata(mMediaSource.getMetadataTrackIdKey());
        stateToReturn.playbackState.trackNumber =
                Long.toString(getLongMetadata(mMediaSource.getMetadataTrackNumberKey()));
        stateToReturn.playbackState.artistName = getStringMetadata(mMediaSource.getMetadataArtistKey());
        stateToReturn.playbackState.albumName = getStringMetadata(mMediaSource.getMetadataAlbumKey());
        stateToReturn.playbackState.duration = getLongMetadata(mMediaSource.getMetadataDurationKey());

        stateToReturn.sessionState = new SessionState();
        stateToReturn.sessionState.supportedContentSelectors = getContentSelector();

        return stateToReturn;
    }

    @Override
    public boolean play(ContentSelector selector, String payload, String sessionId) {
        Log.v(TAG,
                String.format("play content selector: %s, payload: %s, sessionId: %s", selector.toString(), payload,
                        sessionId));
        try {
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("ContentSelector", selector.toString());
            jsonObject.put("payload", payload);
            this.mSessionId = sessionId;
            if (mMediaController != null) {
                mMediaController.getTransportControls().prepareFromSearch(jsonObject.toString(), null);
                mMediaController.getTransportControls().playFromSearch(jsonObject.toString(), null);
                return true;
            } else {
                Log.w(TAG, "play failed since media controller is not active");
                if (mMediaController == null && mMediaBrowserConnectionStatus == MediaBrowserConnectionStates.CONNECTED
                        && mMediaBrowserCompat != null) {
                    createMediaController(mMediaBrowserCompat.getSessionToken());
                    mMediaController.getTransportControls().prepareFromSearch(jsonObject.toString(), null);
                    mMediaController.getTransportControls().playFromSearch(jsonObject.toString(), null);
                    return true;
                }
                Log.w(TAG, "play failed since media browser is also not connected");
                return false;
            }
        } catch (JSONException e) {
            Log.e(TAG, String.format("Play directive failed due to JSONException: %s", e.getMessage()));
            return false;
        }
    }

    @Override
    public boolean playControl(PlayControlType controlType) {
        boolean result;
        switch (controlType) {
            case PAUSE:
                result = pause();
                break;
            case RESUME:
                result = resume();
                break;
            case STOP:
                result = stop();
                break;
            case NEXT:
                result = next();
                break;
            case PREVIOUS:
                result = previous();
                break;
            case START_OVER:
                result = startOver();
                break;
            case FAST_FORWARD:
                result = fastForward();
                break;
            case REWIND:
                result = rewind();
                break;
            case ENABLE_REPEAT_ONE:
                result = repeat(Repeat.REPEAT_ONE);
                break;
            case ENABLE_REPEAT:
                result = repeat(Repeat.REPEAT);
                break;
            case DISABLE_REPEAT:
                result = repeat(Repeat.DISABLE_REPEAT);
                break;
            case ENABLE_SHUFFLE:
                result = shuffle(true);
                break;
            case DISABLE_SHUFFLE:
                result = shuffle(false);
                break;
            case FAVORITE:
                result = markFavorite(true);
                break;
            case UNFAVORITE:
                result = markFavorite(false);
                break;
            default:
                result = false;
                break;
        }
        return result;
    }

    @Override
    public boolean seek(long offset) {
        Log.v(TAG, "seek offset " + offset);
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_SEEK_TO)) {
            mMediaController.getTransportControls().seekTo(offset);
            return true;
        } else
            return false;
    }

    @Override
    public boolean adjustSeek(long deltaOffset) {
        Log.v(TAG, "adjustSeek deltaOffset " + deltaOffset);
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_SEEK_TO)) {
            deltaOffset += mMediaController.getPlaybackState().getPosition();
            mMediaController.getTransportControls().seekTo(deltaOffset);
            return true;
        } else
            return false;
    }

    @Override
    public boolean volumeChanged(float volume) {
        Log.v(TAG, "volumeChanged requested " + volume);
        if (isSessionActive && mMediaController != null) {
            MediaControllerCompat.PlaybackInfo playbackInfo = mMediaController.getPlaybackInfo();
            if (playbackInfo == null) {
                return false;
            }
            mMediaController.setVolumeTo((int) (volume * 100), AudioManager.FLAG_SHOW_UI);
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean mutedStateChanged(AudioOutput.MutedState state) {
        return true;
    }

    private boolean validate(MediaControllerCompat mediaController, long action) {
        if (mediaController == null || mediaController.getPlaybackState() == null) {
            Log.w(TAG, "mediaController is null");
            return false;
        }
        if ((mediaController.getPlaybackState().getActions() & action) == 0) {
            Log.w(TAG, String.format("%d action is not supported", action));
            return false;
        }
        return true;
    }

    private boolean resume() {
        if (mMediaController == null && mMediaBrowserConnectionStatus == MediaBrowserConnectionStates.CONNECTED
                && mMediaBrowserCompat != null) {
            createMediaController(mMediaBrowserCompat.getSessionToken());
        }
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_PLAY)) {
            Log.v(TAG, String.format("Resuming %s", mMediaSource.getSourceType()));
            mMediaController.getTransportControls().prepare();
            mMediaController.getTransportControls().play();
            return true;
        } else {
            Log.w(TAG, "Failed to resume");
            return false;
        }
    }

    private boolean pause() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_PAUSE)) {
            mMediaController.getTransportControls().pause();
            return true;
        } else
            return mMediaController != null && mMediaController.getPlaybackState() != null
                    && (mMediaController.getPlaybackState().getState() == PlaybackStateCompat.STATE_PAUSED
                            || mMediaController.getPlaybackState().getState() == PlaybackStateCompat.STATE_STOPPED);
    }

    private boolean stop() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_STOP)) {
            mMediaController.getTransportControls().stop();
            return true;
        } else
            return mMediaController != null && mMediaController.getPlaybackState() != null
                    && (mMediaController.getPlaybackState().getState() == PlaybackStateCompat.STATE_PAUSED
                            || mMediaController.getPlaybackState().getState() == PlaybackStateCompat.STATE_STOPPED);
    }

    private boolean next() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_SKIP_TO_NEXT)) {
            mMediaController.getTransportControls().skipToNext();
            return true;
        } else
            return false;
    }

    private boolean previous() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_SKIP_TO_PREVIOUS)) {
            mMediaController.getTransportControls().skipToPrevious();
            return true;
        } else
            return false;
    }

    private boolean fastForward() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_FAST_FORWARD)) {
            mMediaController.getTransportControls().fastForward();
            return true;
        } else
            return false;
    }

    private boolean rewind() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_REWIND)) {
            mMediaController.getTransportControls().rewind();
            return true;
        } else
            return false;
    }

    private boolean startOver() {
        if (validate(mMediaController, android.media.session.PlaybackState.ACTION_SKIP_TO_PREVIOUS)) {
            mMediaController.getTransportControls().seekTo(0);
            return true;
        } else
            return false;
    }

    private boolean repeat(Repeat repeat) {
        if (mMediaController == null) {
            Log.w(TAG, "Media Controller is not ready");
            return false;
        }
        switch (repeat) {
            case REPEAT:
                mMediaController.getTransportControls().setRepeatMode(PlaybackStateCompat.REPEAT_MODE_ALL);
                break;
            case REPEAT_ONE:
                mMediaController.getTransportControls().setRepeatMode(PlaybackStateCompat.REPEAT_MODE_ONE);
                break;
            case DISABLE_REPEAT:
                mMediaController.getTransportControls().setRepeatMode(PlaybackStateCompat.REPEAT_MODE_NONE);
                break;
        }
        return true;
    }

    private boolean shuffle(boolean shuffle) {
        if (mMediaController == null) {
            Log.w(TAG, "shuffle:Media Controller is not ready");
            return false;
        }
        if (shuffle)
            mMediaController.getTransportControls().setShuffleMode(PlaybackStateCompat.SHUFFLE_MODE_ALL);
        else
            mMediaController.getTransportControls().setShuffleMode(PlaybackStateCompat.SHUFFLE_MODE_NONE);
        return true;
    }

    private boolean markFavorite(boolean favorite) {
        if (mMediaController == null) {
            Log.w(TAG, "markFavorite:Media Controller is not ready");
            return false;
        }
        if (favorite) {
            mMediaController.getTransportControls().setRating(RatingCompat.newThumbRating(true));
        } else {
            mMediaController.getTransportControls().setRating(RatingCompat.newThumbRating(false));
        }
        return true;
    }

    private SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        if (mMediaController == null || !mMediaController.isSessionReady()
                || mMediaController.getPlaybackState() == null) {
            Log.w(TAG, "MediaController or PlaybackState is null");
            return new SupportedPlaybackOperation[] {SupportedPlaybackOperation.PLAY};
        } else {
            boolean isSessionReady = mMediaController.isSessionReady();
            long actions = mMediaController.getPlaybackState().getActions();
            List<SupportedPlaybackOperation> list = new ArrayList<>();
            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_PAUSE) != 0) {
                list.add(SupportedPlaybackOperation.PAUSE);
            }

            if ((actions & PlaybackStateCompat.ACTION_PLAY) != 0) {
                list.add(SupportedPlaybackOperation.PLAY);
            }

            if ((actions & PlaybackStateCompat.ACTION_STOP) != 0) {
                list.add(SupportedPlaybackOperation.STOP);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS) != 0) {
                list.add(SupportedPlaybackOperation.PREVIOUS);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_SKIP_TO_NEXT) != 0) {
                list.add(SupportedPlaybackOperation.NEXT);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_FAST_FORWARD) != 0) {
                list.add(SupportedPlaybackOperation.FAST_FORWARD);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_REWIND) != 0) {
                list.add(SupportedPlaybackOperation.REWIND);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_SET_SHUFFLE_MODE) != 0) {
                list.add(SupportedPlaybackOperation.ENABLE_SHUFFLE);
                list.add(SupportedPlaybackOperation.DISABLE_SHUFFLE);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_SET_REPEAT_MODE) != 0) {
                list.add(SupportedPlaybackOperation.ENABLE_REPEAT);
                list.add(SupportedPlaybackOperation.DISABLE_REPEAT);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_SET_RATING) != 0) {
                list.add(SupportedPlaybackOperation.FAVORITE);
                list.add(SupportedPlaybackOperation.UNFAVORITE);
            }

            if (isSessionReady && (actions & PlaybackStateCompat.ACTION_SEEK_TO) != 0) {
                list.add(SupportedPlaybackOperation.START_OVER);
                list.add(SupportedPlaybackOperation.SEEK);
                list.add(SupportedPlaybackOperation.ADJUST_SEEK);
            }

            SupportedPlaybackOperation[] ret = new SupportedPlaybackOperation[list.size()];
            list.toArray(ret);
            return ret;
        }
    }

    private ContentSelector[] getContentSelector() {
        List<ContentSelector> list = new ArrayList<>();
        if (mMediaSource.isPlayByChannel()) {
            list.add(ContentSelector.CHANNEL);
        }
        if (mMediaSource.isPlayByFrequency()) {
            list.add(ContentSelector.FREQUENCY);
        }
        if (mMediaSource.isPlayByPreset()) {
            list.add(ContentSelector.PRESET);
        }
        ContentSelector[] ret = new ContentSelector[list.size()];
        list.toArray(ret);
        return ret;
    }

    private String getConvertedState() {
        if (mPlaybackState == null)
            return "IDLE";
        String state;
        switch (mPlaybackState.getState()) {
            case PlaybackStateCompat.STATE_PLAYING:
                state = "PLAYING";
                break;
            case PlaybackStateCompat.STATE_PAUSED:
                state = "PAUSED";
                break;
            case PlaybackStateCompat.STATE_STOPPED:
                state = "STOPPED";
                break;
            case PlaybackStateCompat.STATE_FAST_FORWARDING:
                state = "FAST_FORWARDING";
                break;
            case PlaybackStateCompat.STATE_REWINDING:
                state = "REWINDING";
                break;
            case PlaybackStateCompat.STATE_BUFFERING:
                state = "BUFFER_UNDERRUN";
                break;
            default:
                state = "IDLE";
                break;
        }
        return state;
    }

    private long getOffset() {
        try {
            if (mMediaController == null || !mMediaController.isSessionReady()
                    || mMediaController.getPlaybackState() == null) {
                return 0;
            } else {
                return mMediaController.getPlaybackState().getPosition();
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            return 0;
        }
    }

    private long getLongMetadata(String key) {
        try {
            if (mMediaController == null || mMediaController.getMetadata() == null) {
                return 0;
            } else {
                return mMediaController.getMetadata().getLong(key);
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            return 0;
        }
    }

    private String getStringMetadata(String key) {
        try {
            if (mMediaController == null || mMediaController.getMetadata() == null) {
                Log.w(TAG, "Metadata is null");
                return "";
            } else {
                String value = mMediaController.getMetadata().getString(key);
                return value != null ? value : "";
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            return "";
        }
    }

    private void createMediaController(MediaSessionCompat.Token token) {
        if (mMediaController != null && !isBinderDied) {
            mMediaController.unregisterCallback(mCallback);
        }
        try {
            this.mMediaController = new MediaControllerCompat(mContext, token);
            Log.v(TAG, "mediaSessionStarted, created MediaController");
        } catch (Exception e) {
            Log.e(TAG, String.format("mediaSessionStarted %s", e.getMessage()));
            return;
        }
        this.mMediaController.registerCallback(mCallback);
        isBinderDied = false;
    }

    public void cleanup() {
        if (mMediaBrowserCompat != null && mMediaBrowserCompat.isConnected()) {
            mMediaBrowserCompat.disconnect();
            mMediaBrowserCompat = null;
        }
        if (mMediaController != null && !isBinderDied) {
            mMediaController.unregisterCallback(mCallback);
            mMediaController = null;
        }
    }
}
