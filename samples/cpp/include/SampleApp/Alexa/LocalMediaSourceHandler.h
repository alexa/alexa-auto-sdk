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

#ifndef SAMPLEAPP_ALEXA_LOCALMEDIASOURCEHANDLER_H
#define SAMPLEAPP_ALEXA_LOCALMEDIASOURCEHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include "AASB/Message/Alexa/ExternalMediaAdapter/MediaType.h"
#include "AASB/Message/Alexa/ExternalMediaAdapter/MutedState.h"
#include "AASB/Message/Alexa/ExternalMediaAdapter/PlayControlType.h"
#include <AASB/Message/Alexa/LocalMediaSource/ContentSelector.h>
#include <AASB/Message/Alexa/LocalMediaSource/LocalMediaSourceState.h>
#include <AASB/Message/Alexa/LocalMediaSource/Source.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalMediaSourceHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalMediaSourceHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    LocalMediaSourceHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        aasb::message::alexa::localMediaSource::Source source,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<LocalMediaSourceHandler> {
        return std::shared_ptr<LocalMediaSourceHandler>(new LocalMediaSourceHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the AdjustSeek message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleAdjustSeekMessage(const std::string& message);

    /**
     * Handles the Seek message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSeekMessage(const std::string& message);

    /**
     * Handles the GetState message received from the Engine.
     * The state information is published back to the ending in a Reply message
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetStateMessage(const std::string& message);

    /**
     * Handles the MutedStateChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleMutedStateChangedMessage(const std::string& message);

    /**
     * Handles the VolumeChanged message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleVolumeChangedMessage(const std::string& message);

    /**
     * Handles the PlayControl message received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePlayControlMessage(const std::string& message);

    /**
     * Handles the Play message received from the Engine.
     *
     *  @note Since Alexa is starting the session, use the session Id in the message for further events and errors.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePlayMessage(const std::string& message);

    /**
     * Publish PlayerEventMessage to notify the Engine of local media source player event.
     *
     * @param [in] eventName Canonical event name. Accepted values:
     *      @li "PlaybackSessionStarted"
     *      @li "PlaybackSessionEnded"
     *      @li "PlaybackStarted"
     *      @li "PlaybackStopped"
     *
     * @param [in] sessionId A universally unique identifier (UUID) generated according to the RFC 4122 specification.
     */
    void playerEvent(const std::string& eventName, const std::string& sessionId);

    /**
     * Publish PlayerErrorMessage to notify the Engine of local media source player error.
     *
     * @param [in] errorName The name of the error. Accepted values:
     *      @li "INTERNAL_ERROR"
     * @param [in] code The error code
     * @param [in] description The detailed error description
     * @param [in] fatal true if the error is fatal
     * @param [in] sessionId A universally unique identifier (UUID) generated according to the RFC 4122 specification.
     */
    void playerError(
        const std::string& errorName,
        long code,
        const std::string& description,
        bool fatal,
        const std::string& sessionId);

    /**
     * Provides the implementation for play with a content selection type
     *
     * @param [in] ContentSelector Content selection type
     * @param [in] payload Content selector payload (e.g. "1", "98.7 FM HD 1", "bbc radio four")
     * @param [in] sessionId A universally unique identifier (UUID) generated according to the RFC 4122 specification.
     */
    void play(
        aasb::message::alexa::localMediaSource::ContentSelector contentSelectorType,
        const std::string& payload,
        const std::string& sessionId);

    /**
     * Provides the implementation for play control.
     *
     * @param [in] controlType Playback control type being invoked
     */
    void playControl(aasb::message::alexa::externalMediaAdapter::PlayControlType controlType);

    /**
     * Provides the implementation for seek when the user invokes media seek via voice.
     *
     * @param [in] offset Offset position within media item, in milliseconds
     */
    void seek(std::chrono::milliseconds offset);

    /**
     * Provides implementation for media seek adjustment via speech.
     *
     * @param [in] deltaOffset Change in offset position within media item, in milliseconds
     */
    void adjustSeek(std::chrono::milliseconds deltaOffset);

    /**
     * Provides the state information of the local media source.
     */
    aasb::message::alexa::localMediaSource::LocalMediaSourceState getState();

    /**
     * Provides implementation to set the volume of the output channel.
     *
     * @param [in] volume The volume to set on the output channel. @c volume
     * is in the range [0,1].
     */
    void volumeChanged(float volume);

    /**
     * Provides implementation to apply a muted state has changed for
     * the output channel
     *
     * @param [in] state The muted state to apply to the output channel.
     */
    void mutedStateChanged(aasb::message::alexa::externalMediaAdapter::MutedState state);

    /**
     * Publishes the reply to GetState message from the Engine, with the local media source state information.
     *
     * @param [in] messageId The Id of the GetStateMessage for which the reply is published
     */
    void publishGetStateReply(const std::string& messageId);

private:
    std::weak_ptr<View> m_console{};

    std::string m_sourceMediaProvider = "UNDEFINED";
    std::string m_sessionId;
    aasb::message::alexa::externalMediaAdapter::MediaType m_sourceMediaType =
        aasb::message::alexa::externalMediaAdapter::MediaType::OTHER;
    aasb::message::alexa::localMediaSource::Source m_source;

    std::map<aasb::message::alexa::localMediaSource::Source, std::string> m_localMediaSourceStateMap = {
        {aasb::message::alexa::localMediaSource::Source::BLUETOOTH, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::USB, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::FM_RADIO, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::AM_RADIO, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::SATELLITE_RADIO, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::LINE_IN, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::COMPACT_DISC, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::SIRIUS_XM, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::DAB, "IDLE"},
        {aasb::message::alexa::localMediaSource::Source::DEFAULT, "IDLE"}};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto convertContentSelectorToString(aasb::message::alexa::localMediaSource::ContentSelector contentSelectorType)
        -> std::string;
    auto convertPlayControlTypeToString(aasb::message::alexa::externalMediaAdapter::PlayControlType playControlType)
        -> std::string;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_LOCALMEDIASOURCEHANDLER_H
