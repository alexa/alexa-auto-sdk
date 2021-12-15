package com.amazon.alexa.auto.media;

import android.content.Context;
import android.media.AudioManager;
import android.util.Log;

import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.lang.ref.WeakReference;

/**
 * Helper class to update music stream attributes such as volume and mute
 * status.
 */
public class MusicStreamAttributeUpdater {
    private static final String TAG = MusicStreamAttributeUpdater.class.getSimpleName();

    private WeakReference<Context> mContextWk;

    /**
     * Constructs an instance of @c MusicStreamAttributeUpdater.
     *
     * @param contextWk Android Context.
     */
    public MusicStreamAttributeUpdater(WeakReference<Context> contextWk) {
        this.mContextWk = contextWk;
    }

    /**
     * Set volume of Android music/audio stream
     *
     * @param volume New volume level to be set
     */
    public void changeVolumeForMusicStream(double volume) {
        Log.d(TAG, String.format("changeVolumeForMusicStream %f", volume));

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        Preconditions.checkNotNull(audioManager);

        int currentVolume = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        int maxStreamVolume = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);

        /*
         * For Android devices, the stream volume can be adjusted between 0 and max stream volume.
         * Given that we receive volume value between 0 & 1 from Auto SDK, we can multiply that
         * with max stream volume to get the equivalent percentage of volume w.r.t max stream
         * value. For example, Nexus 9 has max stream volume as 15. When we receive 0.50 from
         * Auto SDK which means we want volume as 50%, this will be 0.5 * 15 which is 8(of 15)
         * after rounding off.
         */
        int volumeToSet = Math.round((float) volume * maxStreamVolume);
        Log.d(TAG,
                "current android volume for music stream is " + currentVolume + " maxStreamVolume " + maxStreamVolume
                        + " new volume " + volumeToSet);

        audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, volumeToSet, 0);
    }

    /**
     * Sets mute status on Music Stream.
     *
     * @param mute Whether to mute or unmute the music stream.
     */
    public void setMuteForMusicStream(boolean mute) {
        Log.d(TAG, "muting " + mute);
        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        AudioManager audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        Preconditions.checkNotNull(audioManager);

        if (audioManager.isVolumeFixed()) {
            audioManager.adjustSuggestedStreamVolume(
                    mute ? AudioManager.ADJUST_MUTE : AudioManager.ADJUST_UNMUTE, AudioManager.STREAM_MUSIC, 0);
        } else {
            Log.i(TAG, "Volume level is fixed on current Android Platform");
        }
    }
}
