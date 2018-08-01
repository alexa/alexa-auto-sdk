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

// aace/phonecontrol/PhoneCallControllerBinder.cpp

#include "aace/phonecontrol/PhoneCallControllerBinder.h"

void PhoneCallControllerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_dial_payload = env->GetMethodID( getJavaClass(), "dial", "(Ljava/lang/String;)Z" );

    // ConnectionState
    jclass connectionStateTypeEnumClass = env->FindClass( "com/amazon/aace/phonecontrol/PhoneCallController$ConnectionState" );
    m_enum_ConnectionState_CONNECTED = NativeLib::FindEnum( env, connectionStateTypeEnumClass, "CONNECTED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$ConnectionState;" );
    m_enum_ConnectionState_DISCONNECTED = NativeLib::FindEnum( env, connectionStateTypeEnumClass, "DISCONNECTED", "Lcom/amazon/aace/phonecontrol/PhoneCallController$ConnectionState;" );
}

bool PhoneCallControllerBinder::dial( const std::string & payload )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_dial_payload != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring payloadStr = context.getEnv()->NewStringUTF( payload.c_str() );
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_dial_payload, payloadStr );
            context.getEnv()->DeleteLocalRef( payloadStr );
        }
    }
    return result;
}


jobject PhoneCallControllerBinder::convert( aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state )
{
    switch( state )
    {
        case aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED:
            return m_enum_ConnectionState_CONNECTED.get();
        case aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED:
            return m_enum_ConnectionState_DISCONNECTED.get();
    }
}

aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState PhoneCallControllerBinder::convertConnectionState( JNIEnv* env, jobject obj )
{
    if( m_enum_ConnectionState_CONNECTED.isSameObject( env, obj ) ) {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::CONNECTED;
    }
    else {
        return aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState::DISCONNECTED;
    }
}

// JNI
#define PHONECALLCONTROLLER(cptr) ((PhoneCallControllerBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callActivated( JNIEnv * env , jobject /* this */, jlong cptr, jstring callId ) {
    PHONECALLCONTROLLER(cptr)->callActivated( NativeLib::convert( env, callId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callTerminated( JNIEnv * env , jobject /* this */, jlong cptr, jstring callId ) {
    PHONECALLCONTROLLER(cptr)->callTerminated( NativeLib::convert( env, callId ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callFailed__JLjava_lang_String_2Ljava_lang_String_2
        ( JNIEnv * env , jobject /* this */, jlong cptr, jstring callId, jstring error ) {
    PHONECALLCONTROLLER(cptr)->callFailed(NativeLib::convert( env, callId ), NativeLib::convert( env, error ));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_callFailed__JLjava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2
        ( JNIEnv * env , jobject /* this */, jlong cptr, jstring callId, jstring error, jstring message ) {
    PHONECALLCONTROLLER(cptr)->callFailed( NativeLib::convert( env, callId ), NativeLib::convert( env, error ), NativeLib::convert( env, message ));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_phonecontrol_PhoneCallController_connectionStateChanged( JNIEnv * env , jobject /* this */, jlong cptr, jobject state ) {
    PHONECALLCONTROLLER(cptr)->connectionStateChanged( PHONECALLCONTROLLER(cptr)->convertConnectionState( env, state ) );
}

}

// END OF FILE
