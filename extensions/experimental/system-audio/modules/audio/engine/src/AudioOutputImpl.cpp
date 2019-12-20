/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/SystemAudio/AudioOutputImpl.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <unistd.h>

namespace aace {
namespace engine {
namespace systemAudio {

#define LXT LX(TAG).d("name", m_name)

// String to identify log entries originating from this file.
static const std::string TAG("aace.systemAudio.AudioOutputImpl");

static constexpr size_t READ_BUFFER_SIZE = 4096;
static constexpr std::chrono::milliseconds NEXT_READ_PERIOD(100);

std::ostream& operator<<(std::ostream& stream, AudioOutputImpl::State state) {
    switch (state) {
        case AudioOutputImpl::State::Created:
            stream << "Created";
            break;
        case AudioOutputImpl::State::Initialized:
            stream << "Initialized";
            break;
        case AudioOutputImpl::State::Preparing:
            stream << "Preparing";
            break;
        case AudioOutputImpl::State::Prepared:
            stream << "Prepared";
            break;
        case AudioOutputImpl::State::Starting:
            stream << "Starting";
            break;
        case AudioOutputImpl::State::Started:
            stream << "Started";
            break;
        case AudioOutputImpl::State::Pausing:
            stream << "Pausing";
            break;
        case AudioOutputImpl::State::Paused:
            stream << "Paused";
            break;
        case AudioOutputImpl::State::Resuming:
            stream << "Resuming";
            break;
        case AudioOutputImpl::State::Stopping:
            stream << "Stopping";
            break;
        case AudioOutputImpl::State::Stopped:
            stream << "Stopped";
            break;
        case AudioOutputImpl::State::Faulted:
            stream << "Faulted";
            break;
    }
    return stream;
}

bool AudioOutputImpl::checkState(std::initializer_list<AudioOutputImpl::State> validStates) {
    return std::find(validStates.begin(), validStates.end(), m_state) != validStates.end();
}

void AudioOutputImpl::setState(State state) {
    AACE_INFO(LXT.d("old", m_state).d("new", state));
    m_state = state;
}

// clang-format off
static aal_listener_t aalListener = {
    .on_start = [](void* user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<AudioOutputImpl*>(user_data);
        self->onStart();
    },
    .on_stop = [](aal_status_t reason, void* user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<AudioOutputImpl*>(user_data);
        self->onStop(reason);
    },
    .on_almost_done = nullptr,
    .on_data = nullptr,
    .on_data_requested = [](void* user_data) {
        ReturnIf(!user_data);
        auto self = static_cast<AudioOutputImpl*>(user_data);
        self->onDataRequested();
    }
};
// clang-format on

AudioOutputImpl::AudioOutputImpl(const int moduleId, const std::string& deviceName, const std::string& name) :
    m_moduleId(moduleId), m_name(name), m_deviceName(deviceName) {
}

AudioOutputImpl::~AudioOutputImpl() {
    m_executor.submit([this] {
        executeStopStreaming();
        if (m_player) {
            aal_player_destroy(m_player);
        }
        if (!m_tmpFile.empty()) {
            std::remove(m_tmpFile.c_str());
        }
    });
    m_executor.waitForSubmittedTasks();
}

std::unique_ptr<AudioOutputImpl> AudioOutputImpl::create(
    const int moduleId, const std::string& deviceName, const std::string& name) {
    try {
        auto audioOutput = std::unique_ptr<AudioOutputImpl>(new AudioOutputImpl(moduleId, deviceName, name));

        ThrowIfNot(audioOutput->initialize(), "initializeFailed");

        return audioOutput;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AudioOutputImpl::initialize() {
    AACE_VERBOSE(LXT);
    setState(State::Initialized);
    return true;
}

bool AudioOutputImpl::writeStreamToFile(aace::audio::AudioStream* stream, const std::string& path) {
    try {
        AACE_VERBOSE(LXT.d("path", path));

        ThrowIfNull(stream, "invalidAudioStream");

        // create the output file
        auto output = std::make_shared<std::ofstream>(path, std::ios::binary | std::ofstream::out | std::ofstream::app);
        ThrowIfNot(output->good(), "createOutputFileFailed");

        // copy the stream to the file
        char buffer[READ_BUFFER_SIZE];
        ssize_t bytesRead;
        ssize_t size = 0;

        while (!stream->isClosed()) {
            bytesRead = stream->read(buffer, READ_BUFFER_SIZE);

            // throw an error if the read failed
            ThrowIf(bytesRead < 0, "readFromStreamFailed");

            // write the data to the output file
            output->write(buffer, bytesRead);

            size += bytesRead;
        }

        output->close();

        AACE_VERBOSE(LXT.m("complete").d("size", size));

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return false;
    }
}

bool AudioOutputImpl::writeStreamToPipeline() {
    try {
        AACE_VERBOSE(LXT);

        ThrowIfNull(m_currentStream, "invalidAudioStream");

        char buffer[READ_BUFFER_SIZE];
        auto size = m_currentStream->read(buffer, READ_BUFFER_SIZE);

        // throw an error if the read failed
        ThrowIf(size < 0, "readFromStreamFailed");

        if (size > 0) {
            // write the data to the player's pipeline
            ThrowIf(aal_player_write(m_player, buffer, size) < 0, "writeToPipelineFailed");
        } else {
            if (m_currentStream->isClosed()) {
                aal_player_notify_end_of_stream(m_player);
                return false;
            } else {
                // if we didn't read any data and the stream is not closed, then
                // sleep some mount of time before next read
                std::this_thread::sleep_for(NEXT_READ_PERIOD);
            }
        }

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));

        // on an error we want to abort the operation so tell the player
        // not to attempt to write anymore data...
        aal_player_notify_end_of_stream(m_player);
        return false;
    }
}

void AudioOutputImpl::onStart() {
    m_executor.submit([this] { executeOnStart(); });
}

void AudioOutputImpl::executeOnStart() {
    AACE_VERBOSE(LXT);
    if (!checkState({
            State::Started,   // TODO: post-playback started
            State::Starting,  // the first starting
            State::Resuming,  // resuming
            State::Stopping,  // immediate stop after start?!
        })) {
        AACE_WARN(LXT.d("reason", "suspicious state change").d("state", m_state));
    }
    setState(State::Started);
    mediaStateChanged(MediaState::PLAYING);
}

void AudioOutputImpl::onStop(aal_status_t reason) {
    m_executor.submit([this, reason] { executeOnStop(reason); });
}

void AudioOutputImpl::executeOnStop(aal_status_t reason) {
    AACE_VERBOSE(LXT.d("reason", reason));
    if (!checkState({
            State::Stopping,  // intentional stop
            State::Pausing,   // pause
            State::Started,   // completed
            State::Starting,  // AAL failed to start
            State::Faulted,   // Fault during playback
        })) {
        AACE_WARN(LXT.d("reason", "suspicious state change").d("state", m_state));
    }
    try {
        executeStopStreaming();
        if (reason == AAL_ERROR) {
            setState(State::Faulted);
            mediaError(MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR);
        } else {
            if (reason == AAL_SUCCESS && m_repeating) {
                // play it again without changing state
                aal_player_play(m_player);
            } else {
                if (m_state == State::Pausing) {
                    setState(State::Paused);
                } else {
                    setState(State::Stopped);
                }
                mediaStateChanged(MediaState::STOPPED);
            }
        }
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
    }
}

void AudioOutputImpl::streamingLoop() {
    do {
        if (!writeStreamToPipeline()) break;
    } while (m_streaming);
}

void AudioOutputImpl::executeStartStreaming() {
    if (m_streamingThread.joinable()) {
        // Streaming has already been started, let's ignore this request.
        return;
    }
    // Start streaming thread
    m_streaming = true;
    m_streamingThread = std::thread(&AudioOutputImpl::streamingLoop, this);
}

void AudioOutputImpl::executeStopStreaming() {
    m_streaming = false;
    if (m_streamingThread.joinable()) {
        m_streamingThread.join();
    }
}

void AudioOutputImpl::onDataRequested() {
    m_executor.submit([this]() { executeStartStreaming(); });
}

//
// aace::audio::AudioOutput
//

bool AudioOutputImpl::prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) {
    return m_executor.submit([this, stream, repeating] { return executePrepare(stream, repeating); }).get();
}

bool AudioOutputImpl::executePrepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) {
    if (!checkState({
                        State::Initialized,  // new one
                        State::Stopped,      // completed
                        State::Faulted,      // failed last time
                    })) {
        AACE_WARN(LXT.d("reason", "suspicious state change").d("state", m_state));
    }

    bool succeeded;
    try {
        AACE_VERBOSE(LXT.d("encoding", stream->getEncoding()).d("repeating", repeating));

        switch (stream->getEncoding()) {
            case audio::AudioStream::Encoding::UNKNOWN:
                // Note: We assume the unknown streams are all MP3 formatted
            case audio::AudioStream::Encoding::MP3: {
                // write the audio stream to a temp file
                char tmpFile[] = "/tmp/aac_audio_XXXXXX";
                int fd = mkstemp(tmpFile);
                if (fd < 0) {
                    AACE_ERROR(LX(TAG).m("mkstemp failed"));
                    succeeded = false;
                    break;
                }
                close(fd);
                ThrowIfNot(writeStreamToFile(stream.get(), tmpFile), "writeStreamToFileFailed");

                // remove the old temp file
                if (!m_tmpFile.empty() && std::remove(m_tmpFile.c_str()) == 0) {
                    AACE_INFO(LXT.m("tempFileRemoved").d("path", m_tmpFile));
                }
                m_tmpFile = tmpFile;
                succeeded = prepareLocked("file://" + m_tmpFile, repeating);
                break;
            }
            case audio::AudioStream::Encoding::LPCM:
                m_currentStream = stream;
                succeeded = prepareLocked("", repeating);
                break;
            default:
                Throw("unsupportedStreamEncoding");
        }
    } catch (std::exception& ex) {
        AACE_WARN(LXT.d("reason", ex.what()));
        succeeded = false;
    }

    setState(succeeded ? State::Prepared : State::Faulted);
    return succeeded;
}

bool AudioOutputImpl::prepare(const std::string& url, bool repeating) {
    return m_executor.submit([this, url, repeating]() { return executePrepare(url, repeating); }).get();
}

bool AudioOutputImpl::executePrepare(const std::string& url, bool repeating) {
    if (!checkState({
            State::Initialized,  // new one
            State::Stopped,      // completed
            State::Faulted,      // failed last time
        })) {
        AACE_WARN(LXT.d("reason", "suspicious state change").d("state", m_state));
    }

    bool succeeded = prepareLocked(url, repeating);
    setState(succeeded ? State::Prepared : State::Faulted);
    return succeeded;
}

bool AudioOutputImpl::prepareLocked(const std::string& url, bool repeating) {
    bool succeeded;
    try {
        AACE_INFO(LXT.d("url", url).d("repeating", repeating));

        if (m_player) {
            aal_player_destroy(m_player);
            m_player = nullptr;
        }

        // clang-format off
        const aal_attributes_t attr = {
            .name = m_name.c_str(),
            .device = m_deviceName.c_str(),
            .rate = 0,
            .uri = url.c_str(),
            .listener = &aalListener,
            .user_data = this,
            .module_id = m_moduleId
        };
        // clang-format on

        m_player = aal_player_create(&attr);
        ThrowIfNull(m_player, "createPlayerFailed");

        if (url.empty()) {
            /* Source is LPCM streaming */
            aal_player_set_stream_type(m_player, AAL_STREAM_LPCM);
        }

        // Apply the values to the new player
        executeVolumeChanged(m_currentVolume);
        executeMutedStateChanged(m_currentMutedState);

        m_repeating = repeating;

        succeeded = true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        succeeded = false;
    }

    return succeeded;
}

bool AudioOutputImpl::play() {
    return m_executor.submit([this] { return executePlay(); }).get();
}

bool AudioOutputImpl::executePlay() {
    try {
        AACE_VERBOSE(LXT);
        ThrowIfNull(m_player, "playerIsNULL");
        if (!checkState({
                State::Prepared,  // first-time play
                State::Stopping,  // repeated play
                State::Stopped,   // play another
            })) {
            AACE_ERROR(LXT.d("reason", "invalid state").d("state", m_state));
            return false;
        }
        setState(State::Starting);
        aal_player_play(m_player);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

bool AudioOutputImpl::stop() {
    return m_executor.submit([this] { return executeStop(); }).get();
}

bool AudioOutputImpl::executeStop() {
    try {
        AACE_VERBOSE(LXT);
        ThrowIfNull(m_player, "playerIsNULL");
        if (!checkState({State::Starting, State::Started, State::Paused})) {
            AACE_ERROR(LXT.d("reason", "invalid state").d("state", m_state));
            return false;
        }
        setState(State::Stopping);
        aal_player_stop(m_player);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

bool AudioOutputImpl::pause() {
    return m_executor.submit([this] { return executePause(); }).get();
}

bool AudioOutputImpl::executePause() {
    try {
        AACE_VERBOSE(LXT);
        ThrowIfNull(m_player, "playerIsNULL");
        if (m_state != State::Started) {
            AACE_ERROR(LXT.d("reason", "invalid state").d("state", m_state));
            return false;
        }
        setState(State::Pausing);
        aal_player_pause(m_player);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

bool AudioOutputImpl::resume() {
    return m_executor.submit([this] { return executeResume(); }).get();
}

bool AudioOutputImpl::executeResume() {
    try {
        AACE_VERBOSE(LXT);
        ThrowIfNull(m_player, "playerIsNULL");
        if (m_state != State::Paused) {
            AACE_ERROR(LXT.d("reason", "invalid state").d("state", m_state));
            return false;
        }
        setState(State::Resuming);
        aal_player_play(m_player);
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

int64_t AudioOutputImpl::getPosition() {
    return m_executor.submit([this] { return executeGetPosition(); }).get();
}

int64_t AudioOutputImpl::executeGetPosition() {
    try {
        AACE_VERBOSE(LXT);
        ThrowIfNull(m_player, "playerIsNULL");
        int64_t pos = aal_player_get_position(m_player);
        // Note: We update position only when it is non-zero
        if (pos != 0) {
            m_currentPosition = pos;
        }
        return m_currentPosition;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return (int64_t)0;
    }
}

bool AudioOutputImpl::setPosition(int64_t position) {
    return m_executor.submit([this, position] { return executeSetPosition(position); }).get();
}

bool AudioOutputImpl::executeSetPosition(int64_t position) {
    try {
        AACE_VERBOSE(LXT.d("position", position));
        ThrowIfNull(m_player, "playerIsNULL");
        aal_player_seek(m_player, position);
        // We save the value for later use
        m_currentPosition = position;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

int64_t AudioOutputImpl::getDuration() {
    return m_executor.submit([this] { return executeGetDuration(); }).get();
}

int64_t AudioOutputImpl::executeGetDuration() {
    try {
        AACE_VERBOSE(LXT);
        ThrowIfNull(m_player, "playerIsNULL");
        int64_t duration = aal_player_get_duration(m_player);
        return duration > -1 ? duration : TIME_UNKNOWN;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        return TIME_UNKNOWN;
    }
}

bool AudioOutputImpl::volumeChanged(float volume) {
    return m_executor.submit([this, volume] { return executeVolumeChanged(volume); }).get();
}

bool AudioOutputImpl::executeVolumeChanged(float volume) {
    try {
        AACE_VERBOSE(LXT.d("volume", volume));
        // Set volume only when AAL handle is available, ignore when NULL
        if (m_player) {
            aal_player_set_volume(m_player, volume);
        }
        // We save the value for later use
        m_currentVolume = volume;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

bool AudioOutputImpl::mutedStateChanged(MutedState state) {
    return m_executor.submit([this, state] { return executeMutedStateChanged(state); }).get();
}

bool AudioOutputImpl::executeMutedStateChanged(MutedState state) {
    try {
        AACE_VERBOSE(LXT.d("state", state));
        // Set mute only when AAL handle is available, ignore when NULL
        if (m_player) {
            aal_player_set_mute(m_player, state == MutedState::MUTED);
        }
        // We save the value for later use
        m_currentMutedState = state;
        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LXT.d("reason", ex.what()));
        setState(State::Faulted);
        return false;
    }
}

}  // namespace systemAudio
}  // namespace engine
}  // namespace aace
