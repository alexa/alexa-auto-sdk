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

// aace/alexa/PlaybackControllerBinder.cpp
// This is an automatically generated file.

#include "aace/alexa/PlaybackControllerBinder.h"

// JNI
#define PLAYBACKCONTROLLER(cptr) ((PlaybackControllerBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_playButtonPressed( JNIEnv * env , jobject /* this */, jlong cptr ) {
    PLAYBACKCONTROLLER(cptr)->playButtonPressed();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_pauseButtonPressed( JNIEnv * env , jobject /* this */, jlong cptr ) {
    PLAYBACKCONTROLLER(cptr)->pauseButtonPressed();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_nextButtonPressed( JNIEnv * env , jobject /* this */, jlong cptr ) {
    PLAYBACKCONTROLLER(cptr)->nextButtonPressed();
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_PlaybackController_previousButtonPressed( JNIEnv * env , jobject /* this */, jlong cptr ) {
    PLAYBACKCONTROLLER(cptr)->previousButtonPressed();
}

}

// END OF FILE
