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

#include <AACE/JNI/Native/JavaField.h>
#include <AACE/JNI/Native/NativeMacros.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.native.JavaField";

namespace aace {
namespace jni {
namespace native {

JavaField::JavaField(const std::string& name, const std::string& signature, jfieldID fieldID) :
        m_name(name), m_signature(signature), m_fieldID(fieldID) {
}

std::shared_ptr<JavaField> JavaField::create(const char* name, const char* signature, jfieldID fieldID) {
    return std::shared_ptr<JavaField>(new JavaField(name, signature, fieldID));
}

jfieldID JavaField::getFieldID() {
    return m_fieldID;
}

}  // namespace native
}  // namespace jni
}  // namespace aace
