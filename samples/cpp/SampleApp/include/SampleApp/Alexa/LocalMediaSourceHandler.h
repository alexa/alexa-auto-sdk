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

#ifndef SAMPLEAPP_ALEXA_LOCALMEDIASOURCEHANDLER_H
#define SAMPLEAPP_ALEXA_LOCALMEDIASOURCEHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Alexa/LocalMediaSource.h>

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  LocalMediaSourceHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class LocalMediaSourceHandler : public aace::alexa::LocalMediaSource /* isa PlatformInterface */ {
private:
    std::weak_ptr<Activity> m_activity{};
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    LocalMediaSourceHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        Source source);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<LocalMediaSourceHandler> {
        return std::shared_ptr<LocalMediaSourceHandler>(new LocalMediaSourceHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::alexa::LocalMediaSource interface

    auto play(ContentSelector contentSelectorType, const std::string& payload) -> bool override;
    auto playControl(PlayControlType controlType) -> bool override;
    auto seek(std::chrono::milliseconds offset) -> bool override;
    auto adjustSeek(std::chrono::milliseconds deltaOffset) -> bool override;
    auto getState() -> LocalMediaSourceState override;

    auto volumeChanged(float volume) -> bool override;
    auto mutedStateChanged(MutedState state) -> bool override;

private:
    std::weak_ptr<View> m_console{};

    std::string m_sourceMediaProvider = "UNDEFINED";
    aace::alexa::LocalMediaSource::MediaType m_sourceMediaType = aace::alexa::LocalMediaSource::MediaType::OTHER;
    aace::alexa::LocalMediaSource::Source m_source;

    std::map<aace::alexa::LocalMediaSource::Source, std::string> m_localMediaSourceStateMap = {
        {aace::alexa::LocalMediaSource::Source::BLUETOOTH, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::USB, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::FM_RADIO, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::AM_RADIO, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::LINE_IN, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::COMPACT_DISC, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::SIRIUS_XM, "IDLE"},
        {aace::alexa::LocalMediaSource::Source::DAB, "IDLE"}};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_LOCALMEDIASOURCEHANDLER_H
