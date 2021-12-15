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

#include <AACE/JNI/Alexa/AlexaSpeakerBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.AlexaSpeakerBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// AlexaSpeakerBinder
//

AlexaSpeakerBinder::AlexaSpeakerBinder(jobject obj) {
    m_alexaSpeakerHandler = std::make_shared<AlexaSpeakerHandler>(obj);
}

//
// AlexaSpeakerHandler
//

AlexaSpeakerHandler::AlexaSpeakerHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/AlexaSpeaker") {
}

void AlexaSpeakerHandler::speakerSettingsChanged(SpeakerType type, bool local, int8_t volume, bool mute) {
    try_with_context {
        jobject typeObj;
        ThrowIfNot(JSpeakerType::checkType(type, &typeObj), "invalidSpeakerType");
        ThrowIfNot(
            m_obj.invoke<void>(
                "speakerSettingsChanged",
                "(Lcom/amazon/aace/alexa/AlexaSpeaker$SpeakerType;ZBZ)V",
                nullptr,
                typeObj,
                local,
                volume,
                mute),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "speakerSettingsChanged", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define ALEXA_SPEAKER_BINDER(ref) reinterpret_cast<aace::jni::alexa::AlexaSpeakerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_AlexaSpeaker_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::AlexaSpeakerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_AlexaSpeaker_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto alexaSpeakerBinder = ALEXA_SPEAKER_BINDER(ref);
        ThrowIfNull(alexaSpeakerBinder, "invalidAlexaSpeakerBinder");
        delete alexaSpeakerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aaced_alexa_AlexaSpeaker_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_AlexaSpeaker_localSetVolume(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject type,
    jbyte volume) {
    try {
        auto alexaSpeakerBinder = ALEXA_SPEAKER_BINDER(ref);
        ThrowIfNull(alexaSpeakerBinder, "invalidAlexaSpeakerBinder");

        aace::jni::alexa::AlexaSpeakerHandler::SpeakerType speakerType;
        ThrowIfNot(aace::jni::alexa::JSpeakerType::checkType(type, &speakerType), "invalidSpeakerType");

        alexaSpeakerBinder->getAlexaSpeaker()->localSetVolume(speakerType, volume);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AlexaSpeaker_setVolume", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_AlexaSpeaker_localAdjustVolume(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject type,
    jbyte delta) {
    try {
        auto alexaSpeakerBinder = ALEXA_SPEAKER_BINDER(ref);
        ThrowIfNull(alexaSpeakerBinder, "invalidAlexaSpeakerBinder");

        aace::jni::alexa::AlexaSpeakerHandler::SpeakerType speakerType;
        ThrowIfNot(aace::jni::alexa::JSpeakerType::checkType(type, &speakerType), "invalidSpeakerType");

        alexaSpeakerBinder->getAlexaSpeaker()->localAdjustVolume(speakerType, delta);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AlexaSpeaker_adjustVolume", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_AlexaSpeaker_localSetMute(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject type,
    jboolean mute) {
    try {
        auto alexaSpeakerBinder = ALEXA_SPEAKER_BINDER(ref);
        ThrowIfNull(alexaSpeakerBinder, "invalidAlexaSpeakerBinder");

        aace::jni::alexa::AlexaSpeakerHandler::SpeakerType speakerType;
        ThrowIfNot(aace::jni::alexa::JSpeakerType::checkType(type, &speakerType), "invalidSpeakerType");

        alexaSpeakerBinder->getAlexaSpeaker()->localSetMute(speakerType, mute);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AlexaSpeaker_setMute", ex.what());
    }
}
}
