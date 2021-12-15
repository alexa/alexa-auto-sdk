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

#include <AACE/JNI/Audio/AudioOutputProviderBinder.h>
#include <AACE/JNI/Audio/AudioOutputBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.audio.AudioOutputProviderBinder";

namespace aace {
namespace jni {
namespace audio {

//
// AudioOutputProviderBinder
//

AudioOutputProviderBinder::AudioOutputProviderBinder(jobject obj) {
    m_audioOutputProviderHandler = std::make_shared<AudioOutputProviderHandler>(obj);
}

//
// AudioOutputProviderHandler
//

AudioOutputProviderHandler::AudioOutputProviderHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/audio/AudioOutputProvider") {
}

std::shared_ptr<aace::audio::AudioOutput> AudioOutputProviderHandler::openChannel(
    const std::string& name,
    AudioOutputType type) {
    try_with_context {
        jobject audioOutputTypeObj;
        ThrowIfNot(JAudioOutputType::checkType(type, &audioOutputTypeObj), "invalidAudioOutputType");

        jobject result = nullptr;
        ThrowIfNot(
            m_obj.invoke(
                "openChannel",
                "(Ljava/lang/String;Lcom/amazon/aace/audio/AudioOutputProvider$AudioOutputType;)Lcom/amazon/aace/audio/"
                "AudioOutput;",
                &result,
                JString(name).get(),
                audioOutputTypeObj),
            "invokeMethodFailed");
        ThrowIfNull(result, "invalidLocation");

        // create an audio input JObject
        JObject audioOutputObj(result, "com/amazon/aace/audio/AudioOutput");
        ThrowIfJavaEx(env, "invalidAudioOutputObj");

        // get the audio input binder native ref
        jlong nativeRef;
        ThrowIfNot(audioOutputObj.invoke("getNativeRef", "()J", &nativeRef), "invokeMethodFailed");

        // cast the native ref to an audio input ptr
        auto audioOutputBinder = reinterpret_cast<AudioOutputBinder*>(nativeRef);
        ThrowIfNull(audioOutputBinder, "invalidAudioOutputBinder");

        // get the audio input handler from the binder
        return audioOutputBinder->getAudioOutputHandler();
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "openChannel", ex.what());
        return nullptr;
    }
}

}  // namespace audio
}  // namespace jni
}  // namespace aace

#define AUDIO_OUTPUT_PROVIDER_BINDER(ref) reinterpret_cast<aace::jni::audio::AudioOutputProviderBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_audio_AudioOutputProvider_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::audio::AudioOutputProviderBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_audio_AudioOutputProvider_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioOutputProviderBinder = AUDIO_OUTPUT_PROVIDER_BINDER(ref);
        ThrowIfNull(audioOutputProviderBinder, "invalidAudioOutputProviderBinder");
        delete audioOutputProviderBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioOutputProvider_disposeBinder", ex.what());
    }
}
}
