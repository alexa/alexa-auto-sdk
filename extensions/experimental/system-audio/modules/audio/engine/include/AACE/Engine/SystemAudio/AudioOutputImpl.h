/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_SYSTEMAUDIO_AUDIO_OUTPUT_IMPL_H
#define AACE_ENGINE_SYSTEMAUDIO_AUDIO_OUTPUT_IMPL_H

#include <AACE/Audio/AudioOutput.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <aal.h>
#include <vector>
#include <deque>

namespace aace {
namespace engine {
namespace systemAudio {

class AudioOutputImpl : public aace::audio::AudioOutput {
public:
    ~AudioOutputImpl() override;

    // Factory
    static std::unique_ptr<AudioOutputImpl> create(
        int moduleId,
        const std::string& deviceName,
        const std::string& name = "");

    // AAL callbacks
    void onStart();
    void onStop(aal_status_t reason);
    void onDataRequested();
    void onAlmostDone();

    // aace::audio::AudioOutput
    bool prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) override;
    bool prepare(const std::string& url, bool repeating) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition(int64_t position) override;
    int64_t getDuration() override;
    int64_t getNumBytesBuffered() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;

private:
    AudioOutputImpl(int moduleId, std::string deviceName, std::string name);
    bool initialize();
    bool writeStreamToFile(aace::audio::AudioStream* stream, const std::string& path);
    bool writeStreamToPipeline();
    void streamingLoop();

    void executeOnStart();
    void executeOnStop(aal_status_t reason);
    void executeStartStreaming();
    void executeStopStreaming();

    bool executePrepare(const std::shared_ptr<aace::audio::AudioStream>& stream, bool repeating);
    bool executePrepare(const std::string& url, bool repeating);
    bool prepareLocked(const std::string& url, const std::shared_ptr<aace::audio::AudioStream>& stream, bool repeating);
    void preparePlayer(const std::string& url, const std::shared_ptr<aace::audio::AudioStream>& stream);
    bool executePlay();
    bool executeStop();
    bool executePause();
    bool executeResume();
    int64_t executeGetPosition();
    bool executeSetPosition(int64_t position);
    int64_t executeGetDuration();
    int64_t executeGetNumBytesBuffered();
    bool executeVolumeChanged(float volume);
    bool executeMutedStateChanged(MutedState state);

    std::vector<std::string> parsePlaylistUrl(const std::string& url);

    // States ending with 'ing' are intermediate states that are not allowed outside an execute* method.
    // That means an execute* method should end with an *ed state by calling waitForState.
    enum class State {
        Created,
        Initialized,
        Preparing,
        Prepared,
        Starting,
        Started,
        Pausing,
        Paused,
        Resuming,
        Stopping,
        Stopped,
        Faulted
    };
    friend std::ostream& operator<<(std::ostream& stream, State state);

    bool checkState(std::initializer_list<State> validStates);
    bool checkState(State state);
    void setState(State state);
    void setStateLocked(State state);
    bool waitForState(State state);

    int m_moduleId;
    std::string m_name;
    aal_handle_t m_player = nullptr;
    std::shared_ptr<aace::audio::AudioStream> m_currentStream;
    std::string m_mediaUrl;
    std::deque<std::string> m_mediaQueue;
    bool m_repeating = false;
    std::atomic<int64_t> m_currentPosition{0};
    float m_currentVolume = 0.5;
    MutedState m_currentMutedState = MutedState::UNMUTED;
    std::string m_tmpFile;
    std::thread m_streamingThread;
    std::atomic<bool> m_streaming;
    std::string m_deviceName;

    State m_state;
    std::mutex m_stateMutex;
    std::condition_variable m_cvStateChange;

    // Executor to serialize AudioOutput operations
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    // Executor to serialize gstreamer callbacks
    alexaClientSDK::avsCommon::utils::threading::Executor m_executorCallback;
};

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_SYSTEMAUDIO_AUDIO_OUTPUT_IMPL_H
