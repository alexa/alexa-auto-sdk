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

package com.amazon.aace.audio;

import com.amazon.aace.core.NativeRef;

/**
 * AudioOutput should be extended to play audio data provided by the Engine.
 *
 * After returning @c true from a playback-controlling method invocation from the Engine (i.e. @c play(), @c pause(),
 * @c stop(), @c resume()), it is required that platform implementation notify the Engine of a playback state change by
 * calling one of @c mediaStateChanged() with the new @c MediaState or @c mediaError() with the @c MediaError. The
 * Engine expects no call to @c mediaStateChanged() in response to an invocation for which the platform returned
 * @c false.
 *
 * The platform implementation may call @c mediaError() or @c mediaStateChanged() with @c MediaState.BUFFERING
 * at any time during a playback operation to notify the Engine of an error or buffer underrun, respectvely.
 * When the media player resumes playback after a buffer underrun, the platform implementation should call
 * @c mediaStateChanged() with @c MediaState.PLAYING.
 *
 * @note The @c AudioOutput platform implementation should be able to support the
 * audio formats recommended by AVS for a familiar Alexa experience:
 * https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html
 */
abstract public class AudioOutput extends NativeRef {
    /**
     * Describes an error during a media playback operation
     */
    public enum MediaError {
        /**
         * An unknown error occurred.
         * @hideinitializer
         */
        MEDIA_ERROR_UNKNOWN("MEDIA_ERROR_UNKNOWN"),
        /**
         * The server recognized the request as malformed (e.g. bad request, unauthorized, forbidden, not found, etc).
         * @hideinitializer
         */
        MEDIA_ERROR_INVALID_REQUEST("MEDIA_ERROR_INVALID_REQUEST"),
        /**
         * The client was unable to reach the service.
         * @hideinitializer
         */
        MEDIA_ERROR_SERVICE_UNAVAILABLE("MEDIA_ERROR_SERVICE_UNAVAILABLE"),
        /**
         * The server accepted the request but was unable to process it as expected.
         * @hideinitializer
         */
        MEDIA_ERROR_INTERNAL_SERVER_ERROR("MEDIA_ERROR_INTERNAL_SERVER_ERROR"),
        /**
         * There was an internal error on the client.
         * @hideinitializer
         */
        MEDIA_ERROR_INTERNAL_DEVICE_ERROR("MEDIA_ERROR_INTERNAL_DEVICE_ERROR");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private MediaError(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes the playback state of the platform media player
     */
    public enum MediaState {
        /**
         * The media player is not currently playing. It may have paused, stopped, or finished.
         * @hideinitializer
         */
        STOPPED("STOPPED"),
        /**
         * The media player is currently playing.
         * @hideinitializer
         */
        PLAYING("PLAYING"),
        /**
         * The media player is currently buffering data.
         * @hideinitializer
         */
        BUFFERING("BUFFERING");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private MediaState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Describes the muted state of the audio output channel
     */
    public enum MutedState {
        /**
         * @hideinitializer
         */
        MUTED("MUTED"),
        /**
         * @hideinitializer
         */
        UNMUTED("UNMUTED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private MutedState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Used when audio time is unknown or indeterminate.
     */
    static public final long TIME_UNKNOWN = -1;

    /**
     * Notifies the platform implementation to prepare for playback of an audio stream source.
     * Audio data will be available to stream from the Engine via @c read(). After returning @c true, the Engine will
     * call
     * @c play() to initiate audio playback.
     *
     * @param  stream The audio stream to prepare
     * @param  repeating @c true if the audio should repeat
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean prepare(AudioStream stream, boolean repeating) {
        return false;
    }

    /**
     * Notifies the platform implementation to prepare for playback of a
     * URL audio source. After returning @c true, the Engine will call @c play() to initiate audio playback.
     *
     * @param  url The URL audio source to prepare
     * @param  repeating @c true if the audio should repeat
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean prepare(String url, boolean repeating) {
        return false;
    }

    /**
     * Notifies the platform implementation to start playback of the current audio source. After returning @c true,
     * the platform implementation must call @c mediaStateChanged() with @c MediaState.PLAYING when the media player
     * begins playing the audio or @c mediaError() if an error occurs.
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean play() {
        return false;
    }

    /**
     * Notifies the platform implementation to stop playback of the current audio source. After returning @c true,
     * the platform implementation must call @c mediaStateChanged() with @c MediaState.STOPPED when the media player
     * stops playing the audio or @c mediaError() if an error occurs.
     *
     * A subsequent call to @c play() will be preceded by calls to @c prepare() and @c setPosition().
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean stop() {
        return false;
    }

    /**
     * Notifies the platform implementation to pause playback of the current audio source. After returning @c true, the
     * platform implementation must call @c mediaStateChanged() with @c MediaState.STOPPED when the media player pauses
     * the audio or @c mediaError() if an error occurs. A subsequent call to @c resume() will not be preceded by calls
     * to @c prepare() and @c setPosition().
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean pause() {
        return false;
    }

    /**
     * Notifies the platform implementation to resume playback of the current audio source. After returning @c true, the
     * platform implementation must call @c mediaStateChanged() with @c MediaState.PLAYING when the media player resumes
     * the audio or @c mediaError() if an error occurs.
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean resume() {
        return false;
    }

    /**
     * Returns the current playback position of the platform media player.
     * If the audio source is not playing, the most recent position played
     * should be returned.
     *
     * @return The platform media player's playback position in milliseconds,
     * or @c TIME_UNKNOWN if the current media position is unknown or invalid.
     */
    public long getPosition() {
        return TIME_UNKNOWN;
    }

    /**
     * Notifies the platform implementation to set the playback position of the current audio source
     * in the platform media player
     *
     * @param  position The playback position in milliseconds to set in the platform media player
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean setPosition(long position) {
        return false;
    }

    /**
     * Returns the duration of the current audio source. If the duration is unknown, then
     * @c TIME_UNKNOWN should be returned.
     *
     * @return The duration of the current audio source in milliseconds, or @c TIME_UNKNOWN.
     */
    public long getDuration() {
        return TIME_UNKNOWN;
    }

    /**
     * Returns the amount of audio data buffered.
     *
     * @return the number of bytes of the audio data buffered, or 0 if it's unknown.
     */
    public long getNumBytesBuffered() {
        return 0;
    }

    /**
     * Notifies the platform implementation to set the volume of the output channel. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param [in] volume The volume to set on the output channel. @c volume
     * is in the range [0,1].
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean volumeChanged(float volume) {
        return false;
    }

    /**
     * Notifies the platform implementation to apply a muted state has changed for
     * the output channel
     *
     * @param [in] state The muted state to apply to the output channel. @c MutedState::MUTED when
     * the output channel be muted, @c MutedState::UNMUTED when unmuted
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean mutedStateChanged(MutedState state) {
        return false;
    }

    /**
     * Notifies the Engine of an error during audio playback
     *
     * @param  error The error encountered by the platform media player during playback
     *
     * @param  description A description of the error
     *
     * @sa MediaError
     */
    protected void mediaError(MediaError type, String error) {
        mediaError(getNativeRef(), type, error);
        if (mMediaStateListener != null) {
            mMediaStateListener.mediaError(type, error);
        }
    }

    /**
     * Notifies the Engine of an audio playback state change in the platform implementation.
     * Must be called when the platform media player transitions between stopped and playing states.
     *
     * @param  state The new playback state of the platform media player
     *
     * @sa MediaState
     */
    protected void mediaStateChanged(MediaState state) {
        mediaStateChanged(getNativeRef(), state);
        if (mMediaStateListener != null) {
            mMediaStateListener.mediaStateChanged(state);
        }
    }

    protected long createNativeRef() {
        return createBinder();
    }

    protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void mediaError(long nativeObject, MediaError type, String error);
    private native void mediaStateChanged(long nativeObject, MediaState state);

    // MediaStateListener

    public interface MediaStateListener {
        void mediaError(MediaError type, String error);
        void mediaStateChanged(MediaState state);
    }

    public void setMediaStateListener(MediaStateListener listener) {
        mMediaStateListener = listener;
    }

    private MediaStateListener mMediaStateListener;
}
