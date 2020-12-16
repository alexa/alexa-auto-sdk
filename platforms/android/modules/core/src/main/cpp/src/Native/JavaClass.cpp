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

#include <AACE/JNI/Native/JavaClass.h>
#include <AACE/JNI/Native/JavaObject.h>
#include <AACE/JNI/Native/JavaString.h>
#include <AACE/JNI/Native/NativeMacros.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.native.JavaClass";

namespace aace {
namespace jni {
namespace native {

// Initialize static variable
std::mutex JavaClass::s_mutex;

std::unordered_map<std::string, JavaClassPtr> JavaClass::s_javaClassRegistry;
GlobalRef<jobject> JavaClass::s_classLoaderObjectRef;
jmethodID JavaClass::s_findClassMethodID;

JavaClass::JavaClass(const char* name, jclass cls) : m_name(name), m_globalClassRef(cls) {
}

bool JavaClass::initializeClassLoader() {
    try_with_context {
        ThrowIfNotNull(s_classLoaderObjectRef.get(), "classLoaderAlreadyInitialized");

        jclass engineClass = env->FindClass("com/amazon/aace/core/Engine");
        ThrowIfJavaEx(env, "FindClassFailed");

        jclass classClass = env->GetObjectClass(engineClass);
        ThrowIfJavaEx(env, "GetObjectClassFailed");

        jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
        ThrowIfJavaEx(env, "FindClassFailed");

        auto getClassLoaderMethod = env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
        ThrowIfJavaEx(env, "GetMethodIDFailed");

        auto classLoader = env->CallObjectMethod(engineClass, getClassLoaderMethod);
        ThrowIfJavaEx(env, "CallObjectMethodFailed");

        auto findClassMethod = env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        ThrowIfJavaEx(env, "GetMethodIDFailed");

        // set the static class loader instance and method
        s_classLoaderObjectRef = classLoader;
        s_findClassMethodID = findClassMethod;

        return true;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "initializeClassLoader", ex.what());
        return false;
    }
}

std::shared_ptr<JavaClass> JavaClass::find(const char* name) {
    try_with_context {
        // lock to prevent loading same class name multiple times
        std::lock_guard<std::mutex> lock(s_mutex);

        // check if class is already loaded
        auto it = s_javaClassRegistry.find(name);

        // if we found the class in the registry then return it
        ReturnIf(it != s_javaClassRegistry.end(), (*it).second);

        // attempt to find the class in the java classpath
        AACE_JNI_INFO(TAG, "find", LS("Loading Java class: " + std::string(name)));

        // validate that the class loader was initialized
        ThrowIfNull(s_classLoaderObjectRef.get(), "invalidClassLoader");

        // find the class using the java class loader
        jclass cls =
            (jclass)env->CallObjectMethod(s_classLoaderObjectRef.get(), s_findClassMethodID, JavaString(name).get());
        ThrowIfJavaEx(env, "CallObjectMethodFailed");
        ThrowIfNull(cls, "invalidJavaClass");

        // add the class to the registry
        auto javaClass = std::shared_ptr<JavaClass>(new JavaClass(name, cls));
        s_javaClassRegistry[name] = javaClass;

        return javaClass;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "find", ex.what());
        return nullptr;
    }
}

JavaMethodPtr JavaClass::getStaticMethod(const char* name, const char* signature) {
    try_with_context {
        std::string key = "static_" + std::string(name) + signature;
        auto it = m_methodMap.find(key);

        // if we found the method in the map then return it
        ReturnIf(it != m_methodMap.end(), (*it).second);

        // attempt to find the method in the referenced java class
        jmethodID methodID = env->GetStaticMethodID(m_globalClassRef.get(), name, signature);
        ThrowIfJavaEx(env, "GetStaticMethodIDFailed");

        // add the method to the map
        auto javaMethod = JavaMethod::create(name, signature, methodID);
        m_methodMap[key] = javaMethod;

        return javaMethod;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getStaticMethod", ex.what());
        return nullptr;
    }
}

JavaMethodPtr JavaClass::getMethod(const char* name, const char* signature) {
    try_with_context {
        std::string key = std::string(name) + signature;
        auto it = m_methodMap.find(key);

        // if we found the method in the map then return it
        ReturnIf(it != m_methodMap.end(), (*it).second);

        // attempt to find the method in the referenced java class
        jmethodID methodID = env->GetMethodID(m_globalClassRef.get(), name, signature);
        ThrowIfJavaEx(env, "GetMethodID");

        // add the method to the map
        auto javaMethod = JavaMethod::create(name, signature, methodID);
        m_methodMap[key] = javaMethod;

        return javaMethod;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getStaticMethod", ex.what());
        return nullptr;
    }
}

JavaFieldPtr JavaClass::getStaticField(const char* name, const char* signature) {
    try_with_context {
        std::string key = "static_" + std::string(name) + signature;
        auto it = m_fieldMap.find(key);

        // if we found the field in the map then return it
        ReturnIf(it != m_fieldMap.end(), (*it).second);

        // attempt to find the field in the referenced java class
        jfieldID fieldID = env->GetStaticFieldID(m_globalClassRef.get(), name, signature);
        ThrowIfJavaEx(env, "GetStaticFieldIDFailed");

        // add the field to the map
        auto javaField = JavaField::create(name, signature, fieldID);
        m_fieldMap[key] = javaField;

        return javaField;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getStaticField", ex.what());
        return nullptr;
    }
}

JavaFieldPtr JavaClass::getField(const char* name, const char* signature) {
    try_with_context {
        std::string key = std::string(name) + signature;
        auto it = m_fieldMap.find(key);

        // if we found the field in the map then return it
        ReturnIf(it != m_fieldMap.end(), (*it).second);

        // attempt to find the field in the referenced java class
        jfieldID fieldID = env->GetFieldID(m_globalClassRef.get(), name, signature);
        ThrowIfJavaEx(env, "GetFieldIDFailed");

        // add the field to the map
        auto javaField = JavaField::create(name, signature, fieldID);
        m_fieldMap[key] = javaField;

        return javaField;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getField", ex.what());
        return nullptr;
    }
}

bool JavaClass::isObjectInstance(jobject obj) {
    try_with_context {
        bool result = env->IsInstanceOf(obj, m_globalClassRef.get());
        ThrowIfJavaEx(env, "IsInstanceOfFailed");

        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "isObjectInstance", ex.what());
        return false;
    }
}

jclass JavaClass::get() {
    return m_globalClassRef.get();
}

//
// invoke method interface
//

template <class T>
bool JavaClass::invokeV(JavaMethodPtr method, T* result, va_list) {
    AACE_JNI_ERROR("TAG", "invokeV", "undefinedTypeSpecification");
    return false;
}

template <class T>
bool JavaClass::invoke(JavaMethodPtr method, T* result, ...) {
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
        AACE_JNI_ERROR("TAG", "invoke", ex.what());
        return false;
    }
}

jobject JavaClass::newInstance(const char* signature, ...) {
    try_with_context {
        auto method = getMethod("<init>", signature);
        ThrowIfNull(method, "invalidObjectInitializerType");

        // start - handle vargs
        va_list args;
        va_start(args, signature);

        // call the invoke method with va_list
        jobject obj = env->NewObjectV(m_globalClassRef.get(), method->getMethodID(), args);
        ThrowIfJavaEx(env, "NewObjectVFailed");

        // end - handle vargs
        va_end(args);

        return obj;
    }
    catch_with_ex {
        AACE_JNI_ERROR("TAG", "newInstance", ex.what());
        return nullptr;
    }
}

}  // namespace native
}  // namespace jni
}  // namespace aace
