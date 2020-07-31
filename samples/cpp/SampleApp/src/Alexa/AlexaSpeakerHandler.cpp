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

#include "SampleApp/Alexa/AlexaSpeakerHandler.h"

// C++ Standard Library
#include <sstream>
#include <regex>  // std::regex

// Guidelines Support Library
#define GSL_THROW_ON_CONTRACT_VIOLATION
#include <gsl/contracts.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaSpeakerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AlexaSpeakerHandler::AlexaSpeakerHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler) :
        m_activity{std::move(activity)}, m_loggerHandler{std::move(loggerHandler)} {
    // Expects((m_activity != nullptr) && (m_loggerHandler != nullptr));
    setupUI();
}

std::weak_ptr<Activity> AlexaSpeakerHandler::getActivity() {
    return m_activity;
}

std::weak_ptr<logger::LoggerHandler> AlexaSpeakerHandler::getLoggerHandler() {
    return m_loggerHandler;
}

// aace::alexa::AlexaSpeaker interface
void AlexaSpeakerHandler::speakerSettingsChanged(
    aace::alexa::AlexaSpeaker::SpeakerType type,
    bool local,
    int8_t volume,
    bool mute) {
    std::stringstream ss;
    ss << "[type=" << type << ",local=" << local << ",volume=" << static_cast<int>(volume) << ",mute=" << mute << "]";
    log(logger::LoggerHandler::Level::INFO, "speakerSettingsChanged" + ss.str());
}

// private

void AlexaSpeakerHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "AlexaSpeakerHandler", message);
}

void AlexaSpeakerHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    m_console = activity->findViewById("id:console");

    activity->registerObserver(Event::onAudioManagerSpeaker, [=](const std::string& value) {
        log(logger::LoggerHandler::Level::VERBOSE, "onAudioManagerSpeaker:" + value);
        static std::regex value_regex("(.+)/(.+)/(.+)", std::regex::optimize);
        std::smatch value_match{};
        if (std::regex_match(value, value_match, value_regex)) {
            if ((value_match.size() - 1) == 3) {
                auto identity = std::string(value_match[1]);
                auto paramName = std::string(value_match[2]);
                auto paramValue = std::string(value_match[3]);
                SpeakerType type;

                // Normalize to lower case
                std::transform(identity.begin(), identity.end(), identity.begin(), ::tolower);
                if (identityMap.find(identity) != identityMap.end()) {
                    type = identityMap.at(identity);
                } else {
                    if (auto console = m_console.lock()) {
                        console->printRuler();
                        console->printLine("Speaker type is invalid: " + identity);
                        console->print("Update your menu with one of these values: ");
                        // Iterate over the map using c++11 range based for loop
                        for (std::pair<std::string, SpeakerType> el : identityMap) {
                            console->print(el.first + " ");
                        }
                        console->printLine("");
                        console->printRuler();
                    }
                    return false;
                }

                if (paramName == "mute") {
                    auto value = paramValue == "true" || paramValue == "1";
                    localSetMute(type, value);
                    if (auto console = m_console.lock()) {
                        std::stringstream message;
                        message << "Entity  : Volume" << std::endl
                                << "Speaker : " << identity << std::endl
                                << "Action  : Mute" << std::endl
                                << "Value   : " << std::boolalpha << value << std::noboolalpha;

                        console->printRuler();
                        console->printLine(message.str());
                        console->printRuler();
                    }
                } else if (paramName == "volume") {
                    std::stringstream message;
                    int value = std::stoi(paramValue);
                    if ((paramValue[0] == '+') || (paramValue[0] == '-')) {
                        localAdjustVolume(type, value);
                        if (auto console = m_console.lock()) {
                            message << "Entity  : Volume" << std::endl
                                    << "Speaker : " << identity << std::endl
                                    << "Action  : Adjust Volume" << std::endl
                                    << "Value   : " << paramValue;

                            console->printRuler();
                            console->printLine(message.str());
                            console->printRuler();
                        }
                    } else {
                        localSetVolume(type, value);
                        if (auto console = m_console.lock()) {
                            message << "Entity  : Volume" << std::endl
                                    << "Speaker : " << identity << std::endl
                                    << "Action  : Set Volume" << std::endl
                                    << "Value   : " << paramValue;

                            console->printRuler();
                            console->printLine(message.str());
                            console->printRuler();
                        }
                    }
                }
                return true;
            }
        } else {
            if (auto console = m_console.lock()) {
                console->printRuler();
                console->printLine("Invalid value found. Refer to documentation for valid value format.");
                console->printRuler();
            }
        }
        return false;
    });
}

}  // namespace alexa
}  // namespace sampleApp
