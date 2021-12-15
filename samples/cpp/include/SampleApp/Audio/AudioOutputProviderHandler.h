/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_AUDIO_AUDIOOUTPUTPROVIDERHANDLER_H
#define SAMPLEAPP_AUDIO_AUDIOOUTPUTPROVIDERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/ApplicationContext.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Audio/AudioOutput/MediaErrorMessage.h>
#include <AASB/Message/Audio/AudioOutput/MediaStateChangedMessage.h>
#include <AASB/Message/Audio/AudioOutput/MutedStateChangedMessage.h>
#include <AASB/Message/Audio/AudioOutput/PrepareStreamMessage.h>

#include <memory>
#include <mutex>
#include <chrono>

namespace sampleApp {
namespace audio {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioOutputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class DefaultAudioOutput;

class AudioOutputProviderHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;
    float m_volume;

protected:
    AudioOutputProviderHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker,
        bool setup = true);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AudioOutputProviderHandler> {
        return std::shared_ptr<AudioOutputProviderHandler>(new AudioOutputProviderHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;
    auto setupUI() -> void;

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto writeStreamToFile(std::shared_ptr<aace::core::MessageStream> stream) -> void;

private:
    auto subscribeToAASBMessages() -> void;

    void mayDuck();
    void startDucking(const std::string& channel, const std::string& token);
    void stopDucking(const std::string& channel, const std::string& token);

    /**
     * Handles MutedStateChangedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleMutedStateChangedMessage(const std::string& message);

    /**
     * Handles PauseMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePauseMessage(const std::string& message);

    /**
     * Handles PlayMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePlayMessage(const std::string& message);

    /**
     * Handles PrepareStreamMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePrepareStreamMessage(const std::string& message);

    /**
     * Handles PrepareURLMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePrepareURLMessage(const std::string& message);

    /**
     * Handles MayDuckMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleMayDuckMessage(const std::string& message);

    /**
     * Handles ResumeMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleResumeMessage(const std::string& message);

    /**
     * Handles SetPositionMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSetPositionMessage(const std::string& message);

    /**
     * Handles StopMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleStopMessage(const std::string& message);

    /**
     * Handles VolumeChangedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleVolumeChangedMessage(const std::string& message);

    /**
    * Handles StartDuckingMessage received from the Engine.
    *
    * @param [in] message The message received from the Engine
    */
    void handleStartDuckingMessage(const std::string& message);

    /**
    * Handles StopDuckingMessage received from the Engine.
    *
    * @param [in] message The message received from the Engine
    */
    void handleStopDuckingMessage(const std::string& message);
    /**
     * Handles GetDurationMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetDurationMessage(const std::string& message);

    /**
     * Handles GetNumBytesBufferedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetNumBytesBufferedMessage(const std::string& message);

    /**
     * Handles GetPositionMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetPositionMessage(const std::string& message);

    /**
     * Provides the implementation to prepare an audio stream for playback.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] audioType The type of audio data to be played
     * @param [in] stream The opened message stream
     */
    void prepareStream(
        const std::string& channel,
        aasb::message::audio::audioOutput::AudioOutputAudioType audioType,
        std::shared_ptr<aace::core::MessageStream> stream);

    /**
     * Provides the implementation to prepare an audio URL for playback.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] audioType The type of audio data to be played
     * @param [in] url The URL audio stream being provided
     * @param [in] repeating True if the platform should loop the audio when playing
     */
    void prepareURL(
        const std::string& channel,
        aasb::message::audio::audioOutput::AudioOutputAudioType audioType,
        const std::string& url,
        bool repeating);

    /**
     * Provides the implementation to play an audio source.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] token A unique token for this audio source.
     */
    void play(const std::string& channel, const std::string& token);

    /**
     * Provides the implementation to stop an audio source.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] token A unique token for this audio source.
     */
    void stop(const std::string& channel, const std::string& token);

    /**
     * Provides the implementation to pause an audio source.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] token A unique token for this audio source.
     */
    void pause(const std::string& channel, const std::string& token);

    /**
     * Provides the implementation to resume an audio source.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] token A unique token for this audio source.
     */
    void resume(const std::string& channel, const std::string& token);

    /**
     * Provides the implementation for getting the current playback position of the audio source.
     */
    int64_t getPosition();

    /**
     * Provides the implementation to set the playback position of the current audio source.
     * 
     * @param [in] position The playback position in milliseconds to set in the platform media player.
     */
    void setPosition(int64_t position);

    /**
     * Provides the implementation to get the duration of the current audio source.
     */
    int64_t getDuration();

    /**
     * Provides the implementation that the volume has changed for an audio source.
     * 
     * @param [in] volume The new volume.
     */
    void volumeChanged(float volume);

    /**
     * Provides the implementation that the muted state has changed for an audio source.
     * 
     * @param [in] state The muted state to apply to the audio source.
     */
    void mutedStateChanged(aasb::message::audio::audioOutput::MutedState state);

    /**
     * Provides the implementation for getting the amount of audio data buffered.
     */
    int64_t getNumBytesBuffered();

    /**
     * Notifies the Engine of an error during audio playback.
     * 
     * @param [in] token A unique token for this audio source.
     * @param [in] error The error encountered by the platform media player during playback.
     * @param [in] description Optional. A description of the error.
     */
    void mediaError(
        const std::string& token,
        aasb::message::audio::audioOutput::MediaError error,
        const std::string& description);

    /**
     * Notifies the Engine of an audio playback state change.
     * 
     * @param [in] channel Name of the channel that is providing audio.
     * @param [in] token A unique token for this audio source.
     * @param [in] state The new playback state of the platform media player.
     */
    void mediaStateChanged(
        const std::string& channel,
        const std::string& token,
        aasb::message::audio::audioOutput::MediaState state);

    void setNameAndType(const std::string& name, aasb::message::audio::audioOutput::AudioOutputAudioType type);

private:
    std::weak_ptr<View> m_console{};
    std::weak_ptr<View> m_alertStateView{};
    std::weak_ptr<ApplicationContext> m_applicationContext{};

    std::chrono::milliseconds m_minPlayDuration;
    std::chrono::milliseconds m_maxPlayDuration;
    std::string m_name;
    Executor m_executer;
    int64_t m_position;
    bool m_playing;
    bool m_paused;
    std::condition_variable m_cv;
    std::mutex m_mutex;
};

}  // namespace audio
}  // namespace sampleApp

#endif  // SAMPLEAPP_AUDIO_AUDIOOUTPUTPROVIDERHANDLER_H
