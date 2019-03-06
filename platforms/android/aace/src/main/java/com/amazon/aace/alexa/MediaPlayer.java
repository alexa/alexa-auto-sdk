/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * MediaPlayer should be extended to play audio data provided by the Engine.
 * MediaPlayer is the interface for audio playback for an @c AudioChannel.
 * After returning @c true from a playback-controlling directive from the Engine (i.e. @c play(), @c pause(), @c stop(), @c resume()), the platform implementation should notify the Engine when the platform
 * media player changes playback state by calling @c mediaStateChanged() with the new @c MediaState. The Engine expects no call to @c mediaStateChanged() for a directive in which the platform
 * returned @c false.
 * The platform implementation may call @c mediaError() or @c mediaStateChanged() with @c MediaState.BUFFERING at any time during a playback operation to notify the Engine of an error or buffer underrun, respectvely.
 * When the media player resumes playback after a buffer underrun, the platform implementation
 * should call @c mediaStateChanged() with @c MediaState.PLAYING.
 *
 * @note The @c MediaPlayer platform implementation should be able to support the
 * audio formats recommended by AVS for a familiar Alexa experience:
 * https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html
 *
 * @sa AudioChannel
 */
abstract public class MediaPlayer extends PlatformInterface
{
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
        private MediaError( String name ) {
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
        private MediaState( String name ) {
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
     * Notifies the platform implementation to prepare for playback of an audio stream source.
     * Audio data will be available to stream from the Engine via @c read(). After returning @c true, the Engine will call
     * @c play() to initiate audio playback.
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean prepare() {
        return false;
    }

    /**
     * Notifies the platform implementation to prepare for playback of a
     * URL audio source. After returning @c true, the Engine will call @c play() to initiate audio playback.
     *
     * @param  url The URL audio source to set in the platform media player
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean prepare( String url ) {
        return false;
    }

    /**
     * Notifies the platform implementation to start playback of the current audio source. After returning @c true, the platform implementation must call @c mediaStateChanged() with @c MediaState.PLAYING when the media player begins playing the audio or @c mediaError() if an error occurs.
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean play() {
        return false;
    }

    /**
     * Notifies the platform implementation to stop playback of the current audio source. After returning @c true, the platform implementation must call @c mediaStateChanged() with @c MediaState.STOPPED when the media player stops playing the audio or @c mediaError() if an error occurs.
     * A subsequent call to @c play() will be preceded by calls to @c prepare()
     * and @c setPosition().
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean stop() {
        return false;
    }

    /**
     * Notifies the platform implementation to pause playback of the current audio source. After returning @c true, the platform implementation must call @c mediaStateChanged() with @c MediaState.STOPPED when the media player pauses the audio or @c mediaError() if an error occurs.
     * A subsequent call to @c resume() will not be preceded by calls to @c prepare()
     * and @c setPosition().
     *
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    public boolean pause() {
        return false;
    }

    /**
     * Notifies the platform implementation to resume playback of the current audio source. After returning @c true, the platform implementation must call @c mediaStateChanged() with @c MediaState.PLAYING when the media player resumes the audio or @c mediaError() if an error occurs.
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
     * @return The platform media player's playback position in milliseconds
     */
    public long getPosition() {
        return 0;
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
    public boolean setPosition( long position ) {
        return false;
    }

    /**
     * @return @c true if the audio stream is closed and no more data is available to read.
     */
    public boolean isClosed() {
        return isClosed( getNativeObject() );
    }

    /**
     * Checks if playback of the current audio source must be repeated when finished playing.
     * If this call returns @c true, the platform implementation should repeat the current audio source. If @c false is returned, the platform implementation should call @c mediaStateChanged() with @c MediaState.STOPPED when the
     * audio is finished playing.
     * Note that when @c stop has been called, audio source should stop repeating regardless of this value.
     *
     * @return @c true if the platform media player must repeat playback of the current audio source,
     * else @c false
     */
    public boolean isRepeating() {
        return isRepeating( getNativeObject() );
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
    final protected void mediaError( MediaError type, String error ) {
        mediaError( getNativeObject(), type, error);
    }

    /**
     * Notifies the Engine of an audio playback state change in the platform implementation.
     * Must be called when the platform media player transitions between stopped and playing states.
     *
     * @param  state The new playback state of the platform media player
     *
     * @sa MediaState
     */
    final protected void mediaStateChanged( MediaState state ) {
        mediaStateChanged( getNativeObject(), state );
    }

    /**
     * Reads audio data from the Engine when available. Audio data will be available after a call to @c prepare() and while @c isClosed() returns false.
     *
     * @param  data The buffer where audio data should be copied
     *
     * @param  size The size of the buffer
     *
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    final protected int read( byte[] buffer ) {
        return read( getNativeObject(), buffer, 0, buffer.length );
    }

    /**
     * Reads audio data from the Engine when available. Audio data will be available after a call to @c prepare() and while @c isClosed() returns false.
     *
     * @param  data The buffer where audio data should be copied
     *
     * @param  size The size of the buffer
     *
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    final protected int read( byte[] data, int offset, int size ) {
        return read( getNativeObject(), data, offset, size );
    }

    private native boolean isClosed( long nativeObject );
    private native boolean isRepeating( long nativeObject );
    private native void mediaError( long nativeObject, MediaError type, String error );
    private native void mediaStateChanged( long nativeObject, MediaState state );
    private native int read( long nativeObject, byte[] data, long offset, long size );
}

// END OF FILE
