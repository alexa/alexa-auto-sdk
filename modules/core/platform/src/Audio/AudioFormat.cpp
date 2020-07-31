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

#include <AACE/Audio/AudioFormat.h>

namespace aace {
namespace audio {

AudioFormat AudioFormat::UNKNOWN;

AudioFormat::AudioFormat(
    Encoding encoding,
    SampleFormat sampleFormat,
    Layout layout,
    Endianness endianness,
    uint32_t sampleRate,
    uint8_t sampleSize,
    uint8_t channels) :
        m_encoding{encoding},
        m_sampleFormat{sampleFormat},
        m_layout{layout},
        m_endianness{endianness},
        m_sampleRate{sampleRate},
        m_sampleSize{sampleSize},
        m_channels{channels} {
}

AudioFormat::Encoding AudioFormat::getEncoding() {
    return m_encoding;
}

AudioFormat::SampleFormat AudioFormat::getSampleFormat() {
    return m_sampleFormat;
}

AudioFormat::Layout AudioFormat::getLayout() {
    return m_layout;
}

AudioFormat::Endianness AudioFormat::getEndianness() {
    return m_endianness;
}

uint32_t AudioFormat::getSampleRate() {
    return m_sampleRate;
}

uint8_t AudioFormat::getSampleSize() {
    return m_sampleSize;
}

uint8_t AudioFormat::getNumChannels() {
    return m_channels;
}

}  // namespace audio
}  // namespace aace
