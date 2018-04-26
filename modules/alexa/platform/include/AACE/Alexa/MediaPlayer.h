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

#ifndef AACE_ALEXA_MEDIA_PLAYER_H
#define AACE_ALEXA_MEDIA_PLAYER_H

#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c MediaPlayer class is the base class for platform media players, and should be extended to handle them.
 */
class MediaPlayer {
protected:
    MediaPlayer() = default;

public:
    using ErrorType = aace::alexa::MediaPlayerEngineInterface::ErrorType;

    virtual ~MediaPlayer() = default;

    /**
     * Called when the platform implementation should prepare for audio playback.
     *
     * Audio data will be available to read at this point and can be cached
     * locally for the platform media player.
     *
     * @return @c true if the call was handled successfully.
     */
    virtual bool prepare() = 0;

    /**
     * Called when the platform implementation should prepare to play back an
     * audio stream URL.
     *
     * @param [in] url Audio stream URL.
     * @return @c true if the call was handled successfully.
     */
    virtual bool prepare( const std::string& url ) = 0;

    /**
     * Called when the platform implementation should start playing audio.
     *
     * @return @c true if the call was handled successfully.
     */
    virtual bool play() = 0;

    /**
     * Called when the platform implementation should stop playing audio.
     *
     * @return @c true if the call was handled successfully,
     * else @c false.
     */
    virtual bool stop() = 0;

    /**
     * Called when the platform implementation should pause the currently playing audio.
     *
     * @return @c true if the call was handled successfully.
     */
    virtual bool pause() = 0;

    /**
     * Called when the platform implementation should resume playing currently paused audio.
     *
     * @return @c true if the call was handled successfully.
     */
    virtual bool resume() = 0;

    /**
     * Called when the Engine needs the media playback position (in milliseconds) of the current audio stream.
     *
     * Must return the current position.
     * @return Playback position (in milliseconds) of the current audio stream.
     */
    virtual int64_t getPosition() = 0;

    /**
     * Called when the Engine needs to set the media playback position (in milliseconds) of the current audio stream.
     *
     * Must set the media player to the position.
     * @param [in] position Position (in milliseconds) to set the current audio stream to.
     *
     * @return @c true if the call was handled successfully.
     *
     */
    virtual bool setPosition( int64_t position ) = 0;

    /**
     * Returns @c true if the current media source should repeat when it is finished playing.
     *
     * The @c MediaPlayer is responsible for repeating the current audio if this call returns @c true. If
     * @c false is returned, the @c MediaPlayer should call @c playbackFinished() when the current audio is
     * done playing.
     *
     * @return @c true if the call was handled successfully.
     */
    bool isRepeating();

    /**
     * Notify the Engine that the audio has started playing.
     *
     * Must be called when the platform implementation's @c MediaPlayer starts playing.
     */
    void playbackStarted();

    /**
     * Notify the Engine that the audio has finished playing.
     *
     * Must be called when the platform implementation's MediaPlayer has finished playing.
     */
    void playbackFinished();

    /**
     * Notify the Engine that audio playback has been paused.
     *
     * Must be called when the platform implementation's MediaPlayer enters a paused state.
     */
    void playbackPaused();

    /**
     * Notify the Engine that audio playback has resumed after being paused.
     *
     * Must be called when the platform implementation's MediaPlayer resumes.
     */
    void playbackResumed();

    /**
     * Notify the Engine that audio playback has stopped.
     *
     * Must be called when the platform implementation's MediaPlayer enters a stopped state.
     */
    void playbackStopped();

    /**
     * Notify the Engine that an error occurred while playing the audio.
     *
     * Must be called if the platform implementation runs into a playback error.
     * @param [in] type The error type.
     * @param [in] error The error description.
     * @sa ErrorType
     */
    void playbackError( const ErrorType& type, const std::string& error );

    /**
     * Read data from the audio stream for playback.
     *
     * @param [in] data The data buffer to read.
     * @param [in] size The size of the data to read.
     * @return The number of bytes read, or zero if end of stream is reached, or -1 if an error occurred.
     */
    ssize_t read( char* data, const size_t size );

    /**
     * @internal
     * Sets engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::MediaPlayerEngineInterface> mediaPlayerEngineInterface );

private:
    std::shared_ptr<aace::alexa::MediaPlayerEngineInterface> m_mediaPlayerEngineInterface;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_MEDIA_PLAYER_H
