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

// aace/alexa/SpeechSynthesizerBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_SPEECH_SYNTHESIZER_BINDER_H
#define AACE_ALEXA_SPEECH_SYNTHESIZER_BINDER_H

#include "AACE/Alexa/SpeechSynthesizer.h"
#include "aace/core/PlatformInterfaceBinder.h"

class SpeechSynthesizerBinder : public PlatformInterfaceBinder, public aace::alexa::SpeechSynthesizer {
public:
    SpeechSynthesizerBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );
};

#endif //AACE_ALEXA_SPEECH_SYNTHESIZER_BINDER_H
