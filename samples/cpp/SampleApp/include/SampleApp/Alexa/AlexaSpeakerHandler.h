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

#ifndef SAMPLEAPP_ALEXA_ALEXASPEAKERHANDLER_H
#define SAMPLEAPP_ALEXA_ALEXASPEAKERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Alexa/AlexaSpeaker.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaSpeakerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlexaSpeakerHandler : public aace::alexa::AlexaSpeaker /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    AlexaSpeakerHandler(std::weak_ptr<Activity> activity, std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlexaSpeakerHandler> {
        return std::shared_ptr<AlexaSpeakerHandler>(new AlexaSpeakerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::alexa::AlexaSpeaker interface
    auto speakerSettingsChanged(aace::alexa::AlexaSpeaker::SpeakerType type, bool local, int8_t volume, bool mute)
        -> void override;

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;

    std::map<std::string, SpeakerType> identityMap{{"speaker", SpeakerType::ALEXA_VOLUME},
                                                   {"alerts", SpeakerType::ALERTS_VOLUME}};
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_ALEXASPEAKERHANDLER_H
