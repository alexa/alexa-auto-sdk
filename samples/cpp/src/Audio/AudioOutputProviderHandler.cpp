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

#include "SampleApp/Audio/AudioOutputProviderHandler.h"

#include <AASB/Message/Audio/AudioOutput/GetDurationMessage.h>
#include <AASB/Message/Audio/AudioOutput/GetNumBytesBufferedMessage.h>
#include <AASB/Message/Audio/AudioOutput/GetPositionMessage.h>
#include <AASB/Message/Audio/AudioOutput/MayDuckMessage.h>
#include <AASB/Message/Audio/AudioOutput/PauseMessage.h>
#include <AASB/Message/Audio/AudioOutput/PlayMessage.h>
#include <AASB/Message/Audio/AudioOutput/PrepareURLMessage.h>
#include <AASB/Message/Audio/AudioOutput/ResumeMessage.h>
#include <AASB/Message/Audio/AudioOutput/SetPositionMessage.h>
#include <AASB/Message/Audio/AudioOutput/StartDuckingMessage.h>
#include <AASB/Message/Audio/AudioOutput/StopDuckingMessage.h>
#include <AASB/Message/Audio/AudioOutput/StopMessage.h>
#include <AASB/Message/Audio/AudioOutput/VolumeChangedMessage.h>

// C++ Standard Library
#include <sstream>
#include <array>
#include <random>
#include <fstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace audio {

using namespace aasb::message::audio::audioOutput;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioOutputProviderHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Used when audio time is unknown or indeterminate.
 */
static const int64_t TIME_UNKNOWN = -1;
static const float DUCKING_COEFFICIENT = 0.2f;

using MessageBroker = aace::core::MessageBroker;
using MessageStream = aace::core::MessageStream;

AudioOutputProviderHandler::AudioOutputProviderHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker,
    bool setup) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)} {
    if (setup) {
        setupUI();
    }
    subscribeToAASBMessages();
}

std::weak_ptr<Activity> AudioOutputProviderHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AudioOutputProviderHandler::getLoggerHandler() {
    return m_loggerHandler;
}

void AudioOutputProviderHandler::setNameAndType(const std::string& name, AudioOutputAudioType type) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    m_applicationContext = activity->getApplicationContext();
    m_name = name;
    if (type == AudioOutputAudioType::TTS) {
        m_minPlayDuration = std::chrono::seconds(1);
        m_maxPlayDuration = std::chrono::seconds(10);
    } else if (type == AudioOutputAudioType::MUSIC) {
        m_minPlayDuration = std::chrono::seconds(1);
        m_maxPlayDuration = std::chrono::seconds(30);
    } else {
        m_minPlayDuration = std::chrono::milliseconds(1);
        m_maxPlayDuration = std::chrono::milliseconds(1);
    }
}

void AudioOutputProviderHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to MutedStateChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleMutedStateChangedMessage(message); },
        MutedStateChangedMessage::topic(),
        MutedStateChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Pause Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePauseMessage(message); },
        PauseMessage::topic(),
        PauseMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Play Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePlayMessage(message); }, PlayMessage::topic(), PlayMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to PrepareStream Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePrepareStreamMessage(message); },
        PrepareStreamMessage::topic(),
        PrepareStreamMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to PrepareURL Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePrepareURLMessage(message); },
        PrepareURLMessage::topic(),
        PrepareURLMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to MayDuck Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleMayDuckMessage(message); },
        MayDuckMessage::topic(),
        MayDuckMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Resume Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleResumeMessage(message); },
        ResumeMessage::topic(),
        ResumeMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to SetPosition Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleSetPositionMessage(message); },
        SetPositionMessage::topic(),
        SetPositionMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to Stop Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStopMessage(message); }, StopMessage::topic(), StopMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to VolumeChanged Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleVolumeChangedMessage(message); },
        VolumeChangedMessage::topic(),
        VolumeChangedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to StartDucking Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStartDuckingMessage(message); },
        StartDuckingMessage::topic(),
        StartDuckingMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to StopDucking Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleStopDuckingMessage(message); },
        StopDuckingMessage::topic(),
        StopDuckingMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetDuration Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetDurationMessage(message); },
        GetDurationMessage::topic(),
        GetDurationMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetNumBytesBuffered Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetNumBytesBufferedMessage(message); },
        GetNumBytesBufferedMessage::topic(),
        GetNumBytesBufferedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetPosition Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetPositionMessage(message); },
        GetPositionMessage::topic(),
        GetPositionMessage::action());
}

void AudioOutputProviderHandler::handleMutedStateChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received MutedStateChangedMessage");

    MutedStateChangedMessage msg = json::parse(message);
    mutedStateChanged(msg.payload.state);
}

void AudioOutputProviderHandler::handlePauseMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PauseMessage");

    PauseMessage msg = json::parse(message);
    pause(msg.payload.channel, msg.payload.token);
}

