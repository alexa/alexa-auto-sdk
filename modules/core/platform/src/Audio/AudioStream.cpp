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

#include <AACE/Audio/AudioStream.h>

namespace aace {
namespace audio {

AudioStream::~AudioStream() = default;

AudioStream::Encoding AudioStream::getEncoding() {
    return getAudioFormat().getEncoding();
}

AudioFormat AudioStream::getAudioFormat() {
    return AudioFormat::UNKNOWN;
}

std::vector<AudioStreamProperty> AudioStream::getProperties() {
    return {};
}

AudioStream::MediaType AudioStream::getMediaType() {
    return MediaType::UNKNOWN;
}

}  // namespace audio
}  // namespace aace
