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

#ifndef AACE_AUDIO_AUDIO_OUTPUT_PROVIDER_H
#define AACE_AUDIO_AUDIO_OUTPUT_PROVIDER_H

#include <AACE/Core/PlatformInterface.h>
#include "AudioOutput.h"

/** @file */

namespace aace {
namespace audio {

class AudioOutputProvider : public aace::core::PlatformInterface {
public:
    enum class AudioOutputType { TTS, MUSIC, NOTIFICATION, ALARM, EARCON, COMMUNICATION, RINGTONE };

protected:
    AudioOutputProvider() = default;

public:
    virtual ~AudioOutputProvider();

    virtual std::shared_ptr<AudioOutput> openChannel(const std::string& name, AudioOutputType type) = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const AudioOutputProvider::AudioOutputType& audioOutputType) {
    switch (audioOutputType) {
        case AudioOutputProvider::AudioOutputType::TTS:
            stream << "TTS";
            break;
        case AudioOutputProvider::AudioOutputType::MUSIC:
            stream << "MUSIC";
            break;
        case AudioOutputProvider::AudioOutputType::NOTIFICATION:
            stream << "NOTIFICATION";
            break;
        case AudioOutputProvider::AudioOutputType::ALARM:
            stream << "ALARM";
            break;
        case AudioOutputProvider::AudioOutputType::EARCON:
            stream << "EARCON";
            break;
        case AudioOutputProvider::AudioOutputType::COMMUNICATION:
            stream << "COMMUNICATION";
            break;
        case AudioOutputProvider::AudioOutputType::RINGTONE:
            stream << "RINGTONE";
            break;
    }
    return stream;
}

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_OUTPUT_PROVIDER_H