void AudioOutputProviderHandler::handlePlayMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PlayMessage");

    PlayMessage msg = json::parse(message);
    play(msg.payload.channel, msg.payload.token);
}

void AudioOutputProviderHandler::handlePrepareStreamMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PrepareStreamMessage");

    PrepareStreamMessage msg = json::parse(message);
    auto stream = m_messageBroker->openStream(msg.payload.streamId, MessageStream::Mode::READ);
    prepareStream(msg.payload.channel, msg.payload.audioType, stream);
}

void AudioOutputProviderHandler::handlePrepareURLMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PrepareURLMessage");

    PrepareURLMessage msg = json::parse(message);
    prepareURL(msg.payload.channel, msg.payload.audioType, msg.payload.url, msg.payload.repeating);
}

void AudioOutputProviderHandler::handleMayDuckMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received MayDuckMessage");
    mayDuck();
}

void AudioOutputProviderHandler::handleResumeMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received ResumeMessage");

    ResumeMessage msg = json::parse(message);
    resume(msg.payload.channel, msg.payload.token);
}

void AudioOutputProviderHandler::handleSetPositionMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received SetPositionMessage");

    SetPositionMessage msg = json::parse(message);
    setPosition(msg.payload.position);
}

void AudioOutputProviderHandler::handleStopMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StopMessage");

    StopMessage msg = json::parse(message);
    stop(msg.payload.channel, msg.payload.token);
}

void AudioOutputProviderHandler::handleVolumeChangedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received VolumeChangedMessage");

    VolumeChangedMessage msg = json::parse(message);
    volumeChanged(msg.payload.volume);
}

void AudioOutputProviderHandler::handleStartDuckingMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StartDuckingMessage");

    StartDuckingMessage msg = json::parse(message);
    startDucking(msg.payload.channel, msg.payload.token);
}

void AudioOutputProviderHandler::handleStopDuckingMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received StopDuckingMessage");

    StopDuckingMessage msg = json::parse(message);
    stopDucking(msg.payload.channel, msg.payload.token);
}

void AudioOutputProviderHandler::handleGetDurationMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetDurationMessage");
    GetDurationMessage msg = json::parse(message);

    // Publish the reply message for getDuration.
    GetDurationMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.duration = getDuration();

    m_messageBroker->publish(replyMsg.toString());
}

void AudioOutputProviderHandler::handleGetNumBytesBufferedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetNumBytesBufferedMessage");
    GetNumBytesBufferedMessage msg = json::parse(message);

    // Publish the reply message for getNumBytesBuffered.
    GetNumBytesBufferedMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.bufferedBytes = getNumBytesBuffered();

    m_messageBroker->publish(replyMsg.toString());
}

void AudioOutputProviderHandler::handleGetPositionMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetPositionMessage");
    GetPositionMessage msg = json::parse(message);

    // Publish the reply message for getPosition.
    GetPositionMessageReply replyMsg;
    replyMsg.header.messageDescription.replyToId = msg.header.id;
    replyMsg.payload.position = getPosition();

    m_messageBroker->publish(replyMsg.toString());
}

void AudioOutputProviderHandler::mediaError(
    const std::string& token,
    MediaError error,
    const std::string& description) {
    MediaErrorMessage msg;
    msg.payload.token = token;
    msg.payload.error = error;
    msg.payload.description = description;

    m_messageBroker->publish(msg.toString());
}

void AudioOutputProviderHandler::mediaStateChanged(
    const std::string& channel,
    const std::string& token,
    MediaState state) {
    MediaStateChangedMessage msg;
    msg.payload.channel = channel;
    msg.payload.token = token;
    msg.payload.state = state;

    m_messageBroker->publish(msg.toString());
}

void AudioOutputProviderHandler::prepareStream(
    const std::string& channel,
    AudioOutputAudioType audioType,
    std::shared_ptr<MessageStream> stream) {
    log(logger::LoggerHandler::Level::INFO, "setStreamsetStream");
    setNameAndType(channel, audioType);
    writeStreamToFile(stream);
}

void AudioOutputProviderHandler::prepareURL(
    const std::string& channel,
    AudioOutputAudioType audioType,
    const std::string& url,
    bool repeating) {
    std::stringstream ss;
    ss << "prepare: " << url << " repeating: " << repeating;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    setNameAndType(channel, audioType);
}

void AudioOutputProviderHandler::mayDuck() {
    log(logger::LoggerHandler::Level::INFO, "stream may duck");
}

