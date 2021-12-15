/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_TEXTTOSPEECH_TEXTTOSPEECH_BINDER_H
#define AACE_JNI_TEXTTOSPEECH_TEXTTOSPEECH_BINDER_H

#include <AACE/TextToSpeech/TextToSpeech.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace textToSpeech {

class TextToSpeechHandler : public aace::textToSpeech::TextToSpeech {
public:
    TextToSpeechHandler(jobject obj);

    // aace::textToSpeech::TextToSpeech
    void prepareSpeechCompleted(
        const std::string& speechId,
        std::shared_ptr<aace::audio::AudioStream> preparedAudio,
        const std::string& metadata) override;
    void prepareSpeechFailed(const std::string& speechId, const std::string& reason) override;
    void capabilitiesReceived(const std::string& requestId, const std::string& capabilities) override;

private:
    JObject m_obj;
};

class TextToSpeechBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    TextToSpeechBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_textToSpeechHandler;
    }

    std::shared_ptr<TextToSpeechHandler> getTextToSpeech() {
        return m_textToSpeechHandler;
    }

private:
    std::shared_ptr<TextToSpeechHandler> m_textToSpeechHandler;
};

}  // namespace textToSpeech
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_TEXTTOSPEECH_TEXTTOSPEECH_BINDER_H