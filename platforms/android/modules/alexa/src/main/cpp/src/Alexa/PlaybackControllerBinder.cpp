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

#include <AACE/JNI/Alexa/PlaybackControllerBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.PlaybackControllerBinder";

// type aliases
using PlaybackButton = aace::jni::alexa::PlaybackControllerHandler::PlaybackButton;
using PlaybackToggle = aace::jni::alexa::PlaybackControllerHandler::PlaybackToggle;

namespace aace {
namespace jni {
namespace alexa {

//
// PlaybackControllerBinder
//

PlaybackControllerBinder::PlaybackControllerBinder(jobject obj) {
    m_playbackControllerHandler = std::make_shared<PlaybackControllerHandler>();
}

//
// PlaybackControllerHandler
//

PlaybackControllerHandler::PlaybackControllerHandler() {
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define PLAYBACK_CONTROLLER_BINDER(ref) reinterpret_cast<aace::jni::alexa::PlaybackControllerBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_PlaybackController_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::PlaybackControllerBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto playbackControllerBinder = PLAYBACK_CONTROLLER_BINDER(ref);
        ThrowIfNull(playbackControllerBinder, "invalidPlaybackControllerBinder");
        delete playbackControllerBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_PlaybackController_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_PlaybackController_buttonPressed(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject playbackButton) {
    try {
        auto playbackControllerBinder = PLAYBACK_CONTROLLER_BINDER(ref);
        ThrowIfNull(playbackControllerBinder, "invalidPlaybackControllerBinder");

        PlaybackButton buttonType;
        ThrowIfNot(
            aace::jni::alexa::JPlaybackButton::checkType(playbackButton, &buttonType), "invalidPlaybackButtonType");

        playbackControllerBinder->getPlaybackController()->buttonPressed(buttonType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_PlaybackController_buttonPressed", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_PlaybackController_togglePressed(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject playbackToggle,
    jboolean action) {
    try {
        auto playbackControllerBinder = PLAYBACK_CONTROLLER_BINDER(ref);
        ThrowIfNull(playbackControllerBinder, "invalidPlaybackControllerBinder");

        PlaybackToggle toggleType;
        ThrowIfNot(
            aace::jni::alexa::JPlaybackToggle::checkType(playbackToggle, &toggleType), "invalidPlaybackToggleType");

        playbackControllerBinder->getPlaybackController()->togglePressed(toggleType, action);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_PlaybackController_togglePressed", ex.what());
    }
}
}
