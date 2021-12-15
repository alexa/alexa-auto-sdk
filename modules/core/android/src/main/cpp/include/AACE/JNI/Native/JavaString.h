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

#ifndef AACE_JNI_NATIVE_JAVA_STRING_H
#define AACE_JNI_NATIVE_JAVA_STRING_H

#include <string>
#include <jni.h>

#include "GlobalRef.h"

namespace aace {
namespace jni {
namespace native {

class JavaString {
public:
    JavaString();
    JavaString(jobject jstr);
    JavaString(jstring jstr);
    JavaString(const std::string& str);
    JavaString(const char* str);
    ~JavaString();

    jstring get();

    std::string toStdStr();
    const char* toCStr();

private:
    GlobalRef<jstring> m_ref;
    const char* m_cstr;
};

}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_JAVA_STRING_H
