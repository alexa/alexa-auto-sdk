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

#ifndef AACE_ENGINE_AUDIO_ISTREAM_AUDIO_STREAM_H
#define AACE_ENGINE_AUDIO_ISTREAM_AUDIO_STREAM_H

#include <memory>
#include <istream>

#include <AACE/Audio/AudioStream.h>

namespace aace {
namespace engine {
namespace audio {

class IStreamAudioStream : public aace::audio::AudioStream {
private:
    IStreamAudioStream(std::shared_ptr<std::istream> stream, const AudioFormat& audioFormat);

public:
    static std::shared_ptr<IStreamAudioStream> create(
        std::shared_ptr<std::istream> stream,
        const AudioFormat& audioFormat = aace::audio::AudioFormat::UNKNOWN);

    // aace::audio::AudioStream
    ssize_t read(char* data, const size_t size) override;
    bool isClosed() override;
    AudioFormat getAudioFormat() override;

private:
    std::shared_ptr<std::istream> m_stream;
    AudioFormat m_audioFormat;
    bool m_closed;
};

}  // namespace audio
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_AUDIO_ISTREAM_AUDIO_STREAM_H
