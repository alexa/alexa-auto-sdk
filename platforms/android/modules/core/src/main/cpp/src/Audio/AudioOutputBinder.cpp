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

#include <AACE/JNI/Audio/AudioOutputBinder.h>
#include <AACE/JNI/Audio/AudioStreamBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.audio.AudioOutputBinder";

namespace aace {
namespace jni {
namespace audio {

//
// AudioOutputBinder
//

AudioOutputBinder::AudioOutputBinder(jobject obj) {
    m_audioOutputHandler = std::make_shared<AudioOutputHandler>(obj);
}

//
// AudioOutputHandler
//

AudioOutputHandler::AudioOutputHandler(jobject obj) : m_obj(obj, "com/amazon/aace/audio/AudioOutput") {
}

bool AudioOutputHandler::prepare(std::shared_ptr<aace::audio::AudioStream> stream, bool repeating) {
    try_with_context {
        JObject javaAudioStream("com/amazon/aace/audio/AudioStream");
        ThrowIfJavaEx(env, "createAudioStreamFailed");

        // create the audio stream binder
        long audioStreamBinder = reinterpret_cast<long>(new AudioStreamBinder(stream));

        // set the java audio stream object native ref to the audio stream binder
        javaAudioStream.invoke<void>("setNativeRef", "(J)V", nullptr, audioStreamBinder);

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "prepare", "(Lcom/amazon/aace/audio/AudioStream;Z)Z", &result, javaAudioStream.get(), repeating),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "prepare", ex.what());
        return false;
    }
}

bool AudioOutputHandler::prepare(const std::string& url, bool repeating) {
    try_with_context {
        jboolean result;
        ThrowIfNot(
            m_obj.invoke("prepare", "(Ljava/lang/String;Z)Z", &result, JString(url).get(), repeating),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "prepare", ex.what());
        return false;
    }
}

bool AudioOutputHandler::play() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("play", "()Z", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "play", ex.what());
        return false;
    }
}

bool AudioOutputHandler::stop() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("stop", "()Z", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "stop", ex.what());
        return false;
    }
}

bool AudioOutputHandler::pause() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("pause", "()Z", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "pause", ex.what());
        return false;
    }
}

bool AudioOutputHandler::resume() {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("resume", "()Z", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "resume", ex.what());
        return false;
    }
}

int64_t AudioOutputHandler::getPosition() {
    try_with_context {
        jlong result;
        ThrowIfNot(m_obj.invoke("getPosition", "()J", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getPosition", ex.what());
        return AudioOutput::TIME_UNKNOWN;
    }
}

bool AudioOutputHandler::setPosition(int64_t position) {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("setPosition", "(J)Z", &result, position), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setPosition", ex.what());
        return false;
    }
}

int64_t AudioOutputHandler::getDuration() {
    try_with_context {
        jlong result;
        ThrowIfNot(m_obj.invoke("getDuration", "()J", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getDuration", ex.what());
        return AudioOutput::TIME_UNKNOWN;
    }
}

int64_t AudioOutputHandler::getNumBytesBuffered() {
    try_with_context {
        jlong result;
        ThrowIfNot(m_obj.invoke("getNumBytesBuffered", "()J", &result), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getNumBytesBuffered", ex.what());
        return 0;
    }
}

bool AudioOutputHandler::volumeChanged(float volume) {
    try_with_context {
        jboolean result;
        ThrowIfNot(m_obj.invoke("volumeChanged", "(F)Z", &result, volume), "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "volumeChanged", ex.what());
        return false;
    }
}

bool AudioOutputHandler::mutedStateChanged(MutedState state) {
    try_with_context {
        jobject mutedStateObj;
        ThrowIfNot(JMutedState::checkType(state, &mutedStateObj), "invalidMutedState");

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "mutedStateChanged", "(Lcom/amazon/aace/audio/AudioOutput$MutedState;)Z", &result, mutedStateObj),
            "invokeMethodFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "mutedStateChanged", ex.what());
        return false;
    }
}

}  // namespace audio
}  // namespace jni
}  // namespace aace

#define AUDIO_OUTPUT_BINDER(ref) reinterpret_cast<aace::jni::audio::AudioOutputBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_audio_AudioOutput_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::audio::AudioOutputBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_audio_AudioOutput_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioOutputBinder = AUDIO_OUTPUT_BINDER(ref);
        ThrowIfNull(audioOutputBinder, "invalidAudioOutputBinder");
        delete audioOutputBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioOutput_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_audio_AudioOutput_mediaError(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject type,
    jstring error) {
    try {
        auto audioOutputBinder = AUDIO_OUTPUT_BINDER(ref);
        ThrowIfNull(audioOutputBinder, "invalidAudioOutputBinder");

        audioOutputBinder->getAudioOutputHandler()->mediaError(
            aace::jni::audio::JMediaError::from(
                type, aace::jni::audio::AudioOutputHandler::MediaError::MEDIA_ERROR_UNKNOWN),
            JString(error).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioOutput_mediaError", ex.what());
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_audio_AudioOutput_mediaStateChanged(JNIEnv* env, jobject /* this */, jlong ref, jobject state) {
    try {
        auto audioOutputBinder = AUDIO_OUTPUT_BINDER(ref);
        ThrowIfNull(audioOutputBinder, "invalidAudioOutputBinder");

        aace::jni::audio::AudioOutputHandler::MediaState checkedState;

        ThrowIfNot(aace::jni::audio::JMediaState::checkType(state, &checkedState), "invalidMediaState");

        audioOutputBinder->getAudioOutputHandler()->mediaStateChanged(checkedState);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_audio_AudioOutput_mediaStateChanged", ex.what());
    }
}
}
