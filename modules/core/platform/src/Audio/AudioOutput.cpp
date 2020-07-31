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

#include <AACE/Audio/AudioOutput.h>

namespace aace {
namespace audio {

AudioOutput::~AudioOutput() = default;  // key function

int64_t AudioOutput::getNumBytesBuffered() {
    return 0;
}

void AudioOutput::mediaStateChanged(MediaState state) {
    if (auto m_audioOutputEngineInterface_lock = m_audioOutputEngineInterface.lock()) {
        m_audioOutputEngineInterface_lock->onMediaStateChanged(state);
    }
}

void AudioOutput::mediaError(MediaError error, const std::string& description) {
    if (auto m_audioOutputEngineInterface_lock = m_audioOutputEngineInterface.lock()) {
        m_audioOutputEngineInterface_lock->onMediaError(error, description);
    }
}

void AudioOutput::setEngineInterface(
    std::shared_ptr<aace::audio::AudioOutputEngineInterface> audioOutputEngineInterface) {
    m_audioOutputEngineInterface = audioOutputEngineInterface;
}

}  // namespace audio
}  // namespace aace
