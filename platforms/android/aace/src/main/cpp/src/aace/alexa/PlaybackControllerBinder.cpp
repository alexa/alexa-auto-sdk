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

// aace/alexa/PlaybackControllerBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/PlaybackControllerBinder.h"

void PlaybackControllerBinder::initialize( JNIEnv* env )
{
    // PlaybackButton
    jclass playbackButtonEnumClass = env->FindClass( "com/amazon/aace/alexa/PlaybackController$PlaybackButton" );
    m_enum_PlaybackButton_PLAY = NativeLib::FindEnum( env, playbackButtonEnumClass, "PLAY", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackButton;" );
    m_enum_PlaybackButton_PAUSE = NativeLib::FindEnum( env, playbackButtonEnumClass, "PAUSE", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackButton;" );;
    m_enum_PlaybackButton_NEXT = NativeLib::FindEnum( env, playbackButtonEnumClass, "NEXT", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackButton;" );;
    m_enum_PlaybackButton_PREVIOUS = NativeLib::FindEnum( env, playbackButtonEnumClass, "PREVIOUS", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackButton;" );;
    m_enum_PlaybackButton_SKIP_FORWARD = NativeLib::FindEnum( env, playbackButtonEnumClass, "SKIP_FORWARD", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackButton;" );;
    m_enum_PlaybackButton_SKIP_BACKWARD = NativeLib::FindEnum( env, playbackButtonEnumClass, "SKIP_BACKWARD", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackButton;" );;

    // PlaybackToggle
    jclass playbackToggleEnumClass = env->FindClass( "com/amazon/aace/alexa/PlaybackController$PlaybackToggle" );
    m_enum_PlaybackToggle_SHUFFLE = NativeLib::FindEnum( env, playbackToggleEnumClass, "SHUFFLE", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackToggle;" );
    m_enum_PlaybackToggle_LOOP = NativeLib::FindEnum( env, playbackToggleEnumClass, "LOOP", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackToggle;" );;
    m_enum_PlaybackToggle_REPEAT = NativeLib::FindEnum( env, playbackToggleEnumClass, "REPEAT", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackToggle;" );;
    m_enum_PlaybackToggle_THUMBS_UP = NativeLib::FindEnum( env, playbackToggleEnumClass, "THUMBS_UP", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackToggle;" );;
    m_enum_PlaybackToggle_THUMBS_DOWN = NativeLib::FindEnum( env, playbackToggleEnumClass, "THUMBS_DOWN", "Lcom/amazon/aace/alexa/PlaybackController$PlaybackToggle;" );;

}

aace::alexa::PlaybackControllerEngineInterface::PlaybackButton PlaybackControllerBinder::convertPlaybackButton( JNIEnv* env, jobject obj )
{
    if( m_enum_PlaybackButton_PLAY.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::PLAY;
    }
    else if( m_enum_PlaybackButton_PAUSE.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::PAUSE;
    }
    else if( m_enum_PlaybackButton_NEXT.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::NEXT;
    }
    else if( m_enum_PlaybackButton_PREVIOUS.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::PREVIOUS;
    }
    else if( m_enum_PlaybackButton_SKIP_FORWARD.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::SKIP_FORWARD;
    }
    else if( m_enum_PlaybackButton_SKIP_BACKWARD.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::SKIP_BACKWARD;
    }
    else {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackButton::PLAY; // anything for undefined?
    }
}

aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle PlaybackControllerBinder::convertPlaybackToggle( JNIEnv* env, jobject obj )
{
    if( m_enum_PlaybackToggle_SHUFFLE.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle::SHUFFLE;
    }
    else if( m_enum_PlaybackToggle_LOOP.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle::LOOP;
    }
    else if( m_enum_PlaybackToggle_REPEAT.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle::REPEAT;
    }
    else if( m_enum_PlaybackToggle_THUMBS_UP.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle::THUMBS_UP;
    }
    else if( m_enum_PlaybackToggle_THUMBS_DOWN.isSameObject( env, obj ) ) {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle::THUMBS_DOWN;
    }
    else {
        return aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle::SHUFFLE; // anything for undefined?
    }
}

// JNI
#define PLAYBACKCONTROLLER(cptr) ((PlaybackControllerBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_buttonPressed( JNIEnv * env , jobject /* this */, jlong cptr, jobject playbackButton ) {
    PLAYBACKCONTROLLER(cptr)->buttonPressed(PLAYBACKCONTROLLER(cptr)->convertPlaybackButton( env, playbackButton ));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_togglePressed( JNIEnv * env , jobject /* this */, jlong cptr, jobject playbackToggle, jboolean action) {
    PLAYBACKCONTROLLER(cptr)->togglePressed(PLAYBACKCONTROLLER(cptr)->convertPlaybackToggle( env, playbackToggle ), action);
}

}

// END OF FILE
