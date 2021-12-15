/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_CORE_NATIVE_LIB_H
#define AACE_JNI_CORE_NATIVE_LIB_H

#include <jni.h>

#include <AACE/JNI/Native/ThreadContext.h>
#include <AACE/JNI/Native/GlobalRef.h>
#include <AACE/JNI/Native/JavaString.h>
#include <AACE/JNI/Native/JavaClass.h>
#include <AACE/JNI/Native/JavaObject.h>
#include <AACE/JNI/Native/JavaMethod.h>
#include <AACE/JNI/Native/JavaField.h>
#include <AACE/JNI/Native/JavaEnum.h>
#include <AACE/JNI/Native/JavaArray.h>
#include <AACE/JNI/Native/NativeMacros.h>

using ThreadContext = aace::jni::native::ThreadContext;

using ClassRef = aace::jni::native::GlobalRef<jclass>;
using ObjectRef = aace::jni::native::GlobalRef<jobject>;

using JString = aace::jni::native::JavaString;
using JObject = aace::jni::native::JavaObject;
using JClass = aace::jni::native::JavaClassPtr;
using JMethod = aace::jni::native::JavaMethodPtr;
using JField = aace::jni::native::JavaFieldPtr;

using JObjectArray = aace::jni::native::JavaObjectArray;
using JByteArray = aace::jni::native::JavaArray<jbyteArray, jbyte>;
using JLongArray = aace::jni::native::JavaArray<jlongArray, jlong>;
using JIntArray = aace::jni::native::JavaArray<jintArray, jint>;

template <class T, class C>
using JEnum = aace::jni::native::JavaEnum<T, C>;

template <class T>
using EnumConfiguration = aace::jni::native::EnumConfiguration<T>;

#endif  // AACE_JNI_CORE_NATIVE_LIB_H
