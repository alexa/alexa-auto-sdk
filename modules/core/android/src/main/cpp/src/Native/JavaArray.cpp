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

#include <AACE/JNI/Native/JavaArray.h>
#include <AACE/JNI/Native/NativeMacros.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.native.JavaArray";

namespace aace {
namespace jni {
namespace native {

//
// JavaObjectArray
//

JavaObjectArray::JavaObjectArray(jsize size, JavaClassPtr elementClass, jobject initialValue) {
    try_with_context {
        jobjectArray arr = env->NewObjectArray(size, elementClass->get(), initialValue);
        ThrowIfJavaEx(env, "newObjectArrayFailed");
        ThrowIfNot(set(arr), "setFailed");
        // delete local array ref
        env->DeleteLocalRef(arr);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaObjectArray", ex.what());
    }
}

JavaObjectArray::JavaObjectArray(jsize size, const char* className, jobject initialValue) {
    try_with_context {
        JavaClassPtr elementClass = JavaClass::find(className);
        ThrowIfNull(elementClass, "invalidClassName");

        jobjectArray arr = env->NewObjectArray(size, elementClass->get(), initialValue);
        ThrowIfJavaEx(env, "newObjectArrayFailed");
        ThrowIfNot(set(arr), "setFailed");
        // delete local array ref
        env->DeleteLocalRef(arr);
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaObjectArray", ex.what());
    }
}

bool JavaObjectArray::getAt(int index, jobject* value) {
    try_with_context {
        ThrowIfNot(index < size() && index >= 0, "indexOutOfRange");

        *value = env->GetObjectArrayElement(get(), index);
        ThrowIfJavaEx(env, "getObjectArrayElementFailed");

        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaObjectArray:getAt", ex.what());
        return false;
    }
}

bool JavaObjectArray::setAt(int index, const jobject value) {
    try_with_context {
        ThrowIfNot(index < size() && index >= 0, "indexOutOfRange");

        env->SetObjectArrayElement(get(), index, value);
        ThrowIfJavaEx(env, "setObjectArrayElementFailed");

        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "JavaObjectArray:setAt", ex.what());
        return false;
    }
}

}  // namespace native
}  // namespace jni
}  // namespace aace
