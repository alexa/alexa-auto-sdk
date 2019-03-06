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

// aace/alexa/AudioPlayerBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/AudioPlayerBinder.h"

AudioPlayerBinder::AudioPlayerBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker ) : aace::alexa::AudioPlayer( mediaPlayer, speaker ) {
}

AudioPlayerBinder::~AudioPlayerBinder() {
}

void AudioPlayerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_playerActivityChanged_state = env->GetMethodID( getJavaClass(), "playerActivityChanged", "(Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;)V" );

    // PlayerActivity
    jclass playerActivityEnumClass = env->FindClass( "com/amazon/aace/alexa/AudioPlayer$PlayerActivity" );
    m_enum_PlayerActivity_IDLE = NativeLib::FindEnum( env, playerActivityEnumClass, "IDLE", "Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;" );
    m_enum_PlayerActivity_PLAYING = NativeLib::FindEnum( env, playerActivityEnumClass, "PLAYING", "Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;" );
    m_enum_PlayerActivity_STOPPED = NativeLib::FindEnum( env, playerActivityEnumClass, "STOPPED", "Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;" );
    m_enum_PlayerActivity_PAUSED = NativeLib::FindEnum( env, playerActivityEnumClass, "PAUSED", "Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;" );
    m_enum_PlayerActivity_BUFFER_UNDERRUN = NativeLib::FindEnum( env, playerActivityEnumClass, "BUFFER_UNDERRUN", "Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;" );
    m_enum_PlayerActivity_FINISHED = NativeLib::FindEnum( env, playerActivityEnumClass, "FINISHED", "Lcom/amazon/aace/alexa/AudioPlayer$PlayerActivity;" );
}

void AudioPlayerBinder::playerActivityChanged(aace::alexa::AudioPlayer::PlayerActivity state)
{
    if( getJavaObject() != nullptr && m_javaMethod_playerActivityChanged_state != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_playerActivityChanged_state, convert( state ) );
        }
    }
}

jobject AudioPlayerBinder::convert( aace::alexa::AudioPlayer::PlayerActivity state )
{
    switch( state )
    {
        case aace::alexa::AudioPlayer::PlayerActivity::IDLE:
            return m_enum_PlayerActivity_IDLE.get();
        case aace::alexa::AudioPlayer::PlayerActivity::PLAYING:
            return m_enum_PlayerActivity_PLAYING.get();
        case aace::alexa::AudioPlayer::PlayerActivity::STOPPED:
            return m_enum_PlayerActivity_STOPPED.get();
        case aace::alexa::AudioPlayer::PlayerActivity::PAUSED:
            return m_enum_PlayerActivity_PAUSED.get();
        case aace::alexa::AudioPlayer::PlayerActivity::BUFFER_UNDERRUN:
            return m_enum_PlayerActivity_BUFFER_UNDERRUN.get();
        case aace::alexa::AudioPlayer::PlayerActivity::FINISHED:
            return m_enum_PlayerActivity_FINISHED.get();
    }
}


// JNI
#define AUDIOPLAYER(cptr) ((AudioPlayerBinder *) cptr)

extern "C" {

}

// END OF FILE
