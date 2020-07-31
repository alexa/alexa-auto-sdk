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

#include <AACE/JNI/Native/JavaString.h>
#include <AACE/JNI/Native/NativeMacros.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.native.JavaString";

namespace aace {
namespace jni {
namespace native {

JavaString::JavaString() : m_cstr(nullptr) {
}

JavaString::JavaString(jobject jstr) : m_cstr(nullptr) {
    try_with_context {
        if (jstr != nullptr) {
            m_ref = GlobalRef<jstring>((jstring)jstr);
        }
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaString", ex.what());
    }
}

JavaString::JavaString(jstring jstr) : m_cstr(nullptr) {
    if (jstr != nullptr) {
        m_ref = GlobalRef<jstring>(jstr);
    }
}

JavaString::JavaString(const std::string& str) : m_cstr(nullptr) {
    try_with_context {
        jstring jstr = env->NewStringUTF(str.c_str());
        ThrowIfJavaEx(env, "newStringUTFFailed");
        m_ref = GlobalRef<jstring>(jstr);
        // delete local string ref
        env->DeleteLocalRef(jstr);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaString", ex.what());
    }
}

JavaString::JavaString(const char* str) : m_cstr(nullptr) {
    try_with_context {
        jstring jstr = env->NewStringUTF(str);
        ThrowIfJavaEx(env, "newStringUTFFailed");
        m_ref = GlobalRef<jstring>(jstr);
        // delete local string ref
        env->DeleteLocalRef(jstr);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaString", ex.what());
    }
}

JavaString::~JavaString() {
    try_with_context {
        if (m_ref.get() != nullptr && m_cstr != nullptr) {
            env->ReleaseStringUTFChars(m_ref.get(), m_cstr);
            ThrowIfJavaEx(env, "releaseStringUTFCharsFailed");
        }
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "~JavaString", ex.what());
    }
}

jstring JavaString::get() {
    try_with_context {
        ThrowIfNull(m_ref.get(), "invalidStringRef");
        return (jstring)env->NewLocalRef(m_ref.get());
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "get", ex.what());
        return nullptr;
    }
}

std::string JavaString::toStdStr() {
    try_with_context {
        ReturnIf(toCStr() == nullptr, std::string()) return m_cstr;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "toStdStr", ex.what());
        return std::string();
    }
}

const char* JavaString::toCStr() {
    if (m_cstr == nullptr) {
        try_with_context {
            ThrowIfNull(get(), "invalidGlobalStringRef");
            m_cstr = context.getEnv()->GetStringUTFChars(get(), nullptr);
            ThrowIfJavaEx(env, "getStringUTFCharsFailed");
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG, "toCStr", ex.what());
        }
    }

    return m_cstr;
}

}  // namespace native
}  // namespace jni
}  // namespace aace
