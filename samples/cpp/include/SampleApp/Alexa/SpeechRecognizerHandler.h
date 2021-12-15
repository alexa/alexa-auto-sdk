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

#ifndef SAMPLEAPP_ALEXA_SPEECHRECOGNIZERHANDLER_H
#define SAMPLEAPP_ALEXA_SPEECHRECOGNIZERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"
#include "SampleApp/PropertyManager/PropertyManagerHandler.h"

#include <AACE/Alexa/AlexaProperties.h>
#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/SpeechRecognizer/StartCaptureMessage.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// C++ Standard Library
#include <atomic>  // std::atomic

namespace sampleApp {

namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SpeechRecognizerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SpeechRecognizerHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::weak_ptr<propertyManager::PropertyManagerHandler> m_propertyManagerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

    std::string m_audioFilePath{};
    std::atomic<bool> m_isStreamingAudioFile{false};
    std::atomic<bool> m_shouldStopStreamingAudioFile{false};
    std::future<void> m_streamTask{};

protected:
    SpeechRecognizerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<propertyManager::PropertyManagerHandler> propertyManagerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<SpeechRecognizerHandler> {
        return std::shared_ptr<SpeechRecognizerHandler>(new SpeechRecognizerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the EndOfSpeechDetected messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleEndOfSpeechDetectedMessage(const std::string& message);

    /**
     * Handles the WakewordDetected messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleWakewordDetectedMessage(const std::string& message);

    /**
     * Provides the implementation for when a wake word is detected
     *
     * @param [in] wakeword The wake word that was detected
     */
    void wakewordDetected(const std::string& wakeword);

    /**
     * Provides the implementation for when end of
     * speech is detected for the current recognize event
     */
    void endOfSpeechDetected();

    /**
     * Publishes the StartCapture message to notify the Engine of a speech recognition event.
     *
     * @param [in] initiator The @c Initiator type for the speech recognition event
     */
    void startCapture(aasb::message::alexa::speechRecognizer::Initiator initiator);

    /**
     * Publishes the StopCapture message to notify the Engine to terminate the current recognize event. The Engine will
     * notify the platform implementation when to stop writing audio samples with the StopAudioInput message.
     */
    void stopCapture();

    /**
     * Notifies the Engine of a speech recognition event initiated by a press-and-hold action on the
     * platform. 
     */
    void holdToTalk();

    /**
     * Notifies the Engine of a speech recognition event initiated by a press-and-release action on the
     * platform.
     *
     * The Engine will terminate the recognize event initiated by the press-and-release action
     * when end of speech is detected.
     */
    void tapToTalk();

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_SPEECHRECOGNIZERHANDLER_H
