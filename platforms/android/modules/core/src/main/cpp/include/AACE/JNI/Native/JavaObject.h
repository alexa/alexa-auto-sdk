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

#ifndef AACE_JNI_NATIVE_JAVA_OBJECT_H
#define AACE_JNI_NATIVE_JAVA_OBJECT_H

#include "JavaClass.h"
#include "JavaString.h"

namespace aace {
namespace jni {
namespace native {

class JavaObject {
public:
    JavaObject() = default;
    JavaObject(jobject obj, const char* className);
    JavaObject(const char* className);

    JavaClassPtr getClass();
    JavaMethodPtr getMethod(const char* name, const char* signature);
    JavaFieldPtr getField(const char* name, const char* signature);
    JavaFieldPtr getStaticField(const char* name, const char* signature);

    // jobject reference accessor
    jobject get();

    // instance method invocation
    template <class T>
    bool invokeV(JavaMethodPtr method, T* result, va_list);
    template <class T>
    bool invoke(JavaMethodPtr method, T* result, ...);
    template <class T>
    bool invoke(const char* name, const char* signature, T* result, ...);

    template <class T>
    bool set(const char* name, const char* signature, T value);
    template <class T>
    bool set(const char* name, T value);
    template <class T>
    bool set(JavaFieldPtr field, T value);
    template <class T>
    bool get(const char* name, const char* signature, T* value);
    template <class T>
    bool get(const char* name, T* value);
    template <class T>
    bool get(JavaFieldPtr field, T* value);
    template <class T>
    bool getStatic(const char* name, const char* signature, T* value);
    template <class T>
    bool getStatic(const char* name, T* value);
    template <class T>
    bool getStatic(JavaFieldPtr field, T* value);

