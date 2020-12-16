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

#ifndef AACE_JNI_NATIVE_JAVA_CLASS_H
#define AACE_JNI_NATIVE_JAVA_CLASS_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "JavaMethod.h"
#include "JavaField.h"
#include "GlobalRef.h"

namespace aace {
namespace jni {
namespace native {

class JavaClass {
private:
    JavaClass(const char* name, jclass cls);

public:
    static bool initializeClassLoader();
    static std::shared_ptr<JavaClass> find(const char* name);

    // method accessors
    JavaMethodPtr getStaticMethod(const char* name, const char* signature);
    JavaMethodPtr getMethod(const char* name, const char* signature);
    JavaFieldPtr getStaticField(const char* name, const char* signature);
    JavaFieldPtr getField(const char* name, const char* signature);

    bool isObjectInstance(jobject obj);

    // static method invocation
    template <class T>
    bool invokeV(JavaMethodPtr method, T* result, va_list);
    template <class T>
    bool invoke(JavaMethodPtr method, T* result, ...);

    jobject newInstance(const char* signature, ...);

    // jclass reference accessor
    jclass get();

private:
    std::string m_name;
    GlobalRef<jclass> m_globalClassRef;
    std::unordered_map<std::string, JavaMethodPtr> m_methodMap;
    std::unordered_map<std::string, JavaFieldPtr> m_fieldMap;

    // global class registry
    static std::unordered_map<std::string, std::shared_ptr<JavaClass>> s_javaClassRegistry;
    static GlobalRef<jobject> s_classLoaderObjectRef;
    static jmethodID s_findClassMethodID;

    // mutex
    static std::mutex s_mutex;
};

using JavaClassPtr = std::shared_ptr<JavaClass>;

// invokeV<void>
template <>
inline bool JavaClass::invokeV<void>(JavaMethodPtr method, void* /*result*/, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        env->CallStaticVoidMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticVoidMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<void>", ex.what());
        return false;
    }
}

// invokeV<jobject>
template <>
inline bool JavaClass::invokeV<jobject>(JavaMethodPtr method, jobject* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticObjectMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticObjectMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jobject>", ex.what());
        return false;
    }
}

// invokeV<jstring>
template <>
inline bool JavaClass::invokeV<jstring>(JavaMethodPtr method, jstring* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = (jstring)env->CallStaticObjectMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticObjectMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jstring>", ex.what());
        return false;
    }
}

// invokeV<jboolean>
template <>
inline bool JavaClass::invokeV<jboolean>(JavaMethodPtr method, jboolean* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticBooleanMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticBooleanMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jboolean>", ex.what());
        return false;
    }
}

// invokeV<jbyte>
template <>
inline bool JavaClass::invokeV<jbyte>(JavaMethodPtr method, jbyte* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticByteMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticByteMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jbyte>", ex.what());
        return false;
    }
}

// invokeV<jchar>
template <>
inline bool JavaClass::invokeV<jchar>(JavaMethodPtr method, jchar* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticCharMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticCharMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jchar>", ex.what());
        return false;
    }
}

// invokeV<jshort>
template <>
inline bool JavaClass::invokeV<jshort>(JavaMethodPtr method, jshort* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticShortMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticShortMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jshort>", ex.what());
        return false;
    }
}

// invokeV<jint>
template <>
inline bool JavaClass::invokeV<jint>(JavaMethodPtr method, jint* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticIntMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticIntMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jint>", ex.what());
        return false;
    }
}

// invokeV<jlong>
template <>
inline bool JavaClass::invokeV<jlong>(JavaMethodPtr method, jlong* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticLongMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticLongMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jlong>", ex.what());
        return false;
    }
}

// invokeV<jfloat>
template <>
inline bool JavaClass::invokeV<jfloat>(JavaMethodPtr method, jfloat* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticFloatMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticFloatMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jfloat>", ex.what());
        return false;
    }
}

// invokeV<jdouble>
template <>
inline bool JavaClass::invokeV<jdouble>(JavaMethodPtr method, jdouble* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalClassRef.get(), "invalidClassReference");
        *result = env->CallStaticDoubleMethodV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallStaticDoubleMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaClass", "invokeV<jdouble>", ex.what());
        return false;
    }
}

}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_JAVA_CLASS_H
