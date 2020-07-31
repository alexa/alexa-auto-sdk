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

#ifndef AACE_AUDIO_AUDIO_OUTPUT_H
#define AACE_AUDIO_AUDIO_OUTPUT_H

#include <memory>

#include "AudioEngineInterfaces.h"
#include "AudioStream.h"

/** @file */

namespace aace {
namespace audio {

/**
 * AudioOutput should be extended to play audio data provided by the Engine.
 *
 * After returning @c true from a playback-controlling method invocation from the Engine (i.e. @c play(), @c pause(),
 * @c stop(), @c resume()), it is required that platform implementation notify the Engine of a playback state change by
 * calling one of @c mediaStateChanged() with the new @c MediaState or @c mediaError() with the @c MediaError. The
 * Engine expects no call to @c mediaStateChanged() in response to an invocation for which the platform returned
 * @c false.
 *
 * The platform implementation may call @c mediaError() or @c mediaStateChanged() with @c MediaState::BUFFERING
 * at any time during a playback operation to notify the Engine of an error or buffer underrun, respectvely.
 * When the media player resumes playback after a buffer underrun, the platform implementation should call
 * @c mediaStateChanged() with @c MediaState::PLAYING.
 *
 * @note The @c AudioOutput platform implementation should be able to support the
 * audio formats recommended by AVS for a familiar Alexa experience:
 * https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html
 */
class AudioOutput {
protected:
    AudioOutput() = default;

public:
    /**
     * Describes the playback state of the platform media player
     * @sa @c aace::alexa::MediaPlayerEngineInterface::MediaState
     */
    using MediaState = aace::audio::AudioOutputEngineInterface::MediaState;

    /**
     * Describes an error during a media playback operation
     * @sa @c aace::alexa::MediaPlayerEngineInterface::MediaError
     */
    using MediaError = aace::audio::AudioOutputEngineInterface::MediaError;

    /**
     * Used when audio time is unknown or indeterminate.
     */
    static const int64_t TIME_UNKNOWN = -1;

    enum class MutedState {
        /**
         * The audio channel state id muted.
         */
        MUTED,

        /**
         * The audio channel state id unmuted.
         */
        UNMUTED
    };

    virtual ~AudioOutput();

    /**
     * Notifies the platform implementation to prepare for playback of an
     * @c AudioStream audio source. After returning @c true, the Engine will call @c play()
     * to initiate audio playback.
     *
     * @param [in] stream The @c AudioStream object that provides the platform implementation
     * audio data to play.
     * @param [in] repeating @c true if the platform should loop the audio when playing.
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool prepare(std::shared_ptr<AudioStream> stream, bool repeating) = 0;

    /**
     * Notifies the platform implementation to prepare for playback of a
     * URL audio source. After returning @c true, the Engine will call @c play()
     * to initiate audio playback.
     *
     * @param [in] url The URL audio source to set in the platform media player
     * @param [in] repeating @c true if the platform should loop the audio when playing.
     * @return @c true if the platform implementation successfully handled the call,
     * else @c false
     */
    virtual bool prepare(const std::string& url, bool repeating) = 0;

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
     * @return The platform media player's playback position in milliseconds, 
     * or @c TIME_UNKNOWN if the current media position is unknown or invalid.
     */
    virtual int64_t getPosition() = 0;

    /**
     * Notifies the platform implementation to set the playback position of the current audio source
     * in the platform media player.
     *
     * @param [in] position The playback position in milliseconds to set in the platform media player
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool setPosition(int64_t position) = 0;

    /**
     * Returns the duration of the current audio source. If the duration is unknown, then
     * @c TIME_UNKNOWN should be returned.
     *
     * @return The duration of the current audio source in milliseconds, or @c TIME_UNKNOWN.
     */
    virtual int64_t getDuration() = 0;

    /**
     * Returns the amount of audio data buffered.
     *
     * @return the number of bytes of the audio data buffered, or 0 if it's unknown.
     */
    virtual int64_t getNumBytesBuffered();

    /**
     * Notifies the platform implementation to set the volume of the output channel. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param [in] volume The volume to set on the output channel. @c volume
     * is in the range [0,1].
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool volumeChanged(float volume) = 0;

    /**
     * Notifies the platform implementation to apply a muted state has changed for
     * the output channel
     *
     * @param [in] state The muted state to apply to the output channel. @c MutedState::MUTED when
     * the output channel be muted, @c MutedState::UNMUTED when unmuted
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool mutedStateChanged(MutedState state) = 0;

    /**
     * Notifies the Engine of an audio playback state change in the platform implementation.
     * Must be called when the platform media player transitions between stopped and playing states.
     *
     * @param [in] state The new playback state of the platform media player
     * @sa MediaState
     */
    void mediaStateChanged(MediaState state);

    /**
     * Notifies the Engine of an error during audio playback
     *
     * @param [in] error The error encountered by the platform media player during playback
     * @param [in] description A description of the error
     * @sa MediaError
     */
    void mediaError(MediaError error, const std::string& description = "");

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::audio::AudioOutputEngineInterface> audioOutputEngineInterface);

private:
    std::weak_ptr<aace::audio::AudioOutputEngineInterface> m_audioOutputEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const AudioOutput::MutedState& state) {
    switch (state) {
        case AudioOutput::MutedState::MUTED:
            stream << "MUTED";
            break;
        case AudioOutput::MutedState::UNMUTED:
            stream << "UNMUTED";
            break;
    }
    return stream;
}

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_OUTPUT_H
