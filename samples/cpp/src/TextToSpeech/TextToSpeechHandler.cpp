/*
 * Copyright 2021-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Core/EngineMacros.h"
#include "SampleApp/TextToSpeech/TextToSpeechHandler.h"

#include <AASB/Message/TextToSpeech/TextToSpeech/GetCapabilitiesMessage.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechCompletedMessage.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechFailedMessage.h>
#include <AASB/Message/TextToSpeech/TextToSpeech/PrepareSpeechMessage.h>

// C++ Standard Library
#include <sstream>
#include <unistd.h>
#include <utility>
#include <fstream>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace textToSpeech {

using namespace aasb::message::textToSpeech::textToSpeech;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TextToSpeechHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t ASYNC_REPLY_TIMEOUT = 1000;
static constexpr size_t READ_BUFFER_SIZE = 4096;

static const std::string ID("TEXT_TO_SPEECH");
static const std::string TEXT_TO_SPEECH_PROVIDER("text-to-speech-provider");

using MessageBroker = aace::core::MessageBroker;
using MessageStream = aace::core::MessageStream;

TextToSpeechHandler::TextToSpeechHandler(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<MessageBroker> messageBroker) :
        m_activity{std::move(activity)},
        m_loggerHandler{std::move(loggerHandler)},
        m_messageBroker{std::move(messageBroker)},
        m_idCounter(0) {
    setupUI();
    subscribeToAASBMessages();
}

std::weak_ptr<logger::LoggerHandler> TextToSpeechHandler::getLoggerHandler() {
    return m_loggerHandler;
}

std::weak_ptr<Activity> TextToSpeechHandler::getActivity() {
    return m_activity;
}

void TextToSpeechHandler::subscribeToAASBMessages() {
    log(logger::LoggerHandler::Level::INFO, "Subscribe to PrepareSpeechCompleted Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePrepareSpeechCompletedMessage(message); },
        PrepareSpeechCompletedMessage::topic(),
        PrepareSpeechCompletedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to PrepareSpeechFailed Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handlePrepareSpeechFailedMessage(message); },
        PrepareSpeechFailedMessage::topic(),
        PrepareSpeechFailedMessage::action());

    log(logger::LoggerHandler::Level::INFO, "Subscribe to GetCapabilitiesReply Messages");
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleGetCapabilitiesReplyMessage(message); },
        GetCapabilitiesMessageReply::topic(),
        GetCapabilitiesMessageReply::action());
}

void TextToSpeechHandler::handlePrepareSpeechCompletedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PrepareSpeechCompletedMessage");
    PrepareSpeechCompletedMessage msg = json::parse(message);
    prepareSpeechCompleted(
        msg.payload.speechId,
        m_messageBroker->openStream(msg.payload.streamId, MessageStream::Mode::READ),
        msg.payload.metadata);
}

void TextToSpeechHandler::handlePrepareSpeechFailedMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received PrepareSpeechFailedMessage");
    PrepareSpeechFailedMessage msg = json::parse(message);
    prepareSpeechFailed(msg.payload.speechId, msg.payload.reason);
}

void TextToSpeechHandler::handleGetCapabilitiesReplyMessage(const std::string& message) {
    log(logger::LoggerHandler::Level::INFO, "Received GetCapabilitiesReplyMessage");
    GetCapabilitiesMessageReply msg = json::parse(message);

    auto promise = getReplyMessagePromise(msg.header.messageDescription.replyToId);
    if (promise) {
        promise->set_value(msg.payload.capabilities);
    } else {
        log(logger::LoggerHandler::Level::ERROR, "promise is null");
    }

    capabilitiesReceived(msg.header.messageDescription.replyToId, msg.payload.capabilities);
}

void TextToSpeechHandler::prepareSpeech(
    const std::string& speechId,
    const std::string& text,
    const std::string& provider,
    const std::string& options) {
    // Publish the "PrepareSpeech" message
    PrepareSpeechMessage msg;
    msg.payload.speechId = speechId;
    msg.payload.text = text;
    msg.payload.provider = provider;
    msg.payload.options = "";
    m_messageBroker->publish(msg.toString());
}

void TextToSpeechHandler::prepareSpeechCompleted(
    const std::string& speechId,
    std::shared_ptr<MessageStream> preparedAudio,
    const std::string& metadata) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Prepare speech Completed");
            console->printLine("Speech ID      : ", speechId);
            console->printLine("Metadata       : ", metadata);
            console->printRuler();
        }
    });
    // For QA testing only. The OEM application should follow the UX guidelines in
    // order to play the audio
    try {
        auto path = "" + speechId + ".mp3";
        log(logger::LoggerHandler::Level::VERBOSE, "path" + path);

        // create the output file
        auto output = std::make_shared<std::ofstream>(path, std::ios::binary);
        // copy the stream to the file
        char buffer[READ_BUFFER_SIZE];
        ssize_t size = 0;

        while (!preparedAudio->isClosed()) {
            ssize_t bytesRead = preparedAudio->read(buffer, READ_BUFFER_SIZE);

            // throw an error if the read failed
            if (bytesRead < 0) {
                log(logger::LoggerHandler::Level::VERBOSE, "bytes read is zero");
                return;
            }

            // write the data to the output file
            output->write(buffer, bytesRead);

            size += bytesRead;
        }

        output->close();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, ex.what());
    }
}

void TextToSpeechHandler::prepareSpeechFailed(const std::string& speechId, const std::string& reason) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Prepare speech Failed");
            console->printLine("Speech ID      : ", speechId);
            console->printLine("Reason       : ", reason);
            console->printRuler();
        }
    });
}

std::string TextToSpeechHandler::getCapabilities(const std::string& requestId, const std::string& provider) {
    // Publish the "GetCapabilities" message
    GetCapabilitiesMessage msg;
    msg.header.id = requestId;
    msg.payload.provider = provider;
    m_messageBroker->publish(msg.toString());
    return waitForAsyncReply(msg.header.id);
}

void TextToSpeechHandler::capabilitiesReceived(const std::string& requestId, const std::string& capabilities) {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }
    activity->runOnUIThread([=]() {
        if (auto console = m_console.lock()) {
            console->printRuler();
            console->printLine("Capabilities Received");
            console->printLine("Capabilities      : ", capabilities);
            console->printLine("Request ID      : ", requestId);
            console->printRuler();
        }
    });
}

void TextToSpeechHandler::log(logger::LoggerHandler::Level level, const std::string& message) {
    auto loggerHandler = m_loggerHandler.lock();
    if (!loggerHandler) {
        return;
    }
    loggerHandler->log(level, "TextToSpeechHandler", message);
}

void TextToSpeechHandler::setupUI() {
    auto activity = m_activity.lock();
    if (!activity) {
        return;
    }

    m_console = activity->findViewById("id:console");

    // prepareSpeech
    activity->registerObserver(Event::onPrepareSpeech, [=](const std::string& value) {
        auto speechId = generateId();
        log(logger::LoggerHandler::Level::VERBOSE, "onPrepareSpeech text: " + value + " speech ID: " + speechId);
        prepareSpeech(speechId, value, TEXT_TO_SPEECH_PROVIDER, "");
        return true;
    });

    // getCapabilities
    activity->registerObserver(Event::onGetCapabilities, [=](const std::string& value) {
        auto requestId = generateId();
        log(logger::LoggerHandler::Level::VERBOSE, "onGetCapabilities request ID: " + requestId);
        getCapabilities(requestId, TEXT_TO_SPEECH_PROVIDER);
        return true;
    });
}

std::string TextToSpeechHandler::generateId() {
    return ID + "-" + std::to_string(++m_idCounter);
}

std::string TextToSpeechHandler::waitForAsyncReply(const std::string& messageId) {
    // create the promise for the text to speech reply message to fulfill
    std::shared_ptr<TextToSpeechPromise> promise = std::make_shared<TextToSpeechPromise>();

    // create a future to receive the promised text to speech reply message when it is received
    std::shared_future<std::string> future(promise->get_future());

    std::string success = "";
    try {
        addReplyMessagePromise(messageId, promise);
        ThrowIfNot(
            future.wait_for(std::chrono::milliseconds(ASYNC_REPLY_TIMEOUT)) == std::future_status::ready,
            "replyMessageTimeout:id=" + messageId);
        ThrowIfNot(future.valid(), "invalidMessageResponse");
        success = future.get();
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "waitForAsyncReply " + std::string(ex.what()));
        promise->set_exception(std::current_exception());
    }
    removeReplyMessagePromise(messageId);
    return success;
}

void TextToSpeechHandler::addReplyMessagePromise(
    const std::string& messageId,
    std::shared_ptr<TextToSpeechPromise> promise) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        ThrowIf(m_promiseMap.find(messageId) != m_promiseMap.end(), "messageIdAlreadyExists");

        // add the promise to the promise map
        m_promiseMap[messageId] = promise;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "addReplyMessagePromise " + std::string(ex.what()));
    }
}

void TextToSpeechHandler::removeReplyMessagePromise(const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        // remove the promise from the promise map
        m_promiseMap.erase(messageId);
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "removeReplyMessagePromise " + std::string(ex.what()));
    }
}

std::shared_ptr<TextToSpeechHandler::TextToSpeechPromise> TextToSpeechHandler::getReplyMessagePromise(
    const std::string& messageId) {
    try {
        std::lock_guard<std::mutex> lock(m_promise_map_access_mutex);

        auto it = m_promiseMap.find(messageId);
        ThrowIf(it == m_promiseMap.end(), "messageIdDoesNotExist");

        return it->second;
    } catch (std::exception& ex) {
        log(logger::LoggerHandler::Level::ERROR, "getReplyMessagePromise " + std::string(ex.what()));
        return nullptr;
    }
}

}  // namespace textToSpeech
}  // namespace sampleApp
