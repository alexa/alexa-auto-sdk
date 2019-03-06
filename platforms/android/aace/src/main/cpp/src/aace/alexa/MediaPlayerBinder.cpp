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

    // MediaError
    jclass mediaErrorEnumClass = env->FindClass( "com/amazon/aace/alexa/MediaPlayer$MediaError" );
    m_enum_MediaError_MEDIA_ERROR_UNKNOWN =
            NativeLib::FindEnum( env, mediaErrorEnumClass, "MEDIA_ERROR_UNKNOWN",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaError;" );
    m_enum_MediaError_MEDIA_ERROR_INVALID_REQUEST =
            NativeLib::FindEnum( env, mediaErrorEnumClass, "MEDIA_ERROR_INVALID_REQUEST",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaError;" );
    m_enum_MediaError_MEDIA_ERROR_SERVICE_UNAVAILABLE =
            NativeLib::FindEnum( env, mediaErrorEnumClass, "MEDIA_ERROR_SERVICE_UNAVAILABLE",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaError;" );
    m_enum_MediaError_MEDIA_ERROR_INTERNAL_SERVER_ERROR =
            NativeLib::FindEnum( env, mediaErrorEnumClass, "MEDIA_ERROR_INTERNAL_SERVER_ERROR",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaError;" );
    m_enum_MediaError_MEDIA_ERROR_INTERNAL_DEVICE_ERROR =
            NativeLib::FindEnum( env, mediaErrorEnumClass, "MEDIA_ERROR_INTERNAL_DEVICE_ERROR",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaError;" );

    // MediaState
    jclass mediaStateEnumClass = env->FindClass( "com/amazon/aace/alexa/MediaPlayer$MediaState" );
    m_enum_MediaState_STOPPED =
            NativeLib::FindEnum( env, mediaStateEnumClass, "STOPPED",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaState;" );
    m_enum_MediaState_PLAYING =
            NativeLib::FindEnum( env, mediaStateEnumClass, "PLAYING",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaState;" );
    m_enum_MediaState_BUFFERING =
            NativeLib::FindEnum( env, mediaStateEnumClass, "BUFFERING",
                                 "Lcom/amazon/aace/alexa/MediaPlayer$MediaState;" );
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

aace::alexa::MediaPlayer::MediaError MediaPlayerBinder::convertMediaError( JNIEnv* env, jobject obj )
{
    if( m_enum_MediaError_MEDIA_ERROR_UNKNOWN.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaError::MEDIA_ERROR_UNKNOWN;
    }
    else if( m_enum_MediaError_MEDIA_ERROR_INVALID_REQUEST.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaError::MEDIA_ERROR_INVALID_REQUEST;
    }
    else if( m_enum_MediaError_MEDIA_ERROR_SERVICE_UNAVAILABLE.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaError::MEDIA_ERROR_SERVICE_UNAVAILABLE;
    }
    else if( m_enum_MediaError_MEDIA_ERROR_INTERNAL_SERVER_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaError::MEDIA_ERROR_INTERNAL_SERVER_ERROR;
    }
    else if( m_enum_MediaError_MEDIA_ERROR_INTERNAL_DEVICE_ERROR.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaError::MEDIA_ERROR_INTERNAL_DEVICE_ERROR;
    }
    else {
        return aace::alexa::MediaPlayer::MediaError::MEDIA_ERROR_UNKNOWN;
    }
}

aace::alexa::MediaPlayer::MediaState MediaPlayerBinder::convertMediaState( JNIEnv* env, jobject obj )
{
    if( m_enum_MediaState_STOPPED.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaState::STOPPED;
    }
    else if( m_enum_MediaState_PLAYING.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaState::PLAYING;
    }
    else if( m_enum_MediaState_BUFFERING.isSameObject( env, obj ) ) {
        return aace::alexa::MediaPlayer::MediaState::BUFFERING;
    }
    else {
        return aace::alexa::MediaPlayer::MediaState::STOPPED;
    }
}

// JNI
#define MEDIAPLAYER(cptr) ((MediaPlayerBinder *) cptr)

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_isClosed( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return MEDIAPLAYER(cptr)->isClosed();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_isRepeating( JNIEnv * env , jobject /* this */, jlong cptr ) {
    return MEDIAPLAYER(cptr)->isRepeating();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_mediaError( JNIEnv * env , jobject /* this */, jlong cptr, jobject type, jstring error ) {
    MEDIAPLAYER(cptr)->mediaError( MEDIAPLAYER(cptr)->convertMediaError( env, type ), NativeLib::convert( env, error ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_mediaStateChanged( JNIEnv * env , jobject /* this */, jlong cptr, jobject state ) {
    MEDIAPLAYER(cptr)->mediaStateChanged( MEDIAPLAYER(cptr)->convertMediaState( env, state ) );
}

JNIEXPORT jlong JNICALL
Java_com_amazon_aace_alexa_MediaPlayer_read( JNIEnv * env , jobject /* this */, jlong cptr, jbyteArray data, jlong offset, jlong size ) {
    jbyte *ptr = env->GetByteArrayElements( data, nullptr );
    jlong count = MEDIAPLAYER(cptr)->read( (char *) ptr + offset, size );
    env->ReleaseByteArrayElements( data, ptr, 0 );
    return count;
}

}

// END OF FILE
