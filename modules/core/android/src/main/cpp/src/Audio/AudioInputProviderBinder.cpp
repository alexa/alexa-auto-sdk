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

#include <AACE/JNI/Audio/AudioInputProviderBinder.h>
#include <AACE/JNI/Audio/AudioInputBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.audio.AudioInputProviderBinder";

namespace aace {
namespace jni {
namespace audio {

//
// AudioInputProviderBinder
//

AudioInputProviderBinder::AudioInputProviderBinder(jobject obj) {
    m_audioInputProviderHandler = std::make_shared<AudioInputProviderHandler>(obj);
}

//
// AudioInputProviderHandler
//

AudioInputProviderHandler::AudioInputProviderHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/audio/AudioInputProvider") {
}

std::shared_ptr<aace::audio::AudioInput> AudioInputProviderHandler::openChannel(
    const std::string& name,
    AudioInputType type) {
    try_with_context {
        jobject audioInputTypeObj;
        ThrowIfNot(JAudioInputType::checkType(type, &audioInputTypeObj), "invalidAudioInputType");

        jobject result = nullptr;
        ThrowIfNot(
            m_obj.invoke(
                "openChannel",
                "(Ljava/lang/String;Lcom/amazon/aace/audio/AudioInputProvider$AudioInputType;)Lcom/amazon/aace/audio/"
                "AudioInput;",
                &result,
                JString(name).get(),
                audioInputTypeObj),
            "invokeMethodFailed");
        ThrowIfNull(result, "invalidLocation");

        // create an audio input JObject
        JObject audioInputObj(result, "com/amazon/aace/audio/AudioInput");
        ThrowIfJavaEx(env, "invalidAudioInputObj");

        // get the audio input binder native ref
        jlong nativeRef;
        ThrowIfNot(audioInputObj.invoke("getNativeRef", "()J", &nativeRef), "invokeMethodFailed");

        // cast the native ref to an audio input ptr
        auto audioInputBinder = reinterpret_cast<AudioInputBinder*>(nativeRef);
        ThrowIfNull(audioInputBinder, "invalidAudioInputBinder");

        // get the audio input handler from the binder
        return audioInputBinder->getAudioInputHandler();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "openChannel", ex.what());
        return nullptr;
    }
}

}  // namespace audio
}  // namespace jni
}  // namespace aace

#define AUDIO_INPUT_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::audio::AudioInputProviderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_audio_AudioInputProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::audio::AudioInputProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_audio_AudioInputProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioInputProviderBinder = AUDIO_INPUT_PROVIDER_BINDER(ref);
        ThrowIfNull(audioInputProviderBinder, "invalidAudioInputProviderBinder");
        delete audioInputProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioInputProvider_disposeBinder", ex.what());
    }
}
}
