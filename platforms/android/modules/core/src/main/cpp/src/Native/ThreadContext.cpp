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

#include <AACE/JNI/Native/ThreadContext.h>
#include <AACE/JNI/Native/JavaClass.h>
#include <AACE/JNI/Native/NativeMacros.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.native.ThreadContext";

// global reference to the jvm
JavaVM* g_javaVM = nullptr;
static bool g_initializeClassLoaderAttempted = false;

namespace aace {
namespace jni {
namespace native {

ThreadContext::ThreadContext() : m_env(nullptr), m_detatch(false) {
    if (g_javaVM != nullptr) {
        m_detatch = g_javaVM->GetEnv((void**)&m_env, JNI_VERSION_1_6) == JNI_EDETACHED;

        if (m_detatch) {
            g_javaVM->AttachCurrentThread(&m_env, nullptr);
        }
    }
}

ThreadContext::~ThreadContext() {
    if (g_javaVM != nullptr && m_env != nullptr && m_detatch) {
        g_javaVM->DetachCurrentThread();
    }
}

JNIEnv* ThreadContext::getEnv() {
    return m_env;
}

bool ThreadContext::isValid() {
    return m_env != nullptr;
}

}  // namespace native
}  // namespace jni
}  // namespace aace

extern "C" {
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    try {
        JNIEnv* env;

        // validate the JNI version
        ThrowIfNot(vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK, "GetEnvFailed");

        // save the jvm reference
        g_javaVM = vm;

        // find the reference to the class loader from main thread so we
        // can use it to load classes at any time. We have to call this after
        // we set the global vm reference...
        if (!g_initializeClassLoaderAttempted) {
            ThrowIfNot(aace::jni::native::JavaClass::initializeClassLoader(), "initializeClassLoaderFailed");
            g_initializeClassLoaderAttempted = true;
        }

        return JNI_VERSION_1_6;
    } catch (const std::exception& ex) {
        AACE_JNI_CRITICAL(TAG, "JNI_OnLoad", ex.what());
        g_javaVM = nullptr;
        g_initializeClassLoaderAttempted = true;
        return -1;
    }
}

void JNI_OnUnload(JavaVM* vm, void* reserved) {
}
}