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

// aace/alexa/MediaPlayerBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_MEDIA_PLAYER_BINDER_H
#define AACE_ALEXA_MEDIA_PLAYER_BINDER_H

#include "AACE/Alexa/MediaPlayer.h"
#include "aace/core/PlatformInterfaceBinder.h"

class MediaPlayerBinder : public PlatformInterfaceBinder, public aace::alexa::MediaPlayer {
public:
    MediaPlayerBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool prepare() override;
    bool prepare( const std::string & url ) override;
    bool play() override;
    bool stop() override;
    bool pause() override;
    bool resume() override;
    int64_t getPosition() override;
    bool setPosition( int64_t position ) override;

public:
    aace::alexa::MediaPlayer::MediaError convertMediaError( JNIEnv* env, jobject obj );
    aace::alexa::MediaPlayer::MediaState convertMediaState( JNIEnv* env, jobject obj );

private:
    jmethodID m_javaMethod_prepare = nullptr;
    jmethodID m_javaMethod_prepare_url = nullptr;
    jmethodID m_javaMethod_play = nullptr;
    jmethodID m_javaMethod_stop = nullptr;
    jmethodID m_javaMethod_pause = nullptr;
    jmethodID m_javaMethod_resume = nullptr;
    jmethodID m_javaMethod_getPosition = nullptr;
    jmethodID m_javaMethod_setPosition_position = nullptr;

    // MediaError
    ObjectRef m_enum_MediaError_MEDIA_ERROR_UNKNOWN;
    ObjectRef m_enum_MediaError_MEDIA_ERROR_INVALID_REQUEST;
    ObjectRef m_enum_MediaError_MEDIA_ERROR_SERVICE_UNAVAILABLE;
    ObjectRef m_enum_MediaError_MEDIA_ERROR_INTERNAL_SERVER_ERROR;
    ObjectRef m_enum_MediaError_MEDIA_ERROR_INTERNAL_DEVICE_ERROR;

    // MediaState
    ObjectRef m_enum_MediaState_STOPPED;
    ObjectRef m_enum_MediaState_PLAYING;
    ObjectRef m_enum_MediaState_BUFFERING;
};

#endif //AACE_ALEXA_MEDIA_PLAYER_BINDER_H