void AudioOutputProviderHandler::play(const std::string& channel, const std::string& token) {
    log(logger::LoggerHandler::Level::INFO, "play");
    m_playing = true;
    m_paused = false;
    mediaStateChanged(channel, token, MediaState::PLAYING);
    m_executer.submit([=]() {
        std::random_device seeder;
        std::mt19937 engine(seeder());
        std::uniform_int_distribution<int> dist(m_minPlayDuration.count(), m_maxPlayDuration.count());
        int sleepTime = 100;
        int reportInterval = 1000;
        auto playTime = dist(engine);
        auto lastReportPosition = 0;
        while (m_position < playTime && m_playing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [=] { return !m_paused; });
            m_position += sleepTime;
            if (lastReportPosition == 0 || (m_position - lastReportPosition) > reportInterval) {
                std::stringstream ss;

                ss << "Position: "
                   << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(m_position)).count()
                   << "s Remaining: "
                   << std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::milliseconds((playTime - m_position)))
                          .count()
                   << "s";

                log(logger::LoggerHandler::Level::VERBOSE, ss.str());
                lastReportPosition = m_position;
            }
        }
        log(logger::LoggerHandler::Level::INFO, "playback finished");
        mediaStateChanged(channel, token, MediaState::STOPPED);
    });
}

void AudioOutputProviderHandler::stop(const std::string& channel, const std::string& token) {
    log(logger::LoggerHandler::Level::INFO, "stop");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_playing = false;
    m_paused = false;
    mediaStateChanged(channel, token, MediaState::STOPPED);
    m_cv.notify_one();
}

void AudioOutputProviderHandler::pause(const std::string& channel, const std::string& token) {
    log(logger::LoggerHandler::Level::INFO, "pause");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = true;
    mediaStateChanged(channel, token, MediaState::STOPPED);
    m_cv.notify_one();
}

void AudioOutputProviderHandler::resume(const std::string& channel, const std::string& token) {
    log(logger::LoggerHandler::Level::INFO, "resume");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_paused = false;
    mediaStateChanged(channel, token, MediaState::PLAYING);
    m_cv.notify_one();
}

void AudioOutputProviderHandler::startDucking(const std::string& channel, const std::string& token) {
    std::stringstream ss;
    ss << "startDucking channel: " << channel << " volumeChanged: " << DUCKING_COEFFICIENT * m_volume;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void AudioOutputProviderHandler::stopDucking(const std::string& channel, const std::string& token) {
    std::stringstream ss;
    ss << "stopDucking channel: " << channel << " volumeChanged: " << m_volume;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

int64_t AudioOutputProviderHandler::getPosition() {
    log(logger::LoggerHandler::Level::INFO, "getPosition");
    return m_position;
}

void AudioOutputProviderHandler::setPosition(int64_t position) {
    std::stringstream ss;
    ss << "setPosition: " << position;
    log(logger::LoggerHandler::Level::INFO, ss.str());
    m_position = position;
}

int64_t AudioOutputProviderHandler::getNumBytesBuffered() {
    return 0;
}

int64_t AudioOutputProviderHandler::getDuration() {
    log(logger::LoggerHandler::Level::INFO, "getDuration");
    return TIME_UNKNOWN;
}

void AudioOutputProviderHandler::volumeChanged(float volume) {
    std::stringstream ss;
    m_volume = volume;
    ss << "volumeChanged: " << volume;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void AudioOutputProviderHandler::mutedStateChanged(MutedState state) {
    std::stringstream ss;
    ss << "mutedStateChanged: ";
    switch (state) {
        case MutedState::MUTED:
            ss << "MUTED";
            break;
        case MutedState::UNMUTED:
            ss << "UNMUTED";
            break;
        default:
            log(logger::LoggerHandler::Level::ERROR, "Invalid MutedState value");
            return;
    }
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void AudioOutputProviderHandler::writeStreamToFile(std::shared_ptr<MessageStream> stream) {
    auto applicationContext = m_applicationContext.lock();
    if (!applicationContext) {
        return;
    }

    auto path = applicationContext->makeTempPath(m_name, "mp3");

    std::stringstream ss;
    ss << "writeStreamToFile: " << path;
    log(logger::LoggerHandler::Level::INFO, ss.str());

    if (std::remove(path.c_str()) == 0) {
        log(logger::LoggerHandler::Level::INFO, "File successfully deleted: " + path);
    }

    auto output = std::make_shared<std::ofstream>(path, std::ios::binary | std::ofstream::out | std::ofstream::app);
    if (!output->good()) {
        log(logger::LoggerHandler::Level::INFO, "Could not create cache file: " + path);
        return;
    }
    char buffer[4096];
    ssize_t bytes = 0;
    ssize_t count;
    while (!stream->isClosed()) {
        count = stream->read(buffer, 4096);
        if (count > 0) {
            bytes += count;
            output->write(buffer, count);
        }
    }
    output->close();

    ss.clear();
    ss << "Cache file complete: " << path << " size: " << bytes;
    log(logger::LoggerHandler::Level::INFO, ss.str());
}

void AudioOutputProviderHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AudioOutputProviderHandler", message);
}

void AudioOutputProviderHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");
}

}  // namespace audio
}  // namespace sampleApp
