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

#ifndef AACE_JNI_NATIVE_JAVA_ARRAY_H
#define AACE_JNI_NATIVE_JAVA_ARRAY_H

#include <jni.h>

#include "GlobalRef.h"
#include "JavaClass.h"
#include "NativeMacros.h"

namespace aace {
namespace jni {
namespace native {

template <class T, class E>
class JavaArray {
public:
    JavaArray() : m_data(nullptr), m_size(0) {
    }

    explicit JavaArray(jsize size) : m_data(nullptr), m_size(size) {
        try_with_context {
            T arr = create(size);
            m_arr = arr;
            env->DeleteLocalRef(arr);
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "JavaArray()", ex.what());
        }
    }

    JavaArray(T arr) : m_data(nullptr), m_size(0) {
        set(arr);
    }

    virtual ~JavaArray() {
        if (m_arr.get() != nullptr && m_data != nullptr) {
            releaseElements(m_arr.get(), m_data);
        }
    }

    T get() {
        return m_arr.get();
    }

    jsize size() {
        return m_size;
    }

    bool isValid() {
        return m_arr.get() != nullptr;
    }

    E* ptr(int idx = 0) {
        try {
            ThrowIf(idx < 0 || idx >= m_size, "indexOutOfRange");
            ThrowIfNot(lock(), "lockArrayDataFailed");
            ThrowIfNull(m_data, "invalidArrayData");

            return m_data + idx;
        } catch (const std::exception& ex) {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "ptr", ex.what());
            return nullptr;
        }
    }

    bool copyTo(jsize idx, jsize len, E* src) {
        try {
            ThrowIf(idx < 0 || idx >= m_size, "startIndexOutOfRange");
            ThrowIf((idx + len) > m_size, "endIndexOutOfRange");
            ThrowIfNull(src, "invalidSource");
            return setRegion(idx, len, src);
        } catch (const std::exception& ex) {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "copyTo", ex.what());
            return false;
        }
    }

    virtual bool getAt(int index, E* value) {
        try {
            ThrowIfNot(index < m_size && index >= 0, "indexOutOfRange");
            ThrowIfNot(lock(), "lockArrayDataFailed");
            ThrowIfNull(m_data, "invalidArrayData");

            *value = m_data[index];

            return true;
        } catch (const std::exception& ex) {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "get", ex.what());
            return false;
        }
    }

    virtual bool setAt(int index, const E value) {
        try {
            ThrowIfNot(index < m_size && index >= 0, "indexOutOfRange");
            ThrowIfNot(lock(), "lockArrayDataFailed");
            ThrowIfNull(m_data, "invalidArrayData");

            m_data[index] = value;

            return true;
        } catch (const std::exception& ex) {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "set", ex.what());
            return false;
        }
    }

protected:
    bool set(T arr) {
        try_with_context {
            ThrowIfNotNull(m_arr.get(), "arrayIsAlreadySet");

            m_arr = arr;
            m_size = env->GetArrayLength(m_arr.get());
            ThrowIfJavaEx(env, "getArrayLengthFailed");

            return true;
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "set", ex.what());
            return false;
        }
    }

    virtual bool lock() {
        ReturnIf(m_data != nullptr, true);

        try_with_context {
            m_data = getElements(m_arr.get());
            ThrowIfNull(m_data, "getArrayElementsFailed");
            return true;
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaArray", "lock", ex.what());
            return false;
        }
    }

    T create(jsize size) {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "create", "unimplementedMethodInClass");
        return nullptr;
    }

    E* getElements(T arr) {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "getElements", "unimplementedMethodInClass");
        return nullptr;
    }

    void releaseElements(T arr, E* data) {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "releaseElements", "unimplementedMethodInClass");
    }

    bool setRegion(jsize idx, jsize len, E* src) {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "setRegion", "unimplementedMethodInClass");
        return false;
    }

private:
    GlobalRef<T> m_arr;
    E* m_data;
    jsize m_size;
};

// JavaArray<jbyteArray,jbyte>
template <>
inline void JavaArray<jbyteArray, jbyte>::releaseElements(jbyteArray arr, jbyte* data){
    try_with_context{env->ReleaseByteArrayElements(arr, data, 0);
ThrowIfJavaEx(env, "ReleaseByteArrayElementsFailed");
}  // namespace native
catch_with_ex {
    AACE_JNI_ERROR("aace.jni.native.JavaArray", "releaseElements<jbyteArray,jbyte>", ex.what());
}
};  // namespace jni

