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

#include <AACE/JNI/Alexa/GlobalPresetBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.alexa.GlobalPresetBinder";

namespace aace {
namespace jni {
namespace alexa {

//
// GlobalPresetBinder
//

GlobalPresetBinder::GlobalPresetBinder(jobject obj) {
    m_globalPresetHandler = std::make_shared<GlobalPresetHandler>(obj);
}

//
// GlobalPresetHandler
//

GlobalPresetHandler::GlobalPresetHandler(jobject obj) : m_obj(obj, "com/amazon/aace/alexa/GlobalPreset") {
}

void GlobalPresetHandler::setGlobalPreset(int preset) {
    try_with_context {
        ThrowIfNot(m_obj.invoke<void>("setGlobalPreset", "(I)V", nullptr, preset), "invokeMethodFailed");
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "setGlobalPreset", ex.what());
    }
}

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#define GLOBAL_PRESET_BINDER(ref) reinterpret_cast<aace::jni::alexa::GlobalPresetBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_alexa_GlobalPreset_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::alexa::GlobalPresetBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_GlobalPreset_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto globalPresetBinder = GLOBAL_PRESET_BINDER(ref);
        ThrowIfNull(globalPresetBinder, "invalidGlobalPresetBinder");
        delete globalPresetBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_alexa_GlobalPreset_disposeBinder", ex.what());
    }
}
}
