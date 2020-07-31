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

#ifndef AACE_AUDIO_AUDIO_INPUT_PROVIDER_H
#define AACE_AUDIO_AUDIO_INPUT_PROVIDER_H

#include "AACE/Core/PlatformInterface.h"
#include "AudioInput.h"

/** @file */

namespace aace {
namespace audio {

class AudioInputProvider : public aace::core::PlatformInterface {
protected:
    AudioInputProvider() = default;

public:
    enum class AudioInputType { VOICE, COMMUNICATION, LOOPBACK };

    virtual ~AudioInputProvider();

    virtual std::shared_ptr<AudioInput> openChannel(const std::string& name, AudioInputType type) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const AudioInputProvider::AudioInputType& audioInputType) {
    switch (audioInputType) {
        case AudioInputProvider::AudioInputType::VOICE:
            stream << "VOICE";
            break;
        case AudioInputProvider::AudioInputType::COMMUNICATION:
            stream << "COMMUNICATION";
            break;
        case AudioInputProvider::AudioInputType::LOOPBACK:
            stream << "LOOPBACK";
            break;
    }
    return stream;
}

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_INPUT_PROVIDER_H
