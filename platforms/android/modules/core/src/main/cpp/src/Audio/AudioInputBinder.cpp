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

#include <AACE/JNI/Audio/AudioInputBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.audio.AudioInputBinder";

namespace aace {
namespace jni {
namespace audio {

//
// AudioInputBinder
//

AudioInputBinder::AudioInputBinder(jobject obj) {
    m_audioInputHandler = std::make_shared<AudioInputHandler>(obj);
}

//
// AudioInputHandler
//

AudioInputHandler::AudioInputHandler(jobject obj) : m_obj(obj, "com/amazon/aace/audio/AudioInput") {
}

bool AudioInputHandler::startAudioInput() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("startAudioInput", "()Z", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "startAudioInput", ex.what());
        return false;
    }
}

bool AudioInputHandler::stopAudioInput() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("stopAudioInput", "()Z", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "stopAudioInput", ex.what());
        return false;
    }
}

}  // namespace audio
}  // namespace jni
}  // namespace aace

#define AUDIO_INPUT_BINDER(ref) reinterpret_cast<aace::jni::audio::AudioInputBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_audio_AudioInput_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::audio::AudioInputBinder(obj));
}

JNIEXPORT void JNICALL Java_com_amazon_aace_audio_AudioInput_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioInputBinder = AUDIO_INPUT_BINDER(ref);
        ThrowIfNull(audioInputBinder, "invalidAudioInputBinder");
        delete audioInputBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioInput_disposeBinder", ex.what());
    }
}

JNIEXPORT jlong JNICALL Java_com_amazon_aace_audio_AudioInput_write(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jbyteArray data,
    jlong offset,
    jlong size) {
    try {
        auto audioInputBinder = AUDIO_INPUT_BINDER(ref);
        ThrowIfNull(audioInputBinder, "invalidAudioInputBinder");

        JByteArray arr(data);

        jint count = audioInputBinder->getAudioInputHandler()->write((int16_t*)arr.ptr(offset), size / 2);

        return count * 2;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioInput_write", ex.what());
        return 0;
    }
}
}