    // assignment opperator
    JavaObject& operator=(const JavaObject& jobj) {
        m_globalObjRef = jobj.m_globalObjRef;
        m_class = jobj.m_class;
        return *this;
    }

private:
    GlobalRef<jobject> m_globalObjRef;
    JavaClassPtr m_class;
};

template <class T>
bool JavaObject::invokeV(JavaMethodPtr method, T* result, va_list) {
    AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV", "undefinedTypeSpecification");
    return false;
}

template <class T>
bool JavaObject::invoke(JavaMethodPtr method, T* result, ...) {
    try_with_context {
        // start - handle vargs
        va_list args;
        va_start(args, result);

        // call the invoke method with va_list
        ThrowIfNot(invokeV<T>(method, result, args), "invokeVFailed");

        // end - handle vargs
        va_end(args);

        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invoke", ex.what());
        return false;
    }
}

template <class T>
bool JavaObject::invoke(const char* name, const char* signature, T* result, ...) {
    try_with_context {
        auto method = getMethod(name, signature);
        ThrowIfNull(method, "invalidMethod");

        // start - handle vargs
        va_list args;
        va_start(args, result);

        // call the invoke method with va_list
        ThrowIfNot(invokeV<T>(method, result, args), "invokeVFailed");

        // end - handle vargs
        va_end(args);

        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invoke", ex.what());
        return false;
    }
}

template <class T>
bool JavaObject::set(const char* name, const char* signature, T value) {
    try_with_context {
        auto field = getField(name, signature);
        ThrowIfNull(field, "invalidField");

        return set<T>(field, value);
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set", ex.what());
        return false;
    }
}

template <class T>
bool JavaObject::get(const char* name, const char* signature, T* value) {
    try_with_context {
        auto field = getField(name, signature);
        ThrowIfNull(field, "invalidField");

        return get<T>(field, value);
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get", ex.what());
        return false;
    }
}

template <class T>
bool JavaObject::getStatic(const char* name, const char* signature, T* value) {
    try_with_context {
        auto field = getStaticField(name, signature);
        ThrowIfNull(field, "invalidField");

        return getStatic<T>(field, value);
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "getStatic", ex.what());
        return false;
    }
}

// invokeV<void>
template <>
inline bool JavaObject::invokeV<void>(JavaMethodPtr method, void* /*result*/, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->CallVoidMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallVoidMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<void>", ex.what());
        return false;
    }
}

// invokeV<jobject>
template <>
inline bool JavaObject::invokeV<jobject>(JavaMethodPtr method, jobject* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallObjectMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallObjectMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jobject>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jobject>(JavaFieldPtr field, jobject value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetObjectField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetObjectFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jobject>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jobject>(JavaFieldPtr field, jobject* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetObjectField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetObjectFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jobject>", ex.what());
        return false;
    }
}

// invokeV<jstring>
template <>
inline bool JavaObject::invokeV<jstring>(JavaMethodPtr method, jstring* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = (jstring)env->CallObjectMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallObjectMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jstring>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jstring>(const char* name, jstring value) {
    return set<jobject>(name, "Ljava/lang/String;", (jobject)value);
}

template <>
inline bool JavaObject::get<jstring>(const char* name, jstring* result) {
    return get<jobject>(name, "Ljava/lang/String;", (jobject*)result);
}

template <>
inline bool JavaObject::set<std::string>(const char* name, std::string value) {
    return set<jstring>(name, "Ljava/lang/String;", JavaString(value).get());
}

template <>
inline bool JavaObject::get<std::string>(const char* name, std::string* result) {
    jstring strResult;
    ReturnIfNot(get<jstring>(name, "Ljava/lang/String;", &strResult), false);
    *result = strResult != nullptr ? JavaString(strResult).toStdStr() : "";
    return true;
}

template <>
inline bool JavaObject::set<jstring>(JavaFieldPtr field, jstring value) {
    return set<jobject>(field, (jobject)value);
}

template <>
inline bool JavaObject::get<jstring>(JavaFieldPtr field, jstring* result) {
    return get<jobject>(field, (jobject*)result);
}

// invokeV<jboolean>
template <>
inline bool JavaObject::invokeV<jboolean>(JavaMethodPtr method, jboolean* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallBooleanMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallBooleanMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jboolean>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jboolean>(const char* name, jboolean value) {
    return set<jboolean>(name, "Z", value);
}

template <>
inline bool JavaObject::get<jboolean>(const char* name, jboolean* result) {
    return get<jboolean>(name, "Z", result);
}

template <>
inline bool JavaObject::set<bool>(const char* name, bool value) {
    return set<jboolean>(name, "Z", static_cast<jboolean>(value));
}

template <>
inline bool JavaObject::get<bool>(const char* name, bool* result) {
    jboolean boolResult;
    ReturnIfNot(get<jboolean>(name, "Z", &boolResult), false);
    *result = static_cast<bool>(boolResult);
    return true;
}

template <>
inline bool JavaObject::set<jboolean>(JavaFieldPtr field, jboolean value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetBooleanField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetBooleanFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jboolean>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jboolean>(JavaFieldPtr field, jboolean* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetBooleanField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetBooleanFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jboolean>", ex.what());
        return false;
    }
}

// invokeV<jbyte>
template <>
inline bool JavaObject::invokeV<jbyte>(JavaMethodPtr method, jbyte* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallByteMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallByteMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jbyte>", ex.what());
        return false;
    }
}

// invokeV<jchar>
template <>
inline bool JavaObject::invokeV<jchar>(JavaMethodPtr method, jchar* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallCharMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallCharMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jchar>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jchar>(const char* name, jchar value) {
    return set<jchar>(name, "C", value);
}

template <>
inline bool JavaObject::get<jchar>(const char* name, jchar* result) {
    return get<jchar>(name, "C", result);
}

template <>
inline bool JavaObject::set<char>(const char* name, char value) {
    return set<jchar>(name, "C", static_cast<jchar>(value));
}

template <>
inline bool JavaObject::get<char>(const char* name, char* result) {
    jchar charResult;
    ReturnIfNot(get<jchar>(name, "C", &charResult), false);
    *result = static_cast<char>(charResult);
    return true;
}

template <>
inline bool JavaObject::set<jchar>(JavaFieldPtr field, jchar value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetCharField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetCharFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jchar>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jchar>(JavaFieldPtr field, jchar* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetCharField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetCharFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jchar>", ex.what());
        return false;
    }
}

// invokeV<jshort>
template <>
inline bool JavaObject::invokeV<jshort>(JavaMethodPtr method, jshort* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallShortMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallShortMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jshort>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jshort>(const char* name, jshort value) {
    return set<jshort>(name, "S", value);
}

template <>
inline bool JavaObject::get<jshort>(const char* name, jshort* result) {
    return get<jshort>(name, "S", result);
}

template <>
inline bool JavaObject::set<jshort>(JavaFieldPtr field, jshort value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetShortField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetShortFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jshort>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jshort>(JavaFieldPtr field, jshort* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetShortField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetShortFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jshort>", ex.what());
        return false;
    }
}

// invokeV<jint>
template <>
inline bool JavaObject::invokeV<jint>(JavaMethodPtr method, jint* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallIntMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallIntMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jint>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jint>(const char* name, jint value) {
    return set<jint>(name, "I", value);
}

template <>
inline bool JavaObject::get<jint>(const char* name, jint* result) {
    return get<jint>(name, "I", result);
}

template <>
inline bool JavaObject::set<jint>(JavaFieldPtr field, jint value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetIntField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetIntFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jint>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jint>(JavaFieldPtr field, jint* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetIntField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetIntFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jint>", ex.what());
        return false;
    }
}

// invokeV<jlong>
template <>
inline bool JavaObject::invokeV<jlong>(JavaMethodPtr method, jlong* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallLongMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallLongMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jlong>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jlong>(const char* name, jlong value) {
    return set<jlong>(name, "J", value);
}

template <>
inline bool JavaObject::get<jlong>(const char* name, jlong* result) {
    return get<jlong>(name, "J", result);
}

/* TODO: compiler issues with Android64
    template <>
    inline bool JavaObject::set<long>( const char* name, long value ) {
        return set<jlong>( name, "J", static_cast<jlong>( value ) );
    }

    template <>
    inline bool JavaObject::get<long>( const char* name, long* result )
    {
        jlong longResult;
        ReturnIfNot( get<jlong>( name, "J", &longResult ), false );
        *result = static_cast<long>( longResult );
        return true;
    }
*/

template <>
inline bool JavaObject::set<jlong>(JavaFieldPtr field, jlong value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetLongField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetLongFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jlong>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jlong>(JavaFieldPtr field, jlong* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetLongField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetLongFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jlong>", ex.what());
        return false;
    }
}

// invokeV<jfloat>
template <>
inline bool JavaObject::invokeV<jfloat>(JavaMethodPtr method, jfloat* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallFloatMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallFloatMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jfloat>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jfloat>(const char* name, jfloat value) {
    return set<jfloat>(name, "F", value);
}

template <>
inline bool JavaObject::get<jfloat>(const char* name, jfloat* result) {
    return get<jfloat>(name, "F", result);
}

template <>
inline bool JavaObject::set<jfloat>(JavaFieldPtr field, jfloat value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetFloatField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetFloatFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jfloat>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jfloat>(JavaFieldPtr field, jfloat* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetFloatField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetFloatFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jfloat>", ex.what());
        return false;
    }
}

// invokeV<jdouble>
template <>
inline bool JavaObject::invokeV<jdouble>(JavaMethodPtr method, jdouble* result, va_list args) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->CallDoubleMethodV(m_globalObjRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "CallDoubleMethodVFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "invokeV<jdouble>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::set<jdouble>(const char* name, jdouble value) {
    return set<jdouble>(name, "D", value);
}

template <>
inline bool JavaObject::get<jdouble>(const char* name, jdouble* result) {
    return get<jdouble>(name, "D", result);
}

template <>
inline bool JavaObject::getStatic<jdouble>(const char* name, jdouble* result) {
    return getStatic<jdouble>(name, "D", result);
}

template <>
inline bool JavaObject::set<jdouble>(JavaFieldPtr field, jdouble value) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        env->SetDoubleField(m_globalObjRef.get(), field->getFieldID(), value);
        ThrowIfJavaEx(env, "SetDoubleFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "set<jdouble>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::get<jdouble>(JavaFieldPtr field, jdouble* result) {
    try_with_context {
        ThrowIfNull(m_globalObjRef.get(), "invalidObjectReference");
        *result = env->GetDoubleField(m_globalObjRef.get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetDoubleFieldFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "get<jdouble>", ex.what());
        return false;
    }
}

template <>
inline bool JavaObject::getStatic<jdouble>(JavaFieldPtr field, jdouble* result) {
    try_with_context {
        *result = env->GetStaticDoubleField(m_class->get(), field->getFieldID());
        ThrowIfJavaEx(env, "GetStaticDoubleFailed");
        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR("aace.jni.native.JavaObject", "getStatic<jdouble>", ex.what());
        return false;
    }
}
}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_JAVA_OBJECT_H
