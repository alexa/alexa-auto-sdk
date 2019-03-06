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

// aace/alexa/AlertsBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/AlertsBinder.h"

AlertsBinder::AlertsBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::Alerts( mediaPlayer, speaker ) {
}

void AlertsBinder::initialize( JNIEnv* env )
{
    m_javaMethod_alertStateChanged_alertToken_state_reason = env->GetMethodID( getJavaClass(), "alertStateChanged", "(Ljava/lang/String;Lcom/amazon/aace/alexa/Alerts$AlertState;Ljava/lang/String;)V" );

    m_javaMethod_alertCreated_alertToken_detailedInfo = env->GetMethodID( getJavaClass(), "alertCreated", "(Ljava/lang/String;Ljava/lang/String;)V" );

    m_javaMethod_alertDeleted_alertToken = env->GetMethodID( getJavaClass(), "alertDeleted", "(Ljava/lang/String;)V" );


    // AlertState
    jclass alertStateEnumClass = env->FindClass( "com/amazon/aace/alexa/Alerts$AlertState" );
    m_enum_AlertState_READY = NativeLib::FindEnum( env, alertStateEnumClass, "READY", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_STARTED = NativeLib::FindEnum( env, alertStateEnumClass, "STARTED", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_STOPPED = NativeLib::FindEnum( env, alertStateEnumClass, "STOPPED", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_SNOOZED = NativeLib::FindEnum( env, alertStateEnumClass, "SNOOZED", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_COMPLETED = NativeLib::FindEnum( env, alertStateEnumClass, "COMPLETED", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_PAST_DUE = NativeLib::FindEnum( env, alertStateEnumClass, "PAST_DUE", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_FOCUS_ENTERED_FOREGROUND = NativeLib::FindEnum( env, alertStateEnumClass, "FOCUS_ENTERED_FOREGROUND", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_FOCUS_ENTERED_BACKGROUND = NativeLib::FindEnum( env, alertStateEnumClass, "FOCUS_ENTERED_BACKGROUND", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
    m_enum_AlertState_ERROR = NativeLib::FindEnum( env, alertStateEnumClass, "ERROR", "Lcom/amazon/aace/alexa/Alerts$AlertState;" );
}

void AlertsBinder::alertStateChanged( const std::string & alertToken, aace::alexa::Alerts::AlertState state, const std::string & reason )
{
    if( getJavaObject() != nullptr && m_javaMethod_alertStateChanged_alertToken_state_reason != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring alertTokenStr = context.getEnv()->NewStringUTF( alertToken.c_str() );
            jstring reasonStr = context.getEnv()->NewStringUTF( reason.c_str() );

            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_alertStateChanged_alertToken_state_reason, alertTokenStr, convert( state ), reasonStr );
            context.getEnv()->DeleteLocalRef( alertTokenStr );
            context.getEnv()->DeleteLocalRef( reasonStr );
        }
    }
}

void AlertsBinder::alertCreated( const std::string & alertToken, const std::string & detailedInfo )
{
    if( getJavaObject() != nullptr && m_javaMethod_alertCreated_alertToken_detailedInfo != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring alertTokenStr = context.getEnv()->NewStringUTF( alertToken.c_str() );
            jstring detailedInfoStr = context.getEnv()->NewStringUTF( detailedInfo.c_str() );

            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_alertCreated_alertToken_detailedInfo, alertTokenStr, detailedInfoStr );
            context.getEnv()->DeleteLocalRef( alertTokenStr );
            context.getEnv()->DeleteLocalRef( detailedInfoStr );
        }
    }
}

void AlertsBinder::alertDeleted( const std::string & alertToken )
{
    if( getJavaObject() != nullptr && m_javaMethod_alertDeleted_alertToken != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring alertTokenStr = context.getEnv()->NewStringUTF( alertToken.c_str() );

            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_alertDeleted_alertToken, alertTokenStr );
            context.getEnv()->DeleteLocalRef( alertTokenStr );
        }
    }
}


jobject AlertsBinder::convert( aace::alexa::Alerts::AlertState state )
{
    switch( state )
    {
        case aace::alexa::Alerts::AlertState::READY:
            return m_enum_AlertState_READY.get();
        case aace::alexa::Alerts::AlertState::STARTED:
            return m_enum_AlertState_STARTED.get();
        case aace::alexa::Alerts::AlertState::STOPPED:
            return m_enum_AlertState_STOPPED.get();
        case aace::alexa::Alerts::AlertState::SNOOZED:
            return m_enum_AlertState_SNOOZED.get();
        case aace::alexa::Alerts::AlertState::COMPLETED:
            return m_enum_AlertState_COMPLETED.get();
        case aace::alexa::Alerts::AlertState::PAST_DUE:
            return m_enum_AlertState_PAST_DUE.get();
        case aace::alexa::Alerts::AlertState::FOCUS_ENTERED_FOREGROUND:
            return m_enum_AlertState_FOCUS_ENTERED_FOREGROUND.get();
        case aace::alexa::Alerts::AlertState::FOCUS_ENTERED_BACKGROUND:
            return m_enum_AlertState_FOCUS_ENTERED_BACKGROUND.get();
        case aace::alexa::Alerts::AlertState::ERROR:
            return m_enum_AlertState_ERROR.get();
    }
}

/*
aace::alexa::Alerts::AlertState AlertsBinder::convertAlertState( jobject obj )
{
    JNIEnv* env = NativeLib::getJNIEnv();

    if( env->IsSameObject( obj, m_enum_AlertState_READY.get() ) ) {
        return aace::alexa::Alerts::AlertState::READY;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_STARTED.get() ) ) {
        return aace::alexa::Alerts::AlertState::STARTED;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_STOPPED.get() ) ) {
        return aace::alexa::Alerts::AlertState::STOPPED;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_SNOOZED.get() ) ) {
        return aace::alexa::Alerts::AlertState::SNOOZED;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_COMPLETED.get() ) ) {
        return aace::alexa::Alerts::AlertState::COMPLETED;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_PAST_DUE.get() ) ) {
        return aace::alexa::Alerts::AlertState::PAST_DUE;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_FOCUS_ENTERED_FOREGROUND.get() ) ) {
        return aace::alexa::Alerts::AlertState::FOCUS_ENTERED_FOREGROUND;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_FOCUS_ENTERED_BACKGROUND.get() ) ) {
        return aace::alexa::Alerts::AlertState::FOCUS_ENTERED_BACKGROUND;
    }
    else if( env->IsSameObject( obj, m_enum_AlertState_ERROR.get() ) ) {
        return aace::alexa::Alerts::AlertState::ERROR;
    }
    else {
        return aace::alexa::Alerts::AlertState::ERROR; // anything for undefined?
    }
}
 */

// JNI
#define ALERTS(cptr) ((AlertsBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_Alerts_localStop( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return ALERTS(cptr)->localStop();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_Alerts_removeAllAlerts( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return ALERTS(cptr)->removeAllAlerts();
}

}

// END OF FILE
