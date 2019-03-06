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

// aace/alexa/NotificationsBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/NotificationsBinder.h"

NotificationsBinder::NotificationsBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::Notifications( mediaPlayer, speaker )
{
}

void NotificationsBinder::initialize( JNIEnv* env )
{
    m_javaMethod_setIndicator_state = env->GetMethodID( getJavaClass(), "setIndicator", "(Lcom/amazon/aace/alexa/Notifications$IndicatorState;)V" );

    // IndicatorState
    jclass indicatorStateEnumClass = env->FindClass( "com/amazon/aace/alexa/Notifications$IndicatorState" );
    m_enum_IndicatorState_OFF = NativeLib::FindEnum( env, indicatorStateEnumClass, "OFF", "Lcom/amazon/aace/alexa/Notifications$IndicatorState;" );
    m_enum_IndicatorState_ON = NativeLib::FindEnum( env, indicatorStateEnumClass, "ON", "Lcom/amazon/aace/alexa/Notifications$IndicatorState;" );
    m_enum_IndicatorState_UNKNOWN = NativeLib::FindEnum( env, indicatorStateEnumClass, "UNKNOWN", "Lcom/amazon/aace/alexa/Notifications$IndicatorState;" );
}

void NotificationsBinder::setIndicator( aace::alexa::Notifications::IndicatorState state )
{
    if( getJavaObject() != nullptr && m_javaMethod_setIndicator_state != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_setIndicator_state, convert( state ) );
        }
    }
}

jobject NotificationsBinder::convert( aace::alexa::Notifications::IndicatorState state )
{
    switch( state )
    {
        case aace::alexa::Notifications::IndicatorState::OFF:
            return m_enum_IndicatorState_OFF.get();
        case aace::alexa::Notifications::IndicatorState::ON:
            return m_enum_IndicatorState_ON.get();
        case aace::alexa::Notifications::IndicatorState::UNKNOWN:
            return m_enum_IndicatorState_UNKNOWN.get();
    }
}

/*
aace::alexa::Notifications::IndicatorState NotificationsBinder::convertIndicatorState( JNIEnv* env, jobject obj )
{
    if( m_enum_IndicatorState_OFF.isSameObject( env, obj ) ) {
        return aace::alexa::Notifications::IndicatorState::OFF;
    }
    else if( m_enum_IndicatorState_ON.isSameObject( env, obj ) ) {
        return aace::alexa::Notifications::IndicatorState::ON;
    }
    else if( m_enum_IndicatorState_UNKNOWN.isSameObject( env, obj ) ) {
        return aace::alexa::Notifications::IndicatorState::UNKNOWN;
    }
    else {
        return aace::alexa::Notifications::IndicatorState::UNKNOWN;
    }
}
*/

// JNI
#define NOTIFICATIONS(cptr) ((NotificationsBinder *) cptr)

extern "C" {

}

// END OF FILE
