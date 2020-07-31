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

#ifndef AACE_JNI_NATIVE_JAVA_ENUM_H
#define AACE_JNI_NATIVE_JAVA_ENUM_H

#include <jni.h>
#include <vector>
#include <string>
#include <memory>

#include "GlobalRef.h"
#include "NativeMacros.h"

namespace aace {
namespace jni {
namespace native {

template <class T>
class EnumConfiguration {
public:
    virtual const char* getClassName() = 0;
    virtual std::vector<std::pair<T, std::string>> getConfiguration() = 0;
};

template <class T, class C>
class JavaEnum {
public:
    static T from(jobject obj, T def) {
        try_with_context {
            ThrowIfNull(getInstance(), "invalidEnumInstance")

                for (EnumTypeInfo& next : getInstance()->getEnumTypes()) {
                if (env->IsSameObject(obj, std::get<2>(next).get())) {
                    return std::get<1>(next);
                }

                ThrowIfJavaEx(env, "IsSameObjectFailed");
            }

            AACE_JNI_WARN("aace.jni.native.JavaEnum", "from", "invalidEnumTypeUsingDefault");
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaEnum", "from", ex.what());
        }

        return def;
    }

    static bool checkType(T type, jobject* obj = nullptr) {
        try_with_context {
            ThrowIfNull(getInstance(), "invalidEnumInstance")

                for (EnumTypeInfo& next : getInstance()->getEnumTypes()) {
                if (std::get<1>(next) == type) {
                    if (obj != nullptr) {
                        *obj = std::get<2>(next).get();
                    }
                    return true;
                }
            }
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaEnum", "checkType", ex.what());
        }

        return false;
    }

    static bool checkType(jobject obj, T* type = nullptr) {
        try_with_context {
            ThrowIfNull(getInstance(), "invalidEnumInstance")

                for (EnumTypeInfo& next : getInstance()->getEnumTypes()) {
                if (env->IsSameObject(obj, std::get<2>(next).get())) {
                    if (type != nullptr) {
                        *type = std::get<1>(next);
                    }
                    return true;
                }

                ThrowIfJavaEx(env, "IsSameObjectFailed");
            }
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaEnum", "checkType", ex.what());
        }

        return false;
    }

private:
    using EnumTypeInfo = std::tuple<std::string, T, GlobalRef<jobject>>;

    JavaEnum<T, C>() {
        static_assert(std::is_base_of<EnumConfiguration<T>, C>::value, "Invalid EnumConfiguration class");

        try_with_context {
            C config;
            auto name = config.getClassName();
            auto enumClass = JavaClass::find(name);
            ThrowIfNull(enumClass, "invalidEnumClass");

            for (std::pair<T, std::string>& next : config.getConfiguration()) {
                jfieldID fieldId = env->GetStaticFieldID(
                    enumClass->get(), next.second.c_str(), std::string("L" + std::string(name) + ";").c_str());
                ThrowIfJavaEx(env, "GetStaticFieldIDFailed");

                jobject obj = env->GetStaticObjectField(enumClass->get(), fieldId);
                ThrowIfJavaEx(env, "GetStaticObjectFieldFailed");

                m_enumTypes.push_back(std::make_tuple(next.second, next.first, GlobalRef<jobject>(obj)));
            }
        }
        catch_with_ex {
            AACE_JNI_ERROR("aace.jni.native.JavaEnum", "initialize", ex.what());
        }
    }

    std::vector<EnumTypeInfo>& getEnumTypes() {
        return m_enumTypes;
    }

    static std::shared_ptr<JavaEnum<T, C>> getInstance() {
        if (s_instance == nullptr) {
            s_instance = std::shared_ptr<JavaEnum<T, C>>(new JavaEnum<T, C>());
        }

        return s_instance;
    }

private:
    std::vector<EnumTypeInfo> m_enumTypes;

    // singleton enum type instance
    static std::shared_ptr<JavaEnum<T, C>> s_instance;
};

template <class T, class C>
std::shared_ptr<JavaEnum<T, C>> JavaEnum<T, C>::s_instance;

}  // namespace native
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_NATIVE_JAVA_ENUM_H
