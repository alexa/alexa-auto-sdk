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

#ifndef AACE_AUDIO_AUDIO_STREAM_H
#define AACE_AUDIO_AUDIO_STREAM_H

#include <iostream>
#include <vector>
#include <string>

#include "AudioFormat.h"

/** @file */

namespace aace {
namespace audio {

class AudioStreamProperty;

class AudioStream {
public:
    using AudioFormat = aace::audio::AudioFormat;
    using Encoding = AudioFormat::Encoding;

    /**
     * An enum representing @c MediaType value.
     */
    enum class MediaType {
        /// MPEG media type.
        MPEG,

        /// WAV media type.
        WAV,

        /// The media type is unknown.
        UNKNOWN
    };

    virtual ~AudioStream();

    /**
     * Reads audio data from the strean when available. Audio data will be
     * available while @c isClosed() returns false.
     *
     * @param [out] data The buffer where audio data should be copied
     * @param [in] size The size of the buffer
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    virtual ssize_t read(char* data, const size_t size) = 0;

    /**
     * Checks if the audio stream from the no more data available to read.
     *
     * @return @c true if the audio stream is closed, @c false if more data
     * will be available
     */

    virtual bool isClosed() = 0;

    /**
     * Returns the encoding format of the @c AudioStream. If the encoding is not known
     * then @c Encoding::UNKNOWN will be returned.
     *
     * @return @c Encoding format of the @c AudioStream
     * @deprecated Use @c getAudioFormat to return the audio stream encoding and additional
     * audio format properties.
     */
    virtual Encoding getEncoding();

    /**
     * Returns the audio format properties of this stream, including the audio encoding type.
     *
     * @return @c AudioFormat specified for this stream.
     */
    virtual AudioFormat getAudioFormat();

    /**
     * Returns the media type of the @c AudioStream. If the type is not known
     * then @c MediaType::UNKNOWN will be returned.
     *
     * @return @c MediaType specified for this stream.
     */
    virtual MediaType getMediaType();

    /**
     * Returns optional meta-data properties for the @c AudioStream.
     *
     * @return List of meta-data properties for the @c AudioStream.
     */
    virtual std::vector<AudioStreamProperty> getProperties();
};

/**
 * Key/value type for providing meta-data with the AudioStream.
 */
class AudioStreamProperty {
public:
    /**
     * AudioStreamProperty constructor
     *
     * @param  key The property key
     * @param  value The property value
     */
    AudioStreamProperty(const std::string& key, const std::string& value) : m_key{key}, m_value{value} {
    }

    /**
     * AudioStreamProperty copy constructor
     *
     * @param  prop The @c AudioStreamProperty being copied
     */
    AudioStreamProperty(const AudioStreamProperty& other) {
        *this = other;
    }

    /**
     * AudioStreamProperty copy constructor
     *
     * @param  other The @c AudioStreamProperty being assigned
     */
    AudioStreamProperty& operator=(const AudioStreamProperty& other) = default;

    /**
     * @return The property key
     */
    std::string getKey() {
        return m_key;
    }

    /**
     * @return The property value
     */
    std::string getValue() {
        return m_value;
    }

private:
    std::string m_key;
    std::string m_value;
};

}  // namespace audio
}  // namespace aace

#endif  // AACE_AUDIO_AUDIO_STREAM_H
