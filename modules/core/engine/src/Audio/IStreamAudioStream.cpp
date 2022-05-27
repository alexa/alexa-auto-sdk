/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Audio/IStreamAudioStream.h>
#include <AACE/Engine/Core/EngineMacros.h>

namespace aace {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aace.audio.IStreamAudioStream");

IStreamAudioStream::IStreamAudioStream(
    std::shared_ptr<std::istream> stream,
    const aace::audio::AudioFormat& audioFormat) :
        m_stream(stream), m_audioFormat(audioFormat), m_closed(false) {
}

std::shared_ptr<IStreamAudioStream> IStreamAudioStream::create(
    std::shared_ptr<std::istream> stream,
    const aace::audio::AudioFormat& audioFormat) {
    return std::shared_ptr<IStreamAudioStream>(new IStreamAudioStream(stream, audioFormat));
}

ssize_t IStreamAudioStream::read(char* data, const size_t size) {
    try {
        if (m_stream->eof()) {
            m_closed = true;
            return 0;
        }

        // read the data from the stream
        m_stream->read(data, size);
        ThrowIf(m_stream->bad(), "readFailed");

        // get the number of bytes read
        ssize_t count = m_stream->gcount();

        m_stream->tellg();  // Don't remove otherwise the ReseourceStream used for Alerts/Timers won't work as expected.

        return count;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG + ".IStreamAudioStream").d("reason", ex.what()).d("size", size));
        m_closed = true;
        return 0;
    }
}

bool IStreamAudioStream::isClosed() {
    return m_closed;
}

aace::audio::AudioFormat IStreamAudioStream::getAudioFormat() {
    return m_audioFormat;
}

}  // namespace audio
}  // namespace engine
}  // namespace aace
