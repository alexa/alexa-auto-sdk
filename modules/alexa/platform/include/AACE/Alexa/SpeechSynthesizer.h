/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Core/PlatformInterface.h>

/** @file */

namespace aace {
namespace alexa {

/**
 * SpeechSynthesizer should be extended to handle Alexa speech output from the Engine.
 *
 * The SpeechSynthesizer @c MediaPlayer and @c Speaker will receive directives from the Engine to handle Alexa speech playback.
 *
 * @note For observing Alexa dialog state transitions, see @c AlexaClient::dialogStateChanged().
 *
 * @sa AudioChannel
 */
class SpeechSynthesizer : public aace::core::PlatformInterface {
protected:
    SpeechSynthesizer() = default;

public:
    virtual ~SpeechSynthesizer();
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_SPEECH_SYNTHESIZER_H
