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

#ifndef AACE_JNI_NATIVE_JAVA_METHOD_H
#define AACE_JNI_NATIVE_JAVA_METHOD_H

#include <jni.h>

#include <string>
#include <memory>

namespace aace {
namespace jni {
namespace native {

class JavaMethod {
private:
    JavaMethod(const std::string& name, const std::string& signature, jmethodID methodID);

public:
    static std::shared_ptr<JavaMethod> create(const char* name, const char* signature, jmethodID methodID);

    jmethodID getMethodID();

private:
    std::string m_name;
    std::string m_signature;
    jmethodID m_methodID;
};

using JavaMethodPtr = std::shared_ptr<JavaMethod>;

//
// JavaMethodInfo
//

class JavaMethodInfo {
public:
    JavaMethodInfo() = default;
    JavaMethodInfo(const char* name, const char* signature, bool is_static = false) :
            m_id(name), m_name(name), m_signature(signature), m_static(is_static) {
    }
    JavaMethodInfo(const char* id, const char* name, const char* signature, bool is_static = false) :
            m_id(id), m_name(name), m_signature(signature), m_static(is_static) {
    }

    const std::string& getId() {
        return m_id;
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
    std::string m_id;
    std::string m_name;
    std::string m_signature;
    bool m_static;
};

}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_JAVA_METHOD_H
