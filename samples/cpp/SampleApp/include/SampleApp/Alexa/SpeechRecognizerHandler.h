/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
//#include "SampleApp/AudioFileReader.h"
//#include "SampleApp/AudioInputManager.h"
#include "SampleApp/Logger/LoggerHandler.h"
#include "SampleApp/PropertyManager/PropertyManagerHandler.h"

#include <AACE/Alexa/AlexaProperties.h>
#include <AACE/Alexa/SpeechRecognizer.h>

// Alexa Auto Core - Audio Reference Implementation
//#include <AACE/Audio/AudioCapture.h>

// C++ Standard Library
#include <atomic>  // std::atomic

namespace sampleApp {

namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SpeechRecognizerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class SpeechRecognizerHandler : public aace::alexa::SpeechRecognizer /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};
    std::weak_ptr<propertyManager::PropertyManagerHandler> m_propertyManagerHandler{};

    std::string m_audioFilePath{};
    std::atomic<bool> m_isStreamingAudioFile{false};
    std::atomic<bool> m_shouldStopStreamingAudioFile{false};
    std::future<void> m_streamTask{};

protected:
    SpeechRecognizerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::weak_ptr<propertyManager::PropertyManagerHandler> propertyManagerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<SpeechRecognizerHandler> {
        return std::shared_ptr<SpeechRecognizerHandler>(new SpeechRecognizerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::alexa::SpeechRecognizer interface

    auto wakewordDetected(const std::string& wakeword) -> bool override;
    auto endOfSpeechDetected() -> void override;

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_SPEECHRECOGNIZERHANDLER_H
