/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/alexa/MediaPlayer.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c MediaPlayer class is the base class for platform media players, and should be extended to handle them.
 */
abstract public class MediaPlayer extends PlatformInterface
{
    /**
     * Type used to identify an error that can occur during media operations.
     */
    public enum ErrorType
    {
        /**
         * An unknown error occurred.
         * @hideinitializer
         */
        MEDIA_ERROR_UNKNOWN("MEDIA_ERROR_UNKNOWN"),
        /**
         * The server recognized the request as being malformed (bad request, unauthorized, forbidden, not found, etc).
         * @hideinitializer
         */
        MEDIA_ERROR_INVALID_REQUEST("MEDIA_ERROR_INVALID_REQUEST"),
        /**
         * The client was unable to reach the service.
         * @hideinitializer
         */
        MEDIA_ERROR_SERVICE_UNAVAILABLE("MEDIA_ERROR_SERVICE_UNAVAILABLE"),
        /**
         * The server accepted the request, but was unable to process the request as expected.
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
        private ErrorType( String name ) {
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
     * Called when the platform implementation should prepare for audio playback.
     * Audio data will be available to read at this point and can be cached locally for the platform media player.
     *
     * @return @c true if the call was handled successfully.
     */
    public boolean prepare() {
        return false;
    }

    /**
     * Called when the platform implementation should prepare to play back an audio stream URL.
     *
     * @param [in] url Audio stream URL.
     * @return @c true if the call was handled successfully.
     */
    public boolean prepare( String url ) {
        return false;
    }

    /**
     * Called when the platform implementation should start playing audio.
     *
     * @return @c true if the call was handled successfully.
     */
    public boolean play() {
        return false;
    }

    /**
     * Called when the platform implementation should stop playing audio.
     *
     * @return @c true if the call was handled successfully, else @c false.
     */
    public boolean stop() {
        return false;
    }

    /**
     * Called when the platform implementation should pause the currently playing audio.
     *
     * @return @c true if the call was handled successfully.
     */
    public boolean pause() {
        return false;
    }

    /**
     * Called when the platform implementation should resume playing currently paused audio.
     *
     * @return @c true if the call was handled successfully.
     */
    public boolean resume() {
        return false;
    }

    /**
     * Called when the Engine needs the media playback position (in milliseconds) of the current audio stream.
     * Must return the current position.
     * @return Playback position (in milliseconds) of the current audio stream.
     */
    public long getPosition() {
        return 0;
    }

    /**
     * Called when the Engine needs to set the media playback position (in milliseconds) of the current audio stream.
     * Must set the media player to the position.
     * @param [in] position Position (in milliseconds) to set the current audio stream to.
     *
     * @return @c true if the call was handled successfully.
     */
    public boolean setPosition( long position ) {
        return false;
    }

    /**
     * Returns @c true if the current media source should repeat when it is finished playing.
     * The @c MediaPlayer is responsible for repeating the current audio if this call returns @c true. If @c false is returned, the @c MediaPlayer should call @c playbackFinished() when the current audio is done playing.
     *
     * @return @c true if the call was handled successfully.
     */
    public boolean isRepeating() {
        return isRepeating( getNativeObject() );
    }

    /**
     * Notify the Engine that the audio has started playing.
     * Must be called when the platform implementation's @c MediaPlayer starts playing.
     */
    final protected void playbackStarted() {
        playbackStarted( getNativeObject() );
    }

    /**
     * Notify the Engine that the audio has finished playing.
     * Must be called when the platform implementation's MediaPlayer has finished playing.
     */
    final protected void playbackFinished() {
        playbackFinished( getNativeObject() );
    }

    /**
     * Notify the Engine that audio playback has been paused.
     * Must be called when the platform implementation's MediaPlayer enters a paused state.
     */
    final protected void playbackPaused() {
        playbackPaused( getNativeObject() );
    }

    /**
     * Notify the Engine that audio playback has resumed after being paused.
     * Must be called when the platform implementation's MediaPlayer resumes.
     */
    final protected void playbackResumed() {
        playbackResumed( getNativeObject() );
    }

    /**
     * Notify the Engine that audio playback has stopped.
     * Must be called when the platform implementation's MediaPlayer enters a stopped state.
     */
    final protected void playbackStopped() {
        playbackStopped( getNativeObject() );
    }

    /**
     * Notify the Engine that an error occurred while playing the audio.
     * Must be called if the platform implementation runs into a playback error.
     * @param [in] type The error type.
     * @param [in] error The error description.
     * @sa ErrorType
     */
    final protected void playbackError( ErrorType type, String error ) {
        playbackError( getNativeObject(), type, error );
    }

    /**
     * Read data from the audio stream for playback.
     *
     * @param [in] data The data buffer to read.
     * @param [in] size The size of the data to read.
     * @return The number of bytes read, or zero if end of stream is reached, or -1 if an error occurred.
     */
    final protected int read( byte[] buffer ) {
        return read( getNativeObject(), buffer, 0, buffer.length );
    }

    final protected int read( byte[] data, int offset, int size ) {
        return read( getNativeObject(), data, offset, size );
    }

    private native boolean isRepeating( long nativeObject );
    private native void playbackStarted( long nativeObject );
    private native void playbackFinished( long nativeObject );
    private native void playbackPaused( long nativeObject );
    private native void playbackResumed( long nativeObject );
    private native void playbackStopped( long nativeObject );
    private native void playbackError( long nativeObject, ErrorType type, String error );
    private native int read( long nativeObject, byte[] data, int offset, int size );
}

// END OF FILE
