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

// aace/alexa/MediaPlayerBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/MediaPlayerBinder.h"

void MediaPlayerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_prepare = env->GetMethodID( getJavaClass(), "prepare", "()Z" );
    m_javaMethod_prepare_url = env->GetMethodID( getJavaClass(), "prepare", "(Ljava/lang/String;)Z" );
    m_javaMethod_play = env->GetMethodID( getJavaClass(), "play", "()Z" );
    m_javaMethod_stop = env->GetMethodID( getJavaClass(), "stop", "()Z" );
    m_javaMethod_pause = env->GetMethodID( getJavaClass(), "pause", "()Z" );
    m_javaMethod_resume = env->GetMethodID( getJavaClass(), "resume", "()Z" );
    m_javaMethod_getPosition = env->GetMethodID( getJavaClass(), "getPosition", "()J" );
    m_javaMethod_setPosition_position = env->GetMethodID( getJavaClass(), "setPosition", "(J)Z" );

    // ErrorType
    jclass mediaErrorTypeEnumClass = env->FindClass( "com/amazon/aace/alexa/MediaPlayer$ErrorType" );
    m_enum_ErrorType_MEDIA_ERROR_UNKNOWN = NativeLib::FindEnum( env, mediaErrorTypeEnumClass, "MEDIA_ERROR_UNKNOWN", "Lcom/amazon/aace/alexa/MediaPlayer$ErrorType;" );
    m_enum_ErrorType_MEDIA_ERROR_INVALID_REQUEST = NativeLib::FindEnum( env, mediaErrorTypeEnumClass, "MEDIA_ERROR_INVALID_REQUEST", "Lcom/amazon/aace/alexa/MediaPlayer$ErrorType;" );
    m_enum_ErrorType_MEDIA_ERROR_SERVICE_UNAVAILABLE = NativeLib::FindEnum( env, mediaErrorTypeEnumClass, "MEDIA_ERROR_SERVICE_UNAVAILABLE", "Lcom/amazon/aace/alexa/MediaPlayer$ErrorType;" );
    m_enum_ErrorType_MEDIA_ERROR_INTERNAL_SERVER_ERROR = NativeLib::FindEnum( env, mediaErrorTypeEnumClass, "MEDIA_ERROR_INTERNAL_SERVER_ERROR", "Lcom/amazon/aace/alexa/MediaPlayer$ErrorType;" );
    m_enum_ErrorType_MEDIA_ERROR_INTERNAL_DEVICE_ERROR = NativeLib::FindEnum( env, mediaErrorTypeEnumClass, "MEDIA_ERROR_INTERNAL_DEVICE_ERROR", "Lcom/amazon/aace/alexa/MediaPlayer$ErrorType;" );
}

bool MediaPlayerBinder::prepare()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_prepare != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_prepare );
        }
    }
    return result;
}

bool MediaPlayerBinder::prepare( const std::string & url )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_prepare_url != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring urlStr = context.getEnv()->NewStringUTF( url.c_str() );
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_prepare_url, urlStr );
            context.getEnv()->DeleteLocalRef( urlStr );
        }
    }
    return result;
}

bool MediaPlayerBinder::play()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_play != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_play );
        }
    }
    return result;
}

bool MediaPlayerBinder::stop()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_stop != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_stop );
        }
    }
    return result;
}

bool MediaPlayerBinder::pause()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_pause != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_pause );
        }
    }
    return result;
}

bool MediaPlayerBinder::resume()
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_resume != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_resume );
        }
    }
    return result;
}

int64_t MediaPlayerBinder::getPosition()
{
    int64_t result = 0;
    if( getJavaObject() != nullptr && m_javaMethod_getPosition != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallLongMethod( getJavaObject(), m_javaMethod_getPosition );
        }
    }
    return result;
}

bool MediaPlayerBinder::setPosition( int64_t position )
{
    bool result = false;
    if( getJavaObject() != nullptr && m_javaMethod_setPosition_position != nullptr )
    {
        ThreadContext context;

        if( context.isValid() ) {
            result = context.getEnv()->CallBooleanMethod( getJavaObject(), m_javaMethod_setPosition_position, position );
        }
    }
    return result;
}

jobject MediaPlayerBinder::convert( aace::alexa::MediaPlayer::ErrorType type )
{
    switch( type )
    {
        case aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_UNKNOWN:
            return m_enum_ErrorType_MEDIA_ERROR_UNKNOWN.get();
        case aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INVALID_REQUEST:
            return m_enum_ErrorType_MEDIA_ERROR_INVALID_REQUEST.get();
        case aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_SERVICE_UNAVAILABLE:
            return m_enum_ErrorType_MEDIA_ERROR_SERVICE_UNAVAILABLE.get();
        case aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INTERNAL_SERVER_ERROR:
            return m_enum_ErrorType_MEDIA_ERROR_INTERNAL_SERVER_ERROR.get();
        case aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR:
            return m_enum_ErrorType_MEDIA_ERROR_INTERNAL_DEVICE_ERROR.get();
    }
}

aace::alexa::MediaPlayer::ErrorType MediaPlayerBinder::convertErrorType( JNIEnv* env, jobject obj )
{
    if( m_enum_ErrorType_MEDIA_ERROR_UNKNOWN.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_UNKNOWN;
    }
    else if( m_enum_ErrorType_MEDIA_ERROR_INVALID_REQUEST.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INVALID_REQUEST;
    }
    else if( m_enum_ErrorType_MEDIA_ERROR_SERVICE_UNAVAILABLE.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_SERVICE_UNAVAILABLE;
    }
    else if( m_enum_ErrorType_MEDIA_ERROR_INTERNAL_SERVER_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INTERNAL_SERVER_ERROR;
    }
    else if( m_enum_ErrorType_MEDIA_ERROR_INTERNAL_DEVICE_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR;
    }
    else {
        return aace::alexa::MediaPlayer::ErrorType::MEDIA_ERROR_INTERNAL_DEVICE_ERROR; // anything for undefined?
    }
}

// JNI
#define MEDIAPLAYER(cptr) ((MediaPlayerBinder *) cptr)

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_isRepeating( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return MEDIAPLAYER(cptr)->isRepeating();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_playbackStarted( JNIEnv * env , jobject /* this */, jlong cptr ) {
    MEDIAPLAYER(cptr)->playbackStarted();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_playbackFinished( JNIEnv * env , jobject /* this */, jlong cptr ) {
    MEDIAPLAYER(cptr)->playbackFinished();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_playbackPaused( JNIEnv * env , jobject /* this */, jlong cptr ) {
    MEDIAPLAYER(cptr)->playbackPaused();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_playbackResumed( JNIEnv * env , jobject /* this */, jlong cptr ) {
    MEDIAPLAYER(cptr)->playbackResumed();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_playbackStopped( JNIEnv * env , jobject /* this */, jlong cptr ) {
    MEDIAPLAYER(cptr)->playbackStopped();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_playbackError( JNIEnv * env , jobject /* this */, jlong cptr, jobject type, jstring error ) {
    MEDIAPLAYER(cptr)->playbackError( MEDIAPLAYER(cptr)->convertErrorType( env, type ), NativeLib::convert( env, error ) );
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_read( JNIEnv * env , jobject /* this */, jlong cptr, jbyteArray data, jlong size ) {
    jbyte *ptr = env->GetByteArrayElements( data, nullptr );
    jlong count = MEDIAPLAYER(cptr)->read( (char *) ptr, size );
    env->ReleaseByteArrayElements( data, ptr, 0 );
    return count;
}

}

// END OF FILE
