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

#include <AACE/JNI/Alexa/SpeechRecognizerBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.SpeechRecognizerBinder";

// type aliases
using Initiator = aace::jni::alexa::SpeechRecognizerHandler::Initiator;

namespace aace {
namespace jni {
namespace alexa {

//
// SpeechRecognizerBinder
//

SpeechRecognizerBinder::SpeechRecognizerBinder(jobject obj) {
    m_speechRecognizerHandler = std::make_shared<SpeechRecognizerHandler>(obj);
}

//
// SpeechRecognizerHandler
//

SpeechRecognizerHandler::SpeechRecognizerHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/SpeechRecognizer") {
}

bool SpeechRecognizerHandler::wakewordDetected(const std::string& wakeword) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("wakewordDetected", "(Ljava/lang/String;)Z", &result, JString(wakeword).get()),
            "invokeFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "wakewordDetected", ex.what());
        return false;
    }
}

void SpeechRecognizerHandler::endOfSpeechDetected() {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("endOfSpeechDetected", "()V", nullptr), "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "endOfSpeechDetected", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define SPEECH_RECOGNIZER_BINDER(ref) reinterpret_cast<aace::jni::alexa::SpeechRecognizerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_SpeechRecognizer_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::SpeechRecognizerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto speechRecognizerBinder = SPEECH_RECOGNIZER_BINDER(ref);
        ThrowIfNull(speechRecognizerBinder, "invalidSpeechRecognizerBinder");
        delete speechRecognizerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aaced_alexa_SpeechRecognizer_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_holdToTalk(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto speechRecognizerBinder = SPEECH_RECOGNIZER_BINDER(ref);
        ThrowIfNull(speechRecognizerBinder, "invalidSpeechRecognizerBinder");

        return speechRecognizerBinder->getSpeechRecognizer()->holdToTalk();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_SpeechRecognizer_holdToTalk", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_tapToTalk(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto speechRecognizerBinder = SPEECH_RECOGNIZER_BINDER(ref);
        ThrowIfNull(speechRecognizerBinder, "invalidSpeechRecognizerBinder");

        return speechRecognizerBinder->getSpeechRecognizer()->tapToTalk();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_SpeechRecognizer_tapToTalk", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_alexa_SpeechRecognizer_startCapture(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject initiator,
    jlong keywordBegin,
    jlong keywordEnd,
    jstring keyword) {
    try {
        auto speechRecognizerBinder = SPEECH_RECOGNIZER_BINDER(ref);
        ThrowIfNull(speechRecognizerBinder, "invalidSpeechRecognizerBinder");

        Initiator initiatorType;
        ThrowIfNot(aace::jni::alexa::JInitiator::checkType(initiator, &initiatorType), "invalidInitiatorType");

        return speechRecognizerBinder->getSpeechRecognizer()->startCapture(
            initiatorType,
            keywordBegin < 0 ? aace::jni::alexa::SpeechRecognizerHandler::UNSPECIFIED_INDEX : keywordBegin,
            keywordEnd < 0 ? aace::jni::alexa::SpeechRecognizerHandler::UNSPECIFIED_INDEX : keywordEnd,
            JString(keyword).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_SpeechRecognizer_startCapture", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_stopCapture(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto speechRecognizerBinder = SPEECH_RECOGNIZER_BINDER(ref);
        ThrowIfNull(speechRecognizerBinder, "invalidSpeechRecognizerBinder");

        return speechRecognizerBinder->getSpeechRecognizer()->stopCapture();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_SpeechRecognizer_stopCapture", ex.what());
        return false;
    }
}
}
