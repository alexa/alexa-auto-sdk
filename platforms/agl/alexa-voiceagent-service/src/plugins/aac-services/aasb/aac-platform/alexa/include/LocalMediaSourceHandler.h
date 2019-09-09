/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_ALEXA_LOCALMEDIASOURCEHANDLER_H
#define AASB_ALEXA_LOCALMEDIASOURCEHANDLER_H

#include <memory>
#include <condition_variable>

#include <AACE/Alexa/LocalMediaSource.h>
#include "ResponseDispatcher.h"
#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

class LocalMediaSourceHandler : public aace::alexa::LocalMediaSource {
public:
    ~LocalMediaSourceHandler() {
    }

    static std::shared_ptr<LocalMediaSourceHandler> create(
        Source source,
        std::shared_ptr<aasb::core::logger::LoggerHandler> logger,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    /// @name aace::alexa::LocalMediaSource Functions
    /// @{
    bool play(ContentSelector contentSelectorType, const std::string& payload) override;
    bool playControl(PlayControlType controlType) override;
    bool seek(std::chrono::milliseconds offset) override;
    bool adjustSeek(std::chrono::milliseconds deltaOffset) override;
    LocalMediaSourceState getState() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(MutedState state) override;
    /// @}

    /**
     * Process incoming events from AASB client meant for topic @c TOPIC_LOCAL_MEDIA_SOURCE
     *
     * @param action Type of event.
     * @param payload Data required to process the event. Complex data can be represented
     *      in JSON string.
     */
    void onReceivedEvent(const std::string& action, const std::string& payload);

private:
    LocalMediaSourceHandler(
        Source source,
        std::weak_ptr<aasb::bridge::ResponseDispatcher> responseDispatcher);

    // Source String
    std::string m_SourceString;

    // Convert source to topic name
    std::string convertSourceToString(Source source);

    // Convert PlayControlType to string
    std::string convertPlayControlTypeToString(PlayControlType controlType);

    // Convert ContentSelectorType to string
    std::string convertContentSelectorTypeToString(ContentSelector contentSelectorType);

    // Populate media source state from JSON
    bool setMediaSourceState(const std::string& payload);

    // Current media state
    LocalMediaSourceState m_mediaSourceState;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;

    // ResponseDispatcher to send status info
    std::weak_ptr<aasb::bridge::ResponseDispatcher> m_responseDispatcher;

    // For sync over async
    std::condition_variable m_getStateResponseCv;
    std::mutex m_mutex;
    bool m_didReceiveGetStateResponse;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_LOCALMEDIASOURCEHANDLER_H