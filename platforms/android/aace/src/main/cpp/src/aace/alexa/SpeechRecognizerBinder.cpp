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

// aace/alexa/SpeechRecognizerBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/SpeechRecognizerBinder.h"
#include <string>

SpeechRecognizerBinder::SpeechRecognizerBinder( bool wakewordDetectionEnabled ) : aace::alexa::SpeechRecognizer( wakewordDetectionEnabled ) {
}

void SpeechRecognizerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_wakewordDetected_wakeword = env->GetMethodID( getJavaClass(), "wakewordDetected", "(Ljava/lang/String;)Z" );
    m_javaMethod_endOfSpeechDetected = env->GetMethodID( getJavaClass(), "endOfSpeechDetected", "()V" );
    m_javaMethod_startAudioInput = env->GetMethodID( getJavaClass(), "startAudioInput", "()Z" );
    m_javaMethod_stopAudioInput = env->GetMethodID( getJavaClass(), "stopAudioInput", "()Z" );

    // Initiator
    jclass initiatorEnumClass = env->FindClass( "com/amazon/aace/alexa/SpeechRecognizer$Initiator" );
    m_enum_Initiator_HOLD_TO_TALK = NativeLib::FindEnum( env, initiatorEnumClass, "HOLD_TO_TALK", "Lcom/amazon/aace/alexa/SpeechRecognizer$Initiator;" );
    m_enum_Initiator_TAP_TO_TALK = NativeLib::FindEnum( env, initiatorEnumClass, "TAP_TO_TALK", "Lcom/amazon/aace/alexa/SpeechRecognizer$Initiator;" );
    m_enum_Initiator_WAKEWORD = NativeLib::FindEnum( env, initiatorEnumClass, "WAKEWORD", "Lcom/amazon/aace/alexa/SpeechRecognizer$Initiator;" );
}

bool SpeechRecognizerBinder::wakewordDetected( const std::string& wakeword )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_wakewordDetected_wakeword != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring wakewordStr = context.getEnv()->NewStringUTF( wakeword.c_str() );

            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_wakewordDetected_wakeword, wakewordStr );

            context.getEnv()->DeleteLocalRef( wakewordStr );
        }
    }
    return result;
}

void SpeechRecognizerBinder::endOfSpeechDetected()
{
    if( getJavaObject() != nullptr && m_javaMethod_endOfSpeechDetected != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_endOfSpeechDetected );
        }
    }
}

bool SpeechRecognizerBinder::startAudioInput()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_startAudioInput != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_startAudioInput );
        }
    }
    return result;
}

bool SpeechRecognizerBinder::stopAudioInput()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_stopAudioInput != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_stopAudioInput );
        }
    }
    return result;
}

aace::alexa::SpeechRecognizer::Initiator SpeechRecognizerBinder::convertInitiator( JNIEnv* env, jobject obj )
{
    if( env->IsSameObject( obj, m_enum_Initiator_TAP_TO_TALK.get() ) ) {
        return aace::alexa::SpeechRecognizer::Initiator::TAP_TO_TALK;
    }
    else if( env->IsSameObject( obj, m_enum_Initiator_HOLD_TO_TALK.get() ) ) {
        return aace::alexa::SpeechRecognizer::Initiator::HOLD_TO_TALK;
    }
    else if( env->IsSameObject( obj, m_enum_Initiator_WAKEWORD.get() ) ) {
        return aace::alexa::SpeechRecognizer::Initiator::WAKEWORD;
    }
}

// JNI
#define SPEECHRECOGNIZER(cptr) ((SpeechRecognizerBinder *) cptr)

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_holdToTalk( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return SPEECHRECOGNIZER(cptr)->holdToTalk();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_tapToTalk( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return SPEECHRECOGNIZER(cptr)->tapToTalk();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_startCapture( JNIEnv * env , jobject /* this */, jlong cptr, jobject initiator, jlong keywordBegin, jlong keywordEnd, jstring keyword ) {
    return SPEECHRECOGNIZER(cptr)->startCapture( SPEECHRECOGNIZER(cptr)->convertInitiator( env, initiator ),
                                                 keywordBegin < 0 ? SpeechRecognizerBinder::UNSPECIFIED_INDEX : keywordBegin,
                                                 keywordEnd < 0 ? SpeechRecognizerBinder::UNSPECIFIED_INDEX : keywordEnd,
                                                 NativeLib::convert( env, keyword ) );
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_stopCapture( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return SPEECHRECOGNIZER(cptr)->stopCapture();
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_write( JNIEnv * env , jobject /* this */, jlong cptr, jbyteArray data, jlong offset, jlong size )
{
    jbyte *ptr = env->GetByteArrayElements( data, nullptr );

    jint count = SPEECHRECOGNIZER(cptr)->write( (int16_t *) &ptr[offset], size / 2 );

    env->ReleaseByteArrayElements( data, ptr, JNI_ABORT );

    return count * 2;
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_enableWakewordDetection( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return SPEECHRECOGNIZER(cptr)->enableWakewordDetection();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_disableWakewordDetection( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return SPEECHRECOGNIZER(cptr)->disableWakewordDetection();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_SpeechRecognizer_isWakewordDetectionEnabled( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return SPEECHRECOGNIZER(cptr)->isWakewordDetectionEnabled();
}

}

// END OF FILE
