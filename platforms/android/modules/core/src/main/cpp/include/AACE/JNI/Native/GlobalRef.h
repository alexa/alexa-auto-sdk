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

#ifndef AACE_JNI_NATIVE_GLOBAL_REF_H
#define AACE_JNI_NATIVE_GLOBAL_REF_H

#include <jni.h>
#include <AACE/JNI/Native/NativeMacros.h>

namespace aace {
namespace jni {
namespace native {

template <class T>
class GlobalRef {
public:
    GlobalRef() : m_globalRef(nullptr) {
    }

    GlobalRef(JNIEnv* env, T obj) : m_globalRef(nullptr) {
        try {
            ThrowIfNull(obj, "invalidReferenceObect");
            m_globalRef = (T)env->NewGlobalRef(obj);
            ThrowIfJavaEx(env, "newGlobalRefFailed");
        } catch (const std::exception& ex) {
            AACE_JNI_ERROR("aace.jni.native.GlobalRef", "GlobalRef", ex.what());
        }
    }

    GlobalRef(T obj) : m_globalRef(nullptr) {
        try_with_context {
            ThrowIfNull(obj, "invalidReferenceObect");
            m_globalRef = (T)env->NewGlobalRef(obj);
            ThrowIfJavaEx(env, "newGlobalRefFailed");
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.GlobalRef", "GlobalRef", ex.what());
        }
    }

    GlobalRef(const GlobalRef<T>& ref) : m_globalRef(nullptr) {
        try_with_context {
            ThrowIfNull(ref.m_globalRef, "invalidReferenceObect");
            m_globalRef = (T)env->NewGlobalRef(ref.m_globalRef);
            ThrowIfJavaEx(env, "newGlobalRefFailed");
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.GlobalRef", "GlobalRef", ex.what());
        }
    }

    ~GlobalRef() {
        reset();
    }

    void reset() {
        if (m_globalRef != nullptr) {
            try_with_context {
                env->DeleteGlobalRef(m_globalRef);
                ThrowIfJavaEx(env, "deleteGlobalRefFailed");
            }
            catch_with_ex {
                AACE_JNI_ERROR("aace.jni.native.GlobalRef", "reset", ex.what());
            }

            m_globalRef = nullptr;
        }
    }

    T get() {
        return m_globalRef;
    }

    // assignment opperator
    GlobalRef<T>& operator=(const GlobalRef<T>& ref) {
        try_with_context {
            if (ref.m_globalRef != nullptr) {
                m_globalRef = (T)env->NewGlobalRef(ref.m_globalRef);
                ThrowIfJavaEx(env, "newGlobalRefFailed");
            } else {
                m_globalRef = nullptr;
            }
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.GlobalRef", "operator=", ex.what());
            m_globalRef = nullptr;
        }

        return *this;
    }

    GlobalRef<T>& operator=(const T& ref) {
        try_with_context {
            if (ref != nullptr) {
                m_globalRef = (T)env->NewGlobalRef(ref);
                ThrowIfJavaEx(env, "newGlobalRefFailed");
            } else {
                m_globalRef = nullptr;
            }
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.GlobalRef", "operator=", ex.what());
            m_globalRef = nullptr;
        }

        return *this;
    }

private:
    T m_globalRef;
};

}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_GLOBAL_REF_H
