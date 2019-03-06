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

// aace/alexa/TemplateRuntimeBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/TemplateRuntimeBinder.h"

void TemplateRuntimeBinder::initialize( JNIEnv* env ) {
    m_javaMethod_renderTemplate_payload = env->GetMethodID( getJavaClass(), "renderTemplate", "(Ljava/lang/String;)V" );
    m_javaMethod_renderPlayerInfo_payload = env->GetMethodID( getJavaClass(), "renderPlayerInfo", "(Ljava/lang/String;)V" );
    m_javaMethod_clearTemplate = env->GetMethodID( getJavaClass(), "clearTemplate", "()V" );
    m_javaMethod_clearPlayerInfo = env->GetMethodID( getJavaClass(), "clearPlayerInfo", "()V" );
}

void TemplateRuntimeBinder::renderTemplate( const std::string & payload )
{
    if( getJavaObject() != nullptr && m_javaMethod_renderTemplate_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );

            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_renderTemplate_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
}

void TemplateRuntimeBinder::renderPlayerInfo( const std::string & payload )
{
    if( getJavaObject() != nullptr && m_javaMethod_renderPlayerInfo_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );

            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_renderPlayerInfo_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
}

void TemplateRuntimeBinder::clearTemplate()
{
    if( getJavaObject() != nullptr && m_javaMethod_clearTemplate != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_clearTemplate );
        }
    }
}

void TemplateRuntimeBinder::clearPlayerInfo()
{
    if( getJavaObject() != nullptr && m_javaMethod_clearPlayerInfo != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_clearPlayerInfo );
        }
    }
}

// JNI
#define TEMPLATERUNTIME(cptr) ((TemplateRuntimeBinder *) cptr)

extern "C" {

}

// END OF FILE
