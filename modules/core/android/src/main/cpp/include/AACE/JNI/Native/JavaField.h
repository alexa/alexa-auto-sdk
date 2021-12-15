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

#ifndef AACE_JNI_NATIVE_JAVA_FIELD_H
#define AACE_JNI_NATIVE_JAVA_FIELD_H

#include <jni.h>

#include <string>
#include <memory>

namespace aace {
namespace jni {
namespace native {

class JavaField {
private:
    JavaField(const std::string& name, const std::string& signature, jfieldID fieldID);

public:
    static std::shared_ptr<JavaField> create(const char* name, const char* signature, jfieldID fieldID);

    jfieldID getFieldID();

private:
    std::string m_name;
    std::string m_signature;
    jfieldID m_fieldID;
};

using JavaFieldPtr = std::shared_ptr<JavaField>;

//
// JavaFieldInfo
//

class JavaFieldInfo {
public:
    JavaFieldInfo() = default;
    JavaFieldInfo(const char* name, const char* signature, bool is_static = false) :
            m_name(name), m_signature(signature), m_static(is_static) {
    }

    const std::string& getName() {
        return m_name;
    }

    const std::string& getSignature() {
        return m_signature;
    }

    bool isStatic() {
        return m_static;
    }

private:
    std::string m_name;
    std::string m_signature;
    bool m_static;
};

}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_JAVA_FIELD_H
