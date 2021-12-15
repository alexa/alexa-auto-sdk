/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/Core/NativeLib.h>

#include "AACE/JNI/Alexa/MediaPlaybackRequestorBinder.h"

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.MediaPlaybackRequestorBinder";

namespace aace {
namespace jni {
namespace alexa {

MediaPlaybackRequestorHandler::MediaPlaybackRequestorHandler(jobject obj) :
        m_obj(obj, "com/amazon/aace/alexa/MediaPlaybackRequestor") {
}

void MediaPlaybackRequestorHandler::mediaPlaybackResponse(MediaPlaybackRequestStatus mediaPlaybackRequestStatus) {
    try_with_context {
        jobject checkedMediaPlaybackRequestStatus;
        ThrowIfNot(
            JMediaPlaybackRequestStatus::checkType(mediaPlaybackRequestStatus, &checkedMediaPlaybackRequestStatus),
            "invalidMediaPlaybackRequestStatus");
        ThrowIfNot(
            m_obj.invoke<void>(
                "mediaPlaybackResponse",
                "(Lcom/amazon/aace/alexa/MediaPlaybackRequestor$MediaPlaybackRequestStatus;)V",
                nullptr,
                checkedMediaPlaybackRequestStatus),
            "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "mediaPlaybackResponse", ex.what());
    }
}

MediaPlaybackRequestorBinder::MediaPlaybackRequestorBinder(jobject obj) {
    m_mediaPlaybackRequestorHandler = std::make_shared<MediaPlaybackRequestorHandler>(obj);
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define MEDIA_PLAYBACK_REQUESTOR_BINDER(ref) reinterpret_cast<aace::jni::alexa::MediaPlaybackRequestorBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_MediaPlaybackRequestor_createBinder(JNIEnv* env, jobject obj) {
    try {
        return reinterpret_cast<long>(new aace::jni::alexa::MediaPlaybackRequestorBinder(obj));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_MediaPlaybackRequestor_createBinder", ex.what());
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlaybackRequestor_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto mediaPlaybackRequestorBinder = MEDIA_PLAYBACK_REQUESTOR_BINDER(ref);
        ThrowIfNull(mediaPlaybackRequestorBinder, "invalidMediaPlaybackRequestorBinder");

        delete mediaPlaybackRequestorBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_MediaPlaybackRequestor_disposeBinder", ex.what());
    }
}

JNIEXPORT void JNICALL Java_com_amazon_aace_alexa_MediaPlaybackRequestor_requestMediaPlayback(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jobject invocationReason,
    jlong elapsedBootTime) {
    try {
        auto mediaPlaybackRequestorBinder = MEDIA_PLAYBACK_REQUESTOR_BINDER(ref);
        ThrowIfNull(mediaPlaybackRequestorBinder, "invalidMediaPlaybackRequestorBinder");

        aace::alexa::MediaPlaybackRequestor::InvocationReason checkedInvocationReason;
        ThrowIfNot(
            aace::jni::alexa::JInvocationReason::checkType(invocationReason, &checkedInvocationReason),
            "invalidInvocationReason");
        mediaPlaybackRequestorBinder->getMediaPlaybackRequestor()->requestMediaPlayback(
            checkedInvocationReason, elapsedBootTime);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_MediaPlaybackRequestor_requestMediaPlayback", ex.what());
    }
}
}

// END OF FILE
