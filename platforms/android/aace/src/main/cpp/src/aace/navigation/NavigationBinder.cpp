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

// aace/navigation/NavigationBinder.cpp
// This is an automatically generated file.

#include "aace/navigation/NavigationBinder.h"

void NavigationBinder::initialize( JNIEnv* env )
{
    m_javaMethod_setDestination_payload = env->GetMethodID( getJavaClass(), "setDestination", "(Ljava/lang/String;)Z" );
    m_javaMethod_cancelNavigation = env->GetMethodID( getJavaClass(), "cancelNavigation", "()Z" );

}

bool NavigationBinder::setDestination( const std::string & payload )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_setDestination_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setDestination_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
    return result;
}

bool NavigationBinder::cancelNavigation()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_cancelNavigation != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_cancelNavigation );
        }
    }
    return result;
}

// JNI
#define NAVIGATION(cptr) ((NavigationBinder *) cptr)

extern "C" {

}

// END OF FILE
