/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ANDROID_SDK_NATIVE_LIB_H
#define AACE_ANDROID_SDK_NATIVE_LIB_H

#include <string>
#include <unordered_map>
#include <jni.h>

class ThreadContext {
public:
    ThreadContext();
    ~ThreadContext();

    bool isValid();

    JNIEnv* getEnv();

private:
    JNIEnv* m_env;
    bool m_detatch;
};

template <class T>
class GlobalRef {
public:
    GlobalRef() : m_globalRef( nullptr ) {}

    GlobalRef( JNIEnv* env, T obj ) : m_globalRef( nullptr ) {
        if( env != nullptr && obj != nullptr ) {
            m_globalRef = (T) env->NewGlobalRef( obj );
        }
    }

    GlobalRef( const GlobalRef<T>& ref ) : m_globalRef( nullptr ) {
        ThreadContext context;
        if( context.isValid() ) {
            m_globalRef = (T) context.getEnv()->NewGlobalRef( ref.m_globalRef );
        }
    }

    ~GlobalRef() {
        if( m_globalRef != nullptr ) {
            ThreadContext context;
            if( context.isValid() ) {
                context.getEnv()->DeleteGlobalRef( m_globalRef );
                m_globalRef = nullptr;
            }
        }
    }

public:
    T get() {
        return m_globalRef;
    }

    bool isInstanceOf( JNIEnv* env, jclass javaClass ) {
        return (m_globalRef != nullptr && env != nullptr) ? env->IsInstanceOf( m_globalRef, javaClass ) : false;
    }

    bool isSameObject( JNIEnv* env, T obj ) {
        return (m_globalRef != nullptr && env != nullptr) ? env->IsSameObject( m_globalRef, obj ) : false;
    }

    // assignment opperator
    GlobalRef<T>& operator= (const GlobalRef<T> &ref) {
        ThreadContext context;
        m_globalRef = (ref.m_globalRef != nullptr && context.isValid()) ? (T) context.getEnv()->NewGlobalRef( ref.m_globalRef ) : nullptr;
        return *this;
    }

private:
    T m_globalRef;
};

using ClassRef = GlobalRef<jclass>;
using ObjectRef = GlobalRef<jobject>;

class NativeLib {
public:
    static std::string convert( JNIEnv *env, jstring jstr );
    static jstring convert( JNIEnv *env, std::string );
    static ObjectRef FindEnum( JNIEnv* env, jclass enumClass, const char* fieldName, const char* type );
    static ClassRef FindClass( JNIEnv* env, const char* className );
};

#endif //AACE_ANDROID_SDK_NATIVE_LIB_H
