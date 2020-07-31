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

#ifndef AACE_AUDIO_AUDIO_FORMAT_H
#define AACE_AUDIO_AUDIO_FORMAT_H

#include <iostream>
#include <vector>
#include <string>

/** @file */

namespace aace {
namespace audio {

class AudioFormat {
public:
    enum class Encoding { UNKNOWN, LPCM, MP3, OPUS };

    enum class SampleFormat { UNKNOWN, SIGNED, UNSIGNED, FLOAT };

    enum class Layout { UNKNOWN, NON_INTERLEAVED, INTERLEAVED };

    enum class Endianness { UNKNOWN, LITTLE, BIG };

    static AudioFormat UNKNOWN;

private:
    AudioFormat() = default;

public:
    AudioFormat(
        Encoding encoding,
        SampleFormat sampleFormat,
        Layout layout,
        Endianness endianness,
        uint32_t sampleRate,
        uint8_t sampleSize,
        uint8_t channels);

    /**
     * Returns the encoding for the @c AudioFormat. If the encoding is not known
     * then @c Encoding::UNKNOWN will be returned.
     *
     * @return @c Encoding the @c AudioFormat
     */
    Encoding getEncoding();

    SampleFormat getSampleFormat();

    Layout getLayout();

    Endianness getEndianness();

    uint32_t getSampleRate();

    uint8_t getSampleSize();

    uint8_t getNumChannels();

private:
    Encoding m_encoding;
    SampleFormat m_sampleFormat;
    Layout m_layout;
    Endianness m_endianness;
    uint32_t m_sampleRate;
    uint8_t m_sampleSize;
    uint8_t m_channels;
};

inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Encoding& encoding) {
    switch (encoding) {
        case AudioFormat::Encoding::UNKNOWN:
            stream << "UNKNOWN";
            break;
        case AudioFormat::Encoding::LPCM:
            stream << "LPCM";
            break;
        case AudioFormat::Encoding::MP3:
            stream << "MP3";
            break;
        case AudioFormat::Encoding::OPUS:
            stream << "OPUS";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::SampleFormat& sampleFormat) {
    switch (sampleFormat) {
        case AudioFormat::SampleFormat::UNKNOWN:
            stream << "UNKNOWN";
            break;
        case AudioFormat::SampleFormat::SIGNED:
            stream << "SIGNED";
            break;
        case AudioFormat::SampleFormat::UNSIGNED:
            stream << "UNSIGNED";
            break;
        case AudioFormat::SampleFormat::FLOAT:
            stream << "FLOAT";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Layout& layout) {
    switch (layout) {
        case AudioFormat::Layout::UNKNOWN:
            stream << "UNKNOWN";
            break;
        case AudioFormat::Layout::NON_INTERLEAVED:
            stream << "NON_INTERLEAVED";
            break;
        case AudioFormat::Layout::INTERLEAVED:
            stream << "INTERLEAVED";
            break;
    }
    return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const AudioFormat::Endianness& endianness) {
    switch (endianness) {
        case AudioFormat::Endianness::UNKNOWN:
            stream << "UNKNOWN";
            break;
        case AudioFormat::Endianness::LITTLE:
            stream << "LITTLE";
            break;
        case AudioFormat::Endianness::BIG:
            stream << "BIG";
            break;
    }
    return stream;
}

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_FORMAT_H
