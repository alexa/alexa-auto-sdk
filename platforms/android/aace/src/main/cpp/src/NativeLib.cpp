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

#include "NativeLib.h"

// global reference to the jvm
JavaVM* g_javaVM = nullptr;

std::string NativeLib::convert( JNIEnv *env, jstring jstr )
{
    if( env != nullptr && jstr != nullptr )
    {
        const char *cstr = env->GetStringUTFChars( jstr, nullptr );
        std::string result = std::string( cstr );

        // release the Java string and UTF-8
        env->ReleaseStringUTFChars( jstr, cstr );

        return result;
    }
    else {
        return std::string();
    }
}

jstring NativeLib::convert( JNIEnv *env, std::string str )
{
    if( env != nullptr )
    {
        return env->NewStringUTF( str.c_str() );
    }
    else {
        return jstring();
    }
}

ObjectRef NativeLib::FindEnum( JNIEnv* env, jclass enumClass, const char* fieldName, const char* type )
{
    if( env != nullptr )
    {
        jfieldID fieldId = env->GetStaticFieldID( enumClass, fieldName, type );
        jobject obj = env->GetStaticObjectField( enumClass, fieldId );

        return ObjectRef( env, obj );
    }
    else {
        return ObjectRef();
    }
}

ClassRef NativeLib::FindClass( JNIEnv* env, const char* className )
{
    if( env != nullptr )
    {
        jclass cls = env->FindClass( className );
        if( env->ExceptionCheck() ) {
            env->ExceptionClear();
            return ClassRef();
        }
        else {
            return ClassRef(env, cls);
        }
    }
    else {
        return ClassRef();
    }
}

//
// ThreadContext
//

ThreadContext::ThreadContext() : m_env( nullptr ), m_detatch( false )
{
    if( g_javaVM != nullptr )
    {
        m_detatch = g_javaVM->GetEnv( (void**) &m_env, JNI_VERSION_1_6 ) == JNI_EDETACHED;

        if( m_detatch ) {
            g_javaVM->AttachCurrentThread( &m_env, nullptr );
        }
    }
}

ThreadContext::~ThreadContext()
{
    if( g_javaVM != nullptr && m_env != nullptr && m_detatch )
    {
        g_javaVM->DetachCurrentThread();
    }
}

JNIEnv* ThreadContext::getEnv() {
    return m_env;
}

bool ThreadContext::isValid() {
    return m_env != nullptr;
}

extern "C" {
jint JNI_OnLoad( JavaVM* vm, void* reserved )
{
    JNIEnv* env;

    if( vm->GetEnv( reinterpret_cast<void**>( &env ), JNI_VERSION_1_6 ) != JNI_OK ) {
        return -1;
    }

    // save the jvm reference
    g_javaVM = vm;

    return JNI_VERSION_1_6;
}

void JNI_OnUnload( JavaVM *vm, void *reserved ) {
}
}
