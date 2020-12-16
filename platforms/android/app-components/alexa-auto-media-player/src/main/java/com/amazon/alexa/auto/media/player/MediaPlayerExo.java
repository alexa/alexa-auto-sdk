package com.amazon.alexa.auto.media.player;

import static com.google.android.exoplayer2.C.TIME_UNSET;

import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;

import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.Timeline;
import com.google.android.exoplayer2.source.MediaSource;

/**
 * A thin wrapper over Exo Player.
 */
public class MediaPlayerExo {
    private static String TAG = MediaPlayerExo.class.getSimpleName();

    @NonNull
    private final SimpleExoPlayer mMainPlayer;
    @NonNull
    private final MediaSourceFactory mMediaSourceFactory;

    /**
     * Construct the instance of @c MediaPlayerExo
     *
     * @param exoPlayer ExoPlayer controlled by this object.
     * @param sourceFactory Media source factory.
     */
    public MediaPlayerExo(@NonNull SimpleExoPlayer exoPlayer, @NonNull MediaSourceFactory sourceFactory) {
        this.mMainPlayer = exoPlayer;
        this.mMediaSourceFactory = sourceFactory;
    }

    /**
     * Returns the player managed by this wrapper object.
     *
     * @return Exo Player.
     */
    @NonNull
    public SimpleExoPlayer getPlayer() {
        return mMainPlayer;
    }

    /**
     * Return the current playback state.
     *
     * @return Current playback state.
     */
    @Player.State
    public int getPlaybackState() {
        return mMainPlayer.getPlaybackState();
    }

    /**
     * Tells if Player is ready to play after source is available or already
     * playing as source is available.
     *
     * @return true if Player is ready to play.
     */
    public boolean getPlayWhenReady() {
        return mMainPlayer.getPlayWhenReady();
    }

    /**
     * Tells if current playback state is stopped.
     *
     * @return true if current playback state is stopped.
     */
    public boolean isStopped() {
        return getMediaState().isPlaybackStopped();
    }

    public MediaState getMediaState() {
        return new MediaState(getPlayWhenReady(), getPlaybackState());
    }

    /**
     * Prepares the Media Player for playing the given source.
     *
     * @param uri {@link Uri} of the source.
     */
    public void prepare(Uri uri) throws Exception {
        Log.d(TAG, "prepare with " + uri);

        MediaSource mediaSource = mMediaSourceFactory.createHttpMediaSource(uri);
        Log.d(TAG, "mediaSource " + mediaSource.toString());
        mMainPlayer.prepare(mediaSource);
    }

    /**
     * Request Media Player to pause the playback.
     */
    public void requestPause() {
        if (mMainPlayer.getPlayWhenReady()) {
            mMainPlayer.setPlayWhenReady(false);
        }
    }

    /**
     * Request Media Player to resume the playback.
     */
    public void requestResume() {
        if (!mMainPlayer.getPlayWhenReady()) {
            mMainPlayer.setPlayWhenReady(true);
        }
    }

    /**
     * Request Media Player to stop the playback.
     */
    public void requestStop() {
        mMainPlayer.stop();
        mMainPlayer.setPlayWhenReady(false);
    }

    /**
     * Request Media Player to start the Playback.
     */
    public void requestPlay() {
        requestResume();
    }

    /**
     * Set the volume level for the playback.
     *
     * @param volumeMultiplier Volume multiplier from 0 to 1.
     */
    public void setVolume(float volumeMultiplier) {
        mMainPlayer.setVolume(volumeMultiplier);
    }

    /**
     * Gets current position of the player.
     *
     * @return Current position in milliseconds.
     */
    public long getPosition() {
        long positionMs = mMainPlayer.getCurrentPosition();
        if (mMainPlayer.isCurrentWindowDynamic()) {
            int currentWindowIndex = mMainPlayer.getCurrentWindowIndex();
            int currentPeriodIndex = mMainPlayer.getCurrentPeriodIndex();

            if (currentWindowIndex >= 0 && currentPeriodIndex >= 0) {
                Timeline timeline = mMainPlayer.getCurrentTimeline();
                Timeline.Window currentWindow = new Timeline.Window();
                Timeline.Period currentPeriod = new Timeline.Period();
                timeline.getWindow(currentWindowIndex, currentWindow);
                timeline.getPeriod(currentPeriodIndex, currentPeriod);

                long defaultPositionMs = currentWindow.getDefaultPositionMs();
                long periodPositionInWindowMs = currentPeriod.getPositionInWindowMs();
                Log.d(TAG,
                        "Dynamic window. Adjusting position. Current Position:" + positionMs + " Default Position: "
                                + defaultPositionMs + " Period Position in Window " + periodPositionInWindowMs);

                if (defaultPositionMs != TIME_UNSET && periodPositionInWindowMs != TIME_UNSET) {
                    positionMs = (positionMs - periodPositionInWindowMs) - defaultPositionMs;
                    if (positionMs < 0) {
                        positionMs = 0;
                    }
                }
            }
        }

        return positionMs;
    }

    /**
     * Gets duration of current playback.
     *
     * @return Duration of current playback in milliseconds.
     */
    public long getDuration() {
        return mMainPlayer.getDuration();
    }

    /**
     * Seek to given position.
     *
     * @param positionMs Position to seek to in milliseconds.
     */
    public void seekToPosition(long positionMs) {
        mMainPlayer.seekTo(positionMs);
    }

    /**
     * Enable/Disable shuffle mode on media player.
     *
     * @param shuffleModeEnabled Whether to enable shuffle mode.
     */
    public void setShuffleModeEnabled(boolean shuffleModeEnabled) {
        mMainPlayer.setShuffleModeEnabled(shuffleModeEnabled);
    }

    /**
     * Set Repeat Mode on media player.
     *
     * @param repeatMode Desired repeat mode to set on player.
     */
    public void setRepeatMode(@Player.RepeatMode int repeatMode) {
        mMainPlayer.setRepeatMode(repeatMode);
    }
}