template <>
inline jbyte* JavaArray<jbyteArray, jbyte>::getElements(jbyteArray arr) {
    try_with_context {
        jbyte* data = env->GetByteArrayElements(arr, nullptr);
        ThrowIfJavaEx(env, "GetByteArrayElementsFailed");
        return data;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "getElements<jbyteArray,jbyte>", ex.what());
        return nullptr;
    }
}

template <>
inline jbyteArray JavaArray<jbyteArray, jbyte>::create(jsize size){
    try_with_context{jbyteArray arr = env->NewByteArray(size);
ThrowIfJavaEx(env, "NewByteArrayFailed");
return arr;
}  // namespace aace
catch_with_ex {
    AACE_JNI_ERROR("aace.jni.native.JavaArray", "create<jbyteArray,jbyte>", ex.what());
    return nullptr;
}
}
;

template <>
inline bool JavaArray<jbyteArray, jbyte>::setRegion(jsize idx, jsize len, jbyte* src) {
    try_with_context {
        env->SetByteArrayRegion(m_arr.get(), idx, len, src);
        ThrowIfJavaEx(env, "SetByteArrayRegionFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "setRegion<jbyteArray,jbyte>", ex.what());
        return false;
    }
}

// JavaArray<jlongArray,jlong>
template <>
inline void JavaArray<jlongArray, jlong>::releaseElements(jlongArray arr, jlong* data){
    try_with_context{env->ReleaseLongArrayElements(arr, data, 0);
ThrowIfJavaEx(env, "ReleaseLongArrayElementsFailed");
}
catch_with_ex {
    AACE_JNI_ERROR("aace.jni.native.JavaArray", "releaseElements<jlongArray,jlong>", ex.what());
}
}
;

template <>
inline jlong* JavaArray<jlongArray, jlong>::getElements(jlongArray arr) {
    try_with_context {
        jlong* data = env->GetLongArrayElements(arr, nullptr);
        ThrowIfJavaEx(env, "GetLongArrayElementsFailed");
        return data;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "getElements<jlongArray,jlong>", ex.what());
        return nullptr;
    }
}

template <>
inline jlongArray JavaArray<jlongArray, jlong>::create(jsize size) {
    try_with_context {
        jlongArray arr = env->NewLongArray(size);
        ThrowIfJavaEx(env, "NewLongArrayFailed");
        return arr;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "create<jlongArray,jlong>", ex.what());
        return nullptr;
    }
}

// JavaArray<jintArray,jint>
template <>
inline void JavaArray<jintArray, jint>::releaseElements(jintArray arr, jint* data){
    try_with_context{env->ReleaseIntArrayElements(arr, data, 0);
ThrowIfJavaEx(env, "ReleaseIntArrayElementsFailed");
}
catch_with_ex {
    AACE_JNI_ERROR("aace.jni.native.JavaArray", "releaseElements<jintArray,jint>", ex.what());
}
}
;

template <>
inline jint* JavaArray<jintArray, jint>::getElements(jintArray arr) {
    try_with_context {
        jint* data = env->GetIntArrayElements(arr, nullptr);
        ThrowIfJavaEx(env, "GetIntArrayElementsFailed");
        return data;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "getElements<jintArray,jint>", ex.what());
        return nullptr;
    }
}

template <>
inline jintArray JavaArray<jintArray, jint>::create(jsize size) {
    try_with_context {
        jintArray arr = env->NewIntArray(size);
        ThrowIfJavaEx(env, "NewIntArrayFailed");
        return arr;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaArray", "create<jintArray,jint>", ex.what());
        return nullptr;
    }
}

//
// JavaObjectArray
//

class JavaObjectArray : public JavaArray<jobjectArray, jobject> {
public:
    JavaObjectArray(jsize size, JavaClassPtr elementClass, jobject initialValue = nullptr);
    JavaObjectArray(jsize size, const char* className, jobject initialValue = nullptr);
    JavaObjectArray(jobjectArray arr) : JavaArray<jobjectArray, jobject>(arr) {
    }

    JavaObjectArray(jsize size) : JavaArray<jobjectArray, jobject>() {
        AACE_JNI_ERROR("aace.jni.native.JavaObjectArray", "JavaObjectArray", "createObjectArrayFailed");
    }

    bool getAt(int index, jobject* value) override;
    bool setAt(int index, const jobject value) override;

protected:
    bool lock() override {
        return true;
    }
};

}  // aace::jni::native
}  // aace::jni
}  // aace

#endif  // AACE_JNI_NATIVE_JAVA_ARRAY_H
