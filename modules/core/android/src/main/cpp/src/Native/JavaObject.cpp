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

#include <AACE/JNI/Native/JavaObject.h>
#include <AACE/JNI/Native/NativeMacros.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.native.JavaObject";

namespace aace {
namespace jni {
namespace native {

JavaObject::JavaObject(jobject obj, const char* className) {
    try_with_context {
        m_class = JavaClass::find(className);
        ThrowIfNull(m_class, "invalidJavaClass");

        // sanity check the obj to make sure it is the specified type
        ThrowIfNot(m_class->isObjectInstance(obj), "invalidObjectInstanceForClass");

        // assign the global object ref
        m_globalObjRef = obj;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaObject", ex.what());
    }
}

JavaObject::JavaObject(const char* className) {
    try_with_context {
        m_class = JavaClass::find(className);
        ThrowIfNull(m_class, "invalidJavaClass");

        // attempt to create a new object with the default constructor
        jobject obj = m_class->newInstance("()V");
        ThrowIfNull(obj, "createNewObjectInstanceFailed");

        // assign the global object ref
        m_globalObjRef = obj;

        // delete local object ref
        env->DeleteLocalRef(obj);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaObject", ex.what());
    }
}

std::shared_ptr<JavaClass> JavaObject::getClass() {
    return m_class;
}

JavaMethodPtr JavaObject::getMethod(const char* name, const char* signature) {
    try_with_context {
        ThrowIfNull(m_class, "invalidJavaClass");
        return m_class->getMethod(name, signature);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getMethod", ex.what());
        return nullptr;
    }
}

JavaFieldPtr JavaObject::getField(const char* name, const char* signature) {
    try_with_context {
        ThrowIfNull(m_class, "invalidJavaClass");
        return m_class->getField(name, signature);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getField", ex.what());
        return nullptr;
    }
}

JavaFieldPtr JavaObject::getStaticField(const char* name, const char* signature) {
    try_with_context {
        ThrowIfNull(m_class, "invalidJavaClass");
        return m_class->getStaticField(name, signature);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getStaticField", ex.what());
        return nullptr;
    }
}

jobject JavaObject::get() {
    return m_globalObjRef.get();
}

}  // namespace native
}  // namespace jni
}  // namespace aace
