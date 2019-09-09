/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_AUDIO_AUDIO_STREAM_BINDER_H
#define AACE_JNI_AUDIO_AUDIO_STREAM_BINDER_H

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Audio/AudioStream.h>

namespace aace {
namespace jni {
namespace audio {

    //
    // AudioStreamBinder
    //

    class AudioStreamBinder {
    public:
        AudioStreamBinder( std::shared_ptr<aace::audio::AudioStream> stream );

        std::shared_ptr<aace::audio::AudioStream> getAudioStream() {
            return m_stream;
        }

    private:
        std::shared_ptr<aace::audio::AudioStream> m_stream;
    };

    //
    // JEncoding
    //

    class JEncodingConfig : public EnumConfiguration<aace::audio::AudioStream::Encoding> {
    public:
        using T = aace::audio::AudioStream::Encoding;

        const char* getClassName() override {
            return "com/amazon/aace/audio/AudioStream$Encoding";
        }

        std::vector<std::pair<T,std::string>> getConfiguration() override {
            return {
                {T::UNKNOWN,"UNKNOWN"},
                {T::LPCM,"LPCM"},
                {T::MP3,"MP3"},
                {T::OPUS,"OPUS"}
            };
        }
    };

    using JEncoding = JEnum<aace::audio::AudioStream::Encoding,JEncodingConfig>;

} // aace::jni::audio
} // aace::jni
} // aace

#endif // AACE_JNI_AUDIO_AUDIO_STREAM_BINDER_H
