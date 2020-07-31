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

#ifndef AACE_JNI_ALEXA_SPEECH_SYNTHESIZER_BINDER_H
#define AACE_JNI_ALEXA_SPEECH_SYNTHESIZER_BINDER_H

#include <AACE/Alexa/SpeechSynthesizer.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class SpeechSynthesizerHandler : public aace::alexa::SpeechSynthesizer {
public:
    SpeechSynthesizerHandler();
};

class SpeechSynthesizerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    SpeechSynthesizerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_speechSynthesizerHandler;
    }

private:
    std::shared_ptr<SpeechSynthesizerHandler> m_speechSynthesizerHandler;
};

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_SPEECH_SYNTHESIZER_BINDER_H
