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

#ifndef AACE_ALEXA_MEDIA_PLAYER_H
#define AACE_ALEXA_MEDIA_PLAYER_H

#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * MediaPlayer should be extended to play audio data provided by the Engine.
 * MediaPlayer is the interface for audio playback for an @c AudioChannel.
 *
 * After returning @c true from a playback-controlling directive from the Engine (i.e. @c play(), @c pause(), @c stop(),
 * @c resume()), the platform implementation should notify the Engine when the platform
 * media player changes playback state by calling @c mediaStateChanged() with the new @c MediaState.
 * The Engine expects no call to @c mediaStateChanged() for a directive in which the platform
 * returned @c false. 
 *
 * The platform implementation may call @c mediaError() or @c mediaStateChanged() with @c MediaState.BUFFERING
 * at any time during a playback operation to notify the Engine of an error or buffer underrun, respectvely.
 * When the media player resumes playback after a buffer underrun, the platform implementation
 * should call @c mediaStateChanged() with @c MediaState.PLAYING.
 *
 * @note The @c MediaPlayer platform implementation should be able to support the 
 * audio formats recommended by AVS for a familiar Alexa experience:
 * https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html
 *
 * @sa AudioChannel
 */  
class MediaPlayer {
protected:
    MediaPlayer() = default;

public:
    /**
     * Describes the playback state of the platform media player
     * @sa @c aace::alexa::MediaPlayerEngineInterface::MediaState
     */
    using MediaState = aace::alexa::MediaPlayerEngineInterface::MediaState;

    /**
     * Describes an error during a media playback operation
     * @sa @c aace::alexa::MediaPlayerEngineInterface::MediaError
     */
    using MediaError = aace::alexa::MediaPlayerEngineInterface::MediaError;

    virtual ~MediaPlayer() = default;

    /**
     * Notifies the platform implementation to prepare for playback of an audio stream source.
     * Audio data will be available to stream from the Engine via @c read(). After returning @c true, the Engine will call
     * @c play() to initiate audio playback.
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool prepare() = 0;

    /**
     * Notifies the platform implementation to prepare for playback of a
     * URL audio source. After returning @c true, the Engine will call @c play()
     * to initiate audio playback.
     *
     * @param [in] url The URL audio source to set in the platform media player
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool prepare( const std::string& url ) = 0;

    /**
     * Notifies the platform implementation to start playback of the current audio source. After returning @c true,
     * the platform implementation must call @c mediaStateChanged() with @c MediaState.PLAYING
     * when the media player begins playing the audio or @c mediaError() if an error occurs.
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool play() = 0;

    /**
     * Notifies the platform implementation to stop playback of the current audio source. After returning @c true,
     * the platform implementation must call @c mediaStateChanged() with @c MediaState.STOPPED
     * when the media player stops playing the audio or immediately if it is already stopped, or @c mediaError() if an error occurs.
     * A subsequent call to @c play() will be preceded by calls to @c prepare() 
     * and @c setPosition().
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool stop() = 0;

    /**
     * Notifies the platform implementation to pause playback of the current audio source. After returning @c true,
     * the platform implementation must call @c mediaStateChanged() with @c MediaState.STOPPED
     * when the media player pauses the audio or @c mediaError() if an error occurs.
     * A subsequent call to @c resume() will not be preceded by calls to @c prepare() 
     * and @c setPosition().
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool pause() = 0;

    /**
     * Notifies the platform implementation to resume playback of the current audio source. After returning @c true,
     * the platform implementation must call @c mediaStateChanged() with @c MediaState.PLAYING
     * when the media player resumes the audio or @c mediaError() if an error occurs.
     *
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool resume() = 0;

    /**
     * Returns the current playback position of the platform media player.
     * If the audio source is not playing, the most recent position played
     * should be returned.
     *
     * @return The platform media player's playback position in milliseconds
     */
    virtual int64_t getPosition() = 0;

    /**
     * Notifies the platform implementation to set the playback position of the current audio source
     * in the platform media player
     *
     * @param [in] position The playback position in milliseconds to set in the platform media player
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool setPosition( int64_t position ) = 0;

    /**
     * Checks if playback of the current audio source must be repeated when finished playing.
     * If this call returns @c true, the platform implementation should repeat the current audio source. If @c false
     * is returned, the platform implementation should call @c mediaStateChanged() with @c MediaState.STOPPED when the
     * audio is finished playing.
     * Note that when @c stop has been called, audio source should stop repeating regardless of this value.
     *
     * @return @c true if the platform media player must repeat playback of the current audio source,
     * else @c false
     */
    bool isRepeating();
    
    /**
     * Notifies the Engine of an audio playback state change in the platform implementation.
     * Must be called when the platform media player transitions between stopped and playing states.
     *
     * @param [in] state The new playback state of the platform media player
     * @sa MediaState
     */
    void mediaStateChanged( MediaState state );
    
    /**
     * Notifies the Engine of an error during audio playback
     *
     * @param [in] error The error encountered by the platform media player during playback
     * @param [in] description A description of the error
     * @sa MediaError
     */
    void mediaError( MediaError error, const std::string& description = "" );

    /**
     * Reads audio data from the Engine when available. Audio data will be available after a call to @c prepare()
     * and while @c isClosed() returns false.
     *
     * @param [out] data The buffer where audio data should be copied
     * @param [in] size The size of the buffer
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    ssize_t read( char* data, const size_t size );

    /**
     * Checks if the current audio stream from the Engine has no more data available to read
     *
     * @return @c true if the audio stream is closed, @c false if more data
     * will be available
     */
    bool isClosed();

    /**
     * @internal
     * Sets the Engine interface delegate.
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
