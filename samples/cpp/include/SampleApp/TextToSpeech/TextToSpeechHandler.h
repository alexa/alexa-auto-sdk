/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_TEXTTOSPEECH_TEXTTOSPEECHHANDLER_H
#define SAMPLEAPP_TEXTTOSPEECH_TEXTTOSPEECHHANDLER_H

#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <unordered_map>
#include <future>
#include <utility>

namespace sampleApp {
namespace textToSpeech {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextToSpeechHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class TextToSpeechHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    TextToSpeechHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<TextToSpeechHandler> {
        return std::shared_ptr<TextToSpeechHandler>(new TextToSpeechHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the PrepareSpeechCompletedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePrepareSpeechCompletedMessage(const std::string& message);

    /**
     * Handles the PrepareSpeechFailedMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handlePrepareSpeechFailedMessage(const std::string& message);

    /**
     * Handles the GetCapabilitiesReplyMessage received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetCapabilitiesReplyMessage(const std::string& message);

    /**
     * Publishes the PrepareSpeech message to notify the Engine to prepare speech from
     * a text/SSML input.
     *
     * @param [in] speechId The speech ID
     * @param [in] text The text/SSML to be used for speech synthesis
     * @param [in] provider The text to speech provider to be used for speech synthesis
     * @param [in] options The options to be used for speech synthesis
     */
    void prepareSpeech(
        const std::string& speechId,
        const std::string& text,
        const std::string& provider,
        const std::string& options);

    /**
     * Notifies the platform implementation about a successful speech synthesis.
     *
     * @param [in] speechId The speech ID
     * @param [in] preparedAudio The audio asset
     * @param [in] metadata The metadata associated with the speech resource
     */
    void prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::core::MessageStream> preparedAudio,
        const std::string& metadata);

    /**
     * Notifies the platform implementation about a failed speech synthesis.
     *
     * @param [in] speechId The speech ID
     * @param [in] reason The failure reason
     */
    void prepareSpeechFailed(const std::string& speechId, const std::string& reason);

    /**
     * Publishes the GetCapabilities message to notify the Engine to get capabilities of a
     * Text to Speech provider.
     *
     * A reply message is expected from the Engine. The default wait time for the reply is 1000 ms.
     * If the reply is not received within that time, a timeout occurs.
     *
     * @param [in] requestId A string that will act as a unique identifier for this message
     * @param [in] provider The text to speech provider to be used for speech synthesis
     * @return 	The capabilities of the Text to Speech provider
     */
    std::string getCapabilities(const std::string& requestId, const std::string& provider);

    /**
     * Notifies the platform implementation of a Text to Speech provider's capabilities.
     *
     * @param [in] requestId The ID of the get capabilities message that this one is replying to
     * @param [in] capabilities The capabilities of the Text to Speech provider
     */
    void capabilitiesReceived(const std::string& requestId, const std::string& capabilities);

private:
    using TextToSpeechPromise = std::promise<std::string>;
    std::mutex m_promise_map_access_mutex;
    std::unordered_map<std::string, std::shared_ptr<TextToSpeechPromise>> m_promiseMap;

    std::string waitForAsyncReply(const std::string& messageId);
    void addReplyMessagePromise(const std::string& messageId, std::shared_ptr<TextToSpeechPromise> promise);
    void removeReplyMessagePromise(const std::string& messageId);
    std::shared_ptr<TextToSpeechPromise> getReplyMessagePromise(const std::string& messageId);

private:
    std::weak_ptr<View> m_console{};

    std::atomic<long> m_idCounter;

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto generateId() -> std::string;
};

}  // namespace textToSpeech
}  // namespace sampleApp

#endif  // SAMPLEAPP_TEXTTOSPEECH_TEXTTOSPEECHHANDLER_H