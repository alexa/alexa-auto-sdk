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

#include <AACE/JNI/Alexa/SpeechSynthesizerBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.SpeechSynthesizerBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// SpeechSynthesizerBinder
//

SpeechSynthesizerBinder::SpeechSynthesizerBinder(jobject obj) {
    m_speechSynthesizerHandler = std::make_shared<SpeechSynthesizerHandler>();
}

//
// SpeechSynthesizerHandler
//

SpeechSynthesizerHandler::SpeechSynthesizerHandler() {
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define SPEECH_SYNTHESIZER_BINDER(ref) reinterpret_cast<aace::jni::alexa::SpeechSynthesizerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_SpeechSynthesizer_createBinder(
    JNIEnv* env,
    jobject obj,
    jlong mediaPlayerRef,
    jlong speakeRef) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::SpeechSynthesizerBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_SpeechSynthesizer_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_SpeechSynthesizer_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto speechSynthesizerBinder = SPEECH_SYNTHESIZER_BINDER(ref);
        ThrowIfNull(speechSynthesizerBinder, "invalidNotificationsBinder");
        delete speechSynthesizerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_SpeechSynthesizer_disposeBinder", ex.what());
    }
}
}
