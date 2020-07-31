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

#include <AACE/JNI/Alexa/AudioPlayerBinder.h>
#include <AACE/JNI/Core/NativeLib.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.AudioPlayerBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// AudioPlayerBinder
//

AudioPlayerBinder::AudioPlayerBinder(jobject obj) {
    m_audioPlayerHandler = std::make_shared<AudioPlayerHandler>(obj);
}

//
// AudioPlayerHandler
//

AudioPlayerHandler::AudioPlayerHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/AudioPlayer") {
}

void AudioPlayerHandler::playerActivityChanged(PlayerActivity state) {
    try_with_context {
        jobject stateObj;

        ThrowIfNot(JAudioPlayerPlayerActivity::checkType(state, &stateObj), "invalidState");
        ThrowIfNot(
            m_obj.invoke<void>(
                "playerActivityChanged", "(Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;)V", nullptr, stateObj),
            "invokeFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "playerActivityChanged", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define AUDIO_PLAYER_BINDER(ref) reinterpret_cast<aace::jni::alexa::AudioPlayerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_AudioPlayer_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::AudioPlayerBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AudioPlayer_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_AudioPlayer_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioPlayerBinder = AUDIO_PLAYER_BINDER(ref);
        ThrowIfNull(audioPlayerBinder, "invalidAudioPlayerBinder");
        delete audioPlayerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AudioPlayer_disposeBinder", ex.what());
    }
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_alexa_AudioPlayer_getPlayerPosition(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioPlayerBinder = AUDIO_PLAYER_BINDER(ref);
        ThrowIfNull(audioPlayerBinder, "invalidAudioPlayerBinder");

        return audioPlayerBinder->getAudioPlayer()->getPlayerPosition();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AudioPlayer_getPlayerPosition", ex.what());
        return -1;
    }
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_alexa_AudioPlayer_getPlayerDuration(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto audioPlayerBinder = AUDIO_PLAYER_BINDER(ref);
        ThrowIfNull(audioPlayerBinder, "invalidAudioPlayerBinder");

        return audioPlayerBinder->getAudioPlayer()->getPlayerDuration();
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_AudioPlayer_getPlayerDuration", ex.what());
        return -1;
    }
}
}
