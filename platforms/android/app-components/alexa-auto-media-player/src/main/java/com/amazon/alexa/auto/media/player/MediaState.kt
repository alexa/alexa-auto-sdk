package com.amazon.alexa.auto.media.player

import android.support.v4.media.session.PlaybackStateCompat
import com.amazon.aacsconstants.MediaConstants
import com.google.android.exoplayer2.Player

/**
 * Media State object to notify clients about new state.
 */
data class MediaState(
    val playWhenReady: Boolean,
    val playbackState: Int) {

    /**
     * Maps the MediaState to AACS Audio Output playback state.
     *
     * @return State mapped to AACS Audio output playback state.
     */
    fun toAACSMediaState() : String {
        when (playbackState) {
            Player.STATE_IDLE, Player.STATE_ENDED -> return MediaConstants.MediaState.STOPPED
            Player.STATE_BUFFERING -> return MediaConstants.MediaState.BUFFERING
            Player.STATE_READY -> return if (playWhenReady) {
                MediaConstants.MediaState.PLAYING
            } else {
                MediaConstants.MediaState.STOPPED
            }
        }
        return MediaConstants.MediaState.STOPPED
    }

    fun toMediaSessionState() : Int {
        return when (playbackState) {
            Player.STATE_BUFFERING ->
                if (playWhenReady)
                    PlaybackStateCompat.STATE_BUFFERING
                else
                    PlaybackStateCompat.STATE_PAUSED
            Player.STATE_READY ->
                if (playWhenReady)
                    PlaybackStateCompat.STATE_PLAYING
                else
                    PlaybackStateCompat.STATE_PAUSED
            Player.STATE_ENDED -> PlaybackStateCompat.STATE_PAUSED
            Player.STATE_IDLE -> PlaybackStateCompat.STATE_PAUSED
            else -> PlaybackStateCompat.STATE_PAUSED
        }
    }

    /**
     * Tells if Playback is stopped and can't be resumed from same
     * position without seeking.
     */
    fun isPlaybackStopped() : Boolean {
        when(playbackState) {
            Player.STATE_IDLE, Player.STATE_ENDED -> return !playWhenReady
            else -> return false
        }
    }
}
