/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/alexa/SpeakerBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/SpeakerBinder.h"

void SpeakerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_setVolume_volume = env->GetMethodID( getJavaClass(), "setVolume", "(B)Z" );
    m_javaMethod_adjustVolume_delta = env->GetMethodID( getJavaClass(), "adjustVolume", "(B)Z" );
    m_javaMethod_setMute_mute = env->GetMethodID( getJavaClass(), "setMute", "(Z)Z" );
    m_javaMethod_getVolume = env->GetMethodID( getJavaClass(), "getVolume", "()B" );
    m_javaMethod_isMuted = env->GetMethodID( getJavaClass(), "isMuted", "()Z" );

    // Type
    jclass typeEnumClass = env->FindClass( "com/amazon/aace/alexa/Speaker$Type" );
    m_enum_Type_AVS_SYNCED = NativeLib::FindEnum( env, typeEnumClass, "AVS_SYNCED", "Lcom/amazon/aace/alexa/Speaker$Type;" );
    m_enum_Type_LOCAL = NativeLib::FindEnum( env, typeEnumClass, "LOCAL", "Lcom/amazon/aace/alexa/Speaker$Type;" );
}

bool SpeakerBinder::setVolume( int8_t volume )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_setVolume_volume != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setVolume_volume, volume );
        }
    }
    return result;
}

bool SpeakerBinder::adjustVolume( int8_t delta )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_adjustVolume_delta != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_adjustVolume_delta, delta );
        }
    }
    return result;
}

bool SpeakerBinder::setMute( bool mute )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_setMute_mute != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setMute_mute, mute );
        }
    }
    return result;
}

int8_t SpeakerBinder::getVolume()
{
    int8_t result = 0;
    if( getJavaObject() != nullptr && m_javaMethod_getVolume != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallByteMethod( getJavaObject(), m_javaMethod_getVolume );
        }
    }
    return result;
}

bool SpeakerBinder::isMuted()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_isMuted != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_isMuted );
        }
    }
    return result;
}

jobject SpeakerBinder::convert( aace::alexa::Speaker::Type type )
{
    switch( type )
    {
        case aace::alexa::Speaker::Type::AVS_SYNCED:
            return m_enum_Type_AVS_SYNCED.get();
        case aace::alexa::Speaker::Type::LOCAL:
            return m_enum_Type_LOCAL.get();
    }
}

/*
aace::alexa::Speaker::Type SpeakerBinder::convertType( jobject obj )
{
    JNIEnv* env = NativeLib::getJNIEnv();

    if( env->IsSameObject( obj, m_enum_Type_AVS_SYNCED ) ) {
        return aace::alexa::Speaker::Type::AVS_SYNCED;
    }
    else if( env->IsSameObject( obj, m_enum_Type_LOCAL ) ) {
        return aace::alexa::Speaker::Type::LOCAL;
    }
    else {
        return aace::alexa::Speaker::Type::LOCAL; // anything for undefined?
    }
}
*/

// JNI
#define SPEAKER(cptr) ((SpeakerBinder *) cptr)

extern "C" {

}

// END OF FILE
