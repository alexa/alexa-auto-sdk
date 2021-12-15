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

#ifndef SAMPLEAPP_ALEXA_ALEXASPEAKERHANDLER_H
#define SAMPLEAPP_ALEXA_ALEXASPEAKERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/AlexaSpeaker/SpeakerSettingsChangedMessage.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AlexaSpeakerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AlexaSpeakerHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    AlexaSpeakerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<AlexaSpeakerHandler> {
        return std::shared_ptr<AlexaSpeakerHandler>(new AlexaSpeakerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    /**
     * Handles the SpeakerSettingsChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine with the changed speaker settings.
     */
    void handleSpeakerSettingsChangedMessage(const std::string& message);

    /**
     * Publishes LocalSetVolumeMessage to notify the Engine of a volume change event.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     * must be scaled to the range [0,100].
     */
    void localSetVolume(aasb::message::alexa::alexaSpeaker::SpeakerType type, int8_t volume);

    /**
     * Publishes LocalAdjustVolumeMessage to notify the Engine of a relative adjustment to the volume setting of the Speaker,
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] delta The volume adjustment to apply to the Speaker. @c delta is
     * in the range [-100, 100].
     */
    void localAdjustVolume(aasb::message::alexa::alexaSpeaker::SpeakerType type, int8_t delta);

    /**
     * Publishes LocalSetMuteMessage to notify the Engine of a mute setting change event
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] mute The new mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false
     */
    void localSetMute(aasb::message::alexa::alexaSpeaker::SpeakerType type, bool mute);

    /**
     * Provides the platform implementation for the changed speaker settings.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] local @c true if the change originated from calling @c AlexaSpeaker::localSetVolume,
     *             @c AlexaSpeaker::localAdjustVolume, or @c AlexaSpeaker::localSetMute.
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     *             will be scaled to the range [0,100].
     * @param [in] mute The mute setting of the Speaker. @c true when the Speaker is muted,
     *             else @c false.
     */
    void speakerSettingsChanged(
        aasb::message::alexa::alexaSpeaker::SpeakerType type,
        bool local,
        int8_t volume,
        bool mute);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto convertSpeakerTypeToString(const aasb::message::alexa::alexaSpeaker::SpeakerType& type) -> std::string;

    std::map<std::string, aasb::message::alexa::alexaSpeaker::SpeakerType> identityMap{
        {"speaker", aasb::message::alexa::alexaSpeaker::SpeakerType::ALEXA_VOLUME},
        {"alerts", aasb::message::alexa::alexaSpeaker::SpeakerType::ALERTS_VOLUME}};
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_ALEXASPEAKERHANDLER_H
