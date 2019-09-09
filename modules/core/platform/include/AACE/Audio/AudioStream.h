/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/** @file */

namespace aace {
namespace audio {

class AudioStream {
public:
    enum class Encoding {
        UNKNOWN,
        LPCM,
        MP3,
        OPUS
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
    virtual ssize_t read( char* data, const size_t size ) = 0;
    
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
     */
    virtual Encoding getEncoding();
};

inline std::ostream& operator<<(std::ostream& stream, const AudioStream::Encoding& encoding) {
    switch (encoding) {
        case AudioStream::Encoding::UNKNOWN:
            stream << "UNKNOWN";
            break;
        case AudioStream::Encoding::LPCM:
            stream << "LPCM";
            break;
        case AudioStream::Encoding::MP3:
            stream << "MP3";
            break;
        case AudioStream::Encoding::OPUS:
            stream << "OPUS";
            break;
    }
    return stream;
}

} // aace::audio
} // aace

#endif // AACE_AUDIO_AUDIO_STREAM_H
