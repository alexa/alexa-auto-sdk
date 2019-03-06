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

// aace/alexa/AudioPlayerBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_AUDIO_PLAYER_BINDER_H
#define AACE_ALEXA_AUDIO_PLAYER_BINDER_H

#include "AACE/Alexa/AudioPlayer.h"
#include "aace/core/PlatformInterfaceBinder.h"

class AudioPlayerBinder : public PlatformInterfaceBinder, public aace::alexa::AudioPlayer {
public:
    AudioPlayerBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );
    virtual ~AudioPlayerBinder();

protected:
    void initialize( JNIEnv* env ) override;

public:
    void playerActivityChanged( aace::alexa::AudioPlayer::PlayerActivity state ) override;

private:
    jobject convert( aace::alexa::AudioPlayer::PlayerActivity state );

private:
    jmethodID m_javaMethod_playerActivityChanged_state = nullptr;

    // PlayerActivity
    ObjectRef m_enum_PlayerActivity_IDLE;
    ObjectRef m_enum_PlayerActivity_PLAYING;
    ObjectRef m_enum_PlayerActivity_STOPPED;
    ObjectRef m_enum_PlayerActivity_PAUSED;
    ObjectRef m_enum_PlayerActivity_BUFFER_UNDERRUN;
    ObjectRef m_enum_PlayerActivity_FINISHED;

};

#endif //AACE_ALEXA_AUDIO_PLAYER_BINDER_H
