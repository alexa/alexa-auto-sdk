/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Audio/AudioOutputBinder.h>
#include <AACE/JNI/Audio/AudioStreamBinder.h>
#include <AACE/JNI/TextToSpeech/TextToSpeechBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.textToSpeech.TextToSpeechBinder";

namespace aace {
namespace jni {
namespace textToSpeech {

//
// TextToSpeechBinder
//

TextToSpeechBinder::TextToSpeechBinder(jobject obj) {
    m_textToSpeechHandler = std::make_shared<TextToSpeechHandler>(obj);
}

//
// TextToSpeechHandler
//

TextToSpeechHandler::TextToSpeechHandler(jobject obj) : m_obj(obj, "com/amazon/aace/textToSpeech/TextToSpeech") {
}

void TextToSpeechHandler::prepareSpeechCompleted(
    const std::string& speechId,
    std::shared_ptr<aace::audio::AudioStream> preparedAudio,
    const std::string& metadata) {
    try_with_context {
        JObject javaAudioStream("com/amazon/aace/audio/AudioStream");
        ThrowIfJavaEx(env, "createAudioStreamFailed");

        // create the audio stream binder
        long audioStreamBinder = reinterpret_cast<long>(new audio::AudioStreamBinder(preparedAudio));

        // set the java audio stream object native ref to the audio stream binder
        javaAudioStream.invoke<void>("setNativeRef", "(J)V", nullptr, audioStreamBinder);

        ThrowIfNot(
            m_obj.invoke<void>(
                "prepareSpeechCompleted",
                "(Ljava/lang/String;Lcom/amazon/aace/audio/AudioStream;Ljava/lang/String;)V",
                nullptr,
                JString(speechId).get(),
                javaAudioStream.get(),
                JString(metadata).get()),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "prepareSpeechCompleted", ex.what());
    }
}

void TextToSpeechHandler::prepareSpeechFailed(const std::string& speechId, const std::string& reason) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "prepareSpeechFailed",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(speechId).get(),
                JString(reason).get()),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "prepareSpeechFailed", ex.what());
    }
}

void TextToSpeechHandler::capabilitiesReceived(const std::string& requestId, const std::string& capabilities) {
    try_with_context {
        ThrowIfNot(
            m_obj.invoke<void>(
                "capabilitiesReceived",
                "(Ljava/lang/String;Ljava/lang/String;)V",
                nullptr,
                JString(requestId).get(),
                JString(capabilities).get()),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "capabilitiesReceived", ex.what());
    }
}

}  // namespace textToSpeech
}  // namespace jni
}  // namespace aace

#define TEXTTOSPEECH_BINDER(ref) reinterpret_cast<aace::jni::textToSpeech::TextToSpeechBinder*>(ref)

extern "C" {

JNIEXPORT jlong JNICALL Java_com_amazon_aace_textToSpeech_TextToSpeech_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::textToSpeech::TextToSpeechBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_textToSpeech_TextToSpeech_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto textToSpeechBinder = TEXTTOSPEECH_BINDER(ref);
        ThrowIfNull(textToSpeechBinder, "invalidTextToSpeechBinder");
        delete textToSpeechBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_textToSpeech_TextToSpeech_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_textToSpeech_TextToSpeech_prepareSpeech(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring speechId,
    jstring text,
    jstring provider,
    jstring options) {
    try {
        auto textToSpeechBinder = TEXTTOSPEECH_BINDER(ref);
        ThrowIfNull(textToSpeechBinder, "invalidTextToSpeechBinder");

        ThrowIfNot(
            textToSpeechBinder->getTextToSpeech()->prepareSpeech(
                JString(speechId).toStdStr(),
                JString(text).toStdStr(),
                JString(provider).toStdStr(),
                JString(options).toStdStr()),
            "prepareSpeechFailed");
        return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_textToSpeech_TextToSpeech_getCapabilities(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring requestId,
    jstring provider) {
    try {
        auto textToSpeechBinder = TEXTTOSPEECH_BINDER(ref);
        ThrowIfNull(textToSpeechBinder, "invalidTextToSpeechBinder");

        ThrowIfNot(
            textToSpeechBinder->getTextToSpeech()->getCapabilities(
                JString(requestId).toStdStr(), JString(provider).toStdStr()),
            "getCapabilitiesFailed");
        return true;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, __func__, ex.what());
        return false;
    }
}
}
