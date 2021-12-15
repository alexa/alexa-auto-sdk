/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
    AudioStreamBinder(std::shared_ptr<aace::audio::AudioStream> stream);

    std::shared_ptr<aace::audio::AudioStream> getAudioStream() {
        return m_stream;
    }

private:
    std::shared_ptr<aace::audio::AudioStream> m_stream;
};

//
// JAudioStreamEncoding
//

class JAudioStreamEncodingConfig : public EnumConfiguration<aace::audio::AudioStream::Encoding> {
public:
    using T = aace::audio::AudioStream::Encoding;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioStream$Encoding";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNKNOWN, "UNKNOWN"}, {T::LPCM, "LPCM"}, {T::MP3, "MP3"}, {T::OPUS, "OPUS"}};
    }
};

using JAudioStreamEncoding = JEnum<aace::audio::AudioStream::Encoding, JAudioStreamEncodingConfig>;

//
// JAudioFormatEncoding
//

class JAudioFormatEncodingConfig : public EnumConfiguration<aace::audio::AudioFormat::Encoding> {
public:
    using T = aace::audio::AudioFormat::Encoding;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioFormat$Encoding";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNKNOWN, "UNKNOWN"}, {T::LPCM, "LPCM"}, {T::MP3, "MP3"}, {T::OPUS, "OPUS"}};
    }
};

using JAudioFormatEncoding = JEnum<aace::audio::AudioFormat::Encoding, JAudioFormatEncodingConfig>;

//
// JAudioFormatSampleFormat
//

class JAudioFormatSampleFormatConfig : public EnumConfiguration<aace::audio::AudioFormat::SampleFormat> {
public:
    using T = aace::audio::AudioFormat::SampleFormat;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioFormat$SampleFormat";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNKNOWN, "UNKNOWN"}, {T::SIGNED, "SIGNED"}, {T::UNSIGNED, "UNSIGNED"}, {T::FLOAT, "FLOAT"}};
    }
};

using JAudioFormatSampleFormat = JEnum<aace::audio::AudioFormat::SampleFormat, JAudioFormatSampleFormatConfig>;

//
// JAudioFormatLayout
//

class JAudioFormatLayoutConfig : public EnumConfiguration<aace::audio::AudioFormat::Layout> {
public:
    using T = aace::audio::AudioFormat::Layout;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioFormat$Layout";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNKNOWN, "UNKNOWN"}, {T::NON_INTERLEAVED, "NON_INTERLEAVED"}, {T::INTERLEAVED, "INTERLEAVED"}};
    }
};

using JAudioFormatLayout = JEnum<aace::audio::AudioFormat::Layout, JAudioFormatLayoutConfig>;

//
// JAudioFormatEndianness
//

class JAudioFormatEndiannessConfig : public EnumConfiguration<aace::audio::AudioFormat::Endianness> {
public:
    using T = aace::audio::AudioFormat::Endianness;

    const char* getClassName() override {
        return "com/amazon/aace/audio/AudioFormat$Endianness";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::UNKNOWN, "UNKNOWN"}, {T::LITTLE, "LITTLE"}, {T::BIG, "BIG"}};
    }
};

using JAudioFormatEndianness = JEnum<aace::audio::AudioFormat::Endianness, JAudioFormatEndiannessConfig>;

}  // namespace audio
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_AUDIO_AUDIO_STREAM_BINDER_H
