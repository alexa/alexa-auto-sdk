/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AASB/Engine/Alexa/AASBSpeechRecognizer.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <AASB/Message/Alexa/SpeechRecognizer/EndOfSpeechDetectedMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/Initiator.h>
#include <AASB/Message/Alexa/SpeechRecognizer/StartCaptureMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/StopCaptureMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/WakewordDetectedMessage.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBSpeechRecognizer");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBSpeechRecognizer> AASBSpeechRecognizer::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the speech recognizer platform handler
        auto speechRecognizer = std::shared_ptr<AASBSpeechRecognizer>(new AASBSpeechRecognizer());

        // initialize the platform handler
        ThrowIfNot(speechRecognizer->initialize(messageBroker), "initializeFailed");

        return speechRecognizer;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBSpeechRecognizer::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        // save the message broker reference
        m_messageBroker = messageBroker;

        // create a wp reference
        std::weak_ptr<AASBSpeechRecognizer> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::speechRecognizer::StartCaptureMessage::topic(),
            aasb::message::alexa::speechRecognizer::StartCaptureMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::alexa::speechRecognizer::StartCaptureMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    sp->startCapture(
                        static_cast<Initiator>(payload.initiator),
                        payload.keywordBegin,
                        payload.keywordEnd,
                        payload.keyword);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "StartCaptureMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::speechRecognizer::StopCaptureMessage::topic(),
            aasb::message::alexa::speechRecognizer::StopCaptureMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    sp->stopCapture();
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "StopCaptureMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

bool AASBSpeechRecognizer::wakewordDetected(const std::string& wakeword) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::speechRecognizer::WakewordDetectedMessage message;
        message.payload.wakeword = wakeword;

        m_messageBroker_lock->publish(message.toString()).send();

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

void AASBSpeechRecognizer::endOfSpeechDetected() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::speechRecognizer::EndOfSpeechDetectedMessage message;

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
