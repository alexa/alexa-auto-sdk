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

#ifndef AACE_JNI_ALEXA_SPEECH_RECOGNIZER_BINDER_H
#define AACE_JNI_ALEXA_SPEECH_RECOGNIZER_BINDER_H

#include <AACE/Alexa/SpeechRecognizer.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class SpeechRecognizerHandler : public aace::alexa::SpeechRecognizer {
public:
    SpeechRecognizerHandler(jobject obj);

    // aace::alexa::SpeechRecognizer
    bool wakewordDetected(const std::string& wakeword) override;
    void endOfSpeechDetected() override;

private:
    JObject m_obj;
};

class SpeechRecognizerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    SpeechRecognizerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_speechRecognizerHandler;
    }

    std::shared_ptr<SpeechRecognizerHandler> getSpeechRecognizer() {
        return m_speechRecognizerHandler;
    }

private:
    std::shared_ptr<SpeechRecognizerHandler> m_speechRecognizerHandler;
};

//
// JInitiator
//

class JInitiatorConfig : public EnumConfiguration<SpeechRecognizerHandler::Initiator> {
public:
    using T = SpeechRecognizerHandler::Initiator;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/SpeechRecognizer$Initiator";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::HOLD_TO_TALK, "HOLD_TO_TALK"}, {T::TAP_TO_TALK, "TAP_TO_TALK"}, {T::WAKEWORD, "WAKEWORD"}};
    }
};

using JInitiator = JEnum<SpeechRecognizerHandler::Initiator, JInitiatorConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_SPEECH_RECOGNIZER_BINDER_H
