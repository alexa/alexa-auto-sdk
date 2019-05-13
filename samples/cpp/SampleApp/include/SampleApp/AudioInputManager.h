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
 *
 */

#ifndef SAMPLEAPP_AUDIOINPUTMANAGER_H_
#define SAMPLEAPP_AUDIOINPUTMANAGER_H_

#include <AACE/Audio/AudioChannel.h>
#include <memory>
#include <unordered_map>

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

namespace sampleApp {

using MicrophoneDataHandler = std::function<ssize_t(const int16_t *, const size_t)>;

/**
 * @c AudioInputManager facilitates feeding microphone input to multiple
 * clients simultaneously (or to an individual client if there is only one client).
 *
 */
class AudioInputManager {
  public:
    AudioInputManager(std::weak_ptr<Activity> activity,
                      std::weak_ptr<logger::LoggerHandler> loggerHandler,
                      std::shared_ptr<aace::audio::AudioCapture> inputChannel);

    /**
     * Start streaming from the audio input channel
     *
     * @param listener The function to receive incoming samples
     * @return @c true if the call is succeeded, else @c false
     */
    bool startAudioInput(const std::string &name, const MicrophoneDataHandler &listener);

    /**
     * Stop streaming from the audio input channel
     *
     * @return @c true if the call is succeeded, else @c false
     */
    bool stopAudioInput(const std::string &name);

  private:
    auto notifyDataAvailable(const int16_t *data, const size_t size) -> bool;
    auto startAudioInput() -> bool;
    auto log(logger::LoggerHandler::Level level, const std::string &message) -> void;

    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

  private:
    std::shared_ptr<aace::audio::AudioCapture> m_inputChannel;
    std::unordered_map<std::string, MicrophoneDataHandler> m_observers;
    std::mutex m_mutex;
    bool m_capturing;
};

} // namespace sampleApp

#endif // SAMPLEAPP_AUDIOINPUTMANAGER_H_