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

// aace/alexa/PlaybackControllerBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_PLAYBACK_CONTROLLER_BINDER_H
#define AACE_ALEXA_PLAYBACK_CONTROLLER_BINDER_H

#include "AACE/Alexa/PlaybackController.h"
#include "aace/core/PlatformInterfaceBinder.h"

class PlaybackControllerBinder : public PlatformInterfaceBinder, public aace::alexa::PlaybackController {
public:
    PlaybackControllerBinder() = default;
    aace::alexa::PlaybackControllerEngineInterface::PlaybackButton convertPlaybackButton( JNIEnv* env, jobject obj );
    aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle convertPlaybackToggle( JNIEnv* env, jobject obj );

protected:
    void initialize( JNIEnv* env ) override;

private:
    jobject convert( aace::alexa::PlaybackControllerEngineInterface::PlaybackButton playbackButton );
    jobject convert( aace::alexa::PlaybackControllerEngineInterface::PlaybackToggle playbackToggle );

    // PlaybackButton
    ObjectRef m_enum_PlaybackButton_PLAY;
    ObjectRef m_enum_PlaybackButton_PAUSE;
    ObjectRef m_enum_PlaybackButton_NEXT;
    ObjectRef m_enum_PlaybackButton_PREVIOUS;
    ObjectRef m_enum_PlaybackButton_SKIP_FORWARD;
    ObjectRef m_enum_PlaybackButton_SKIP_BACKWARD;

    // PlaybackToggle
    ObjectRef m_enum_PlaybackToggle_SHUFFLE;
    ObjectRef m_enum_PlaybackToggle_LOOP;
    ObjectRef m_enum_PlaybackToggle_REPEAT;
    ObjectRef m_enum_PlaybackToggle_THUMBS_UP;
    ObjectRef m_enum_PlaybackToggle_THUMBS_DOWN;
};

#endif //AACE_ALEXA_PLAYBACK_CONTROLLER_BINDER_H
