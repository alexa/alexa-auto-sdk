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

#ifndef AACE_ALEXA_SPEECH_SYNTHESIZER_H
#define AACE_ALEXA_SPEECH_SYNTHESIZER_H

#include "AudioChannel.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c SpeechSynthesizer class must be extended by the platform implementation to handle
 * playback of Alexa's synthesized speech audio data.
 * @sa MediaPlayer Speaker
 */
class SpeechSynthesizer : public AudioChannel {
protected:
    SpeechSynthesizer( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

public:
    virtual ~SpeechSynthesizer() = default;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_SPEECH_SYNTHESIZER_H
