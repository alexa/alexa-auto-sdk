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

#include <AACE/Audio/AudioInput.h>

namespace aace {
namespace audio {

AudioInput::~AudioInput() = default;  // key function

ssize_t AudioInput::write(const int16_t* data, const size_t size) {
    return m_audioInputEngineInterface != nullptr ? m_audioInputEngineInterface->write(data, size) : 0;
}

void AudioInput::setEngineInterface(std::shared_ptr<aace::audio::AudioInputEngineInterface> audioInputEngineInterface) {
    m_audioInputEngineInterface = audioInputEngineInterface;
}

}  // namespace audio
}  // namespace aace
