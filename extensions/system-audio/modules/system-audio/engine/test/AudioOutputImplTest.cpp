/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AACE/Engine/SystemAudio/AudioOutputImpl.h>
#include <AACE/Audio/AudioEngineInterfaces.h>

#include <aal/common.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>

namespace aace {
namespace test {
namespace unit {

using namespace ::testing;
using namespace ::aace::audio;

class AudioOutputImplTest : public Test {
public:
    void SetUp() override {
        auto aal_module_count = aal_get_module_count();
        ASSERT_TRUE(aal_module_count > 0);
        auto ok = aal_initialize(0);
        ASSERT_TRUE(ok);
    }

    void TearDown() override {
        aal_deinitialize(0);
    }
};

struct MockAudioOutputEngineInterface : public AudioOutputEngineInterface {
    MOCK_METHOD1(onMediaStateChanged, void(MediaState state));
    MOCK_METHOD2(onMediaError, void(MediaError error, const std::string& description));

    std::mutex mutex;
    std::condition_variable cv;
    AudioOutputEngineInterface::MediaState mediaState = AudioOutputEngineInterface::MediaState::STOPPED;

    void setupDefaultExpectation() {
        EXPECT_CALL(*this, onMediaStateChanged(_))
            .WillRepeatedly(Invoke([this](AudioOutputEngineInterface::MediaState state) {
                std::unique_lock<std::mutex> lock(mutex);
                mediaState = state;
                cv.notify_one();
            }));
        EXPECT_CALL(*this, onMediaError(_, _)).Times(Exactly(0));
    }

    void wait(MediaState expectedMediaState) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this, expectedMediaState] { return mediaState == expectedMediaState; });
    }
};

struct MockAudioStream : public AudioStream {
    MOCK_METHOD2(read, ssize_t(char* data, size_t size));
    MOCK_METHOD0(isClosed, bool());
    MOCK_METHOD0(getEncoding, AudioStream::Encoding());
    MOCK_METHOD0(getAudioFormat, AudioFormat());

    size_t streamSize;

    void setupAsSilentLPCM(int seconds) {
        AudioStream::AudioFormat audioFormat = {
            AudioFormat::Encoding::LPCM,
            AudioFormat::SampleFormat::SIGNED,
            AudioFormat::Layout::INTERLEAVED,
            AudioFormat::Endianness::LITTLE,
            16000,
            2 * CHAR_BIT,
            1,
        };

        streamSize = seconds * audioFormat.getSampleRate() * audioFormat.getSampleSize() / CHAR_BIT;
        EXPECT_CALL(*this, getEncoding()).WillRepeatedly(Return(audioFormat.getEncoding()));
        EXPECT_CALL(*this, getAudioFormat()).WillRepeatedly(Return(audioFormat));
        EXPECT_CALL(*this, read(_, _)).WillRepeatedly(Invoke([this](char* data, size_t size) -> ssize_t {
            size_t count = std::min(streamSize, size);
            std::memset(data, 0, count);
            streamSize -= count;
            return (ssize_t)count;
        }));
        EXPECT_CALL(*this, isClosed()).WillRepeatedly(Invoke([this]() { return streamSize <= 0; }));
    }

    void setupAsMp3Stream(uint8_t* streamBytes, size_t len) {
        AudioStream::AudioFormat audioFormat = {
            AudioFormat::Encoding::MP3,
            AudioFormat::SampleFormat::FLOAT,
            AudioFormat::Layout::INTERLEAVED,
            AudioFormat::Endianness::LITTLE,
            44100,
            2 * CHAR_BIT,
            1,
        };

        streamSize = len;
        EXPECT_CALL(*this, getEncoding()).WillRepeatedly(Return(audioFormat.getEncoding()));
        EXPECT_CALL(*this, getAudioFormat()).WillRepeatedly(Return(audioFormat));
        EXPECT_CALL(*this, read(_, _))
            .WillRepeatedly(Invoke([this, streamBytes, len](char* data, size_t size) -> ssize_t {
                size_t count = std::min(streamSize, size);
                std::memcpy(data, streamBytes + len - streamSize, count);
                streamSize -= count;
                return (ssize_t)count;
            }));
        EXPECT_CALL(*this, isClosed()).WillRepeatedly(Invoke([this]() { return streamSize <= 0; }));
    }
};

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, createByModuleId) {
    auto ao = engine::systemAudio::AudioOutputImpl::create(-1, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao == nullptr);

    ao = engine::systemAudio::AudioOutputImpl::create(0, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    ASSERT_EQ(ao->getDuration(), -1);
    ASSERT_EQ(ao->getNumBytesBuffered(), 0);
    ASSERT_EQ(ao->getPosition(), 0);
}

#ifdef TEST_AAL_MODULES

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, prepare) {
    auto ao = engine::systemAudio::AudioOutputImpl::create(0, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    ASSERT_TRUE(ao->prepare("https://ic1.sslstream.com/kore-ir", false));
    ASSERT_TRUE(ao->prepare("http://str0.creacast.com/topmusic_hq.mp3", false));
    ASSERT_TRUE(ao->prepare("http://stream.revma.ihrhls.com/zc2569", false));
    ASSERT_TRUE(ao->prepare("http://opml.radiotime.com/Tune.ashx?id=e71963667&sid=s35419&formats=aac,mp3", true));
    ASSERT_TRUE(ao->prepare("http://radiodeejay-lh.akamaihd.net/i/RadioDeejay_Live_1@189857/master.m3u8", false));
}

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, playBlankMP3) {
    auto ao = engine::systemAudio::AudioOutputImpl::create(0, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    auto mockAOEI = std::make_shared<MockAudioOutputEngineInterface>();
    mockAOEI->setupDefaultExpectation();
    ao->setEngineInterface(mockAOEI);

    ASSERT_TRUE(ao->prepare("https://cdn-embed.tunein.com/resources/media/blank.mp3", false));
    ASSERT_TRUE(ao->play());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);
    // wait until EOS
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);

    // Stop it again should make no harm
    ASSERT_TRUE(ao->stop());
}

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, playTuneIn) {
    auto ao = engine::systemAudio::AudioOutputImpl::create(0, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    auto mockAOEI = std::make_shared<MockAudioOutputEngineInterface>();
    mockAOEI->setupDefaultExpectation();
    ao->setEngineInterface(mockAOEI);

    ASSERT_TRUE(ao->prepare("http://opml.radiotime.com/Tune.ashx?id=e71963667&sid=s35419&formats=aac,mp3", false));
    ASSERT_TRUE(ao->play());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    ASSERT_TRUE(ao->pause());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);

    ASSERT_TRUE(ao->getPosition() >= 0);
    ASSERT_EQ(ao->getDuration(), 0);
    ASSERT_TRUE(ao->getNumBytesBuffered() >= 0);

    ASSERT_TRUE(ao->resume());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    ASSERT_TRUE(ao->stop());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);
}

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, playLpcmStream) {
    auto ao = engine::systemAudio::AudioOutputImpl::create(0, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    auto mockAOEI = std::make_shared<MockAudioOutputEngineInterface>();
    mockAOEI->setupDefaultExpectation();
    ao->setEngineInterface(mockAOEI);

    auto stream = std::make_shared<MockAudioStream>();
    stream->setupAsSilentLPCM(1);

    ASSERT_TRUE(ao->prepare(stream, false));
    ASSERT_TRUE(ao->play());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    ASSERT_TRUE(ao->pause());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);

    ASSERT_TRUE(ao->getPosition() >= 0);
    ASSERT_EQ(ao->getDuration(), -1);
    ASSERT_TRUE(ao->getNumBytesBuffered() >= 0);

    ASSERT_TRUE(ao->setPosition(0));
    ASSERT_TRUE(ao->getPosition() == 0);

    ASSERT_TRUE(ao->resume());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    // wait until EOS
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);
}

// generated by `ffmpeg -f lavfi -i anullsrc=r=44100:cl=mono -t 0.1 -q:a 9 -acodec libmp3lame -f mp3 - | xxd -i`
static unsigned char blank_mp3[] = {
    0x49, 0x44, 0x33, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x54, 0x53, 0x53, 0x45, 0x00, 0x00, 0x00, 0x0f, 0x00,
    0x00, 0x03, 0x4c, 0x61, 0x76, 0x66, 0x35, 0x38, 0x2e, 0x34, 0x35, 0x2e, 0x31, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfb, 0x40, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x49, 0x6e, 0x66, 0x6f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
    0x00, 0x05, 0x00, 0x00, 0x02, 0xbe, 0x00, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
    0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4,
    0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xb4, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda,
    0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xda, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x61, 0x76, 0x63,
    0x35, 0x38, 0x2e, 0x39, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x03,
    0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xbe, 0x10, 0xba, 0x9c, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0xfb, 0x10, 0xc4, 0x00, 0x03, 0xc0, 0x00, 0x01, 0xa4, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x34, 0x80, 0x00, 0x00,
    0x04, 0x4c, 0x41, 0x4d, 0x45, 0x33, 0x2e, 0x31, 0x30, 0x30, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xff, 0xfb, 0x10, 0xc4, 0x29, 0x83, 0xc0, 0x00, 0x01, 0xa4, 0x00,
    0x00, 0x00, 0x20, 0x00, 0x00, 0x34, 0x80, 0x00, 0x00, 0x04, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xff, 0xfb,
    0x10, 0xc4, 0x53, 0x03, 0xc0, 0x00, 0x01, 0xa4, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x34, 0x80, 0x00, 0x00, 0x04,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xff, 0xfb, 0x10, 0xc4, 0x7c, 0x83, 0xc0, 0x00, 0x01, 0xa4, 0x00, 0x00,
    0x00, 0x20, 0x00, 0x00, 0x34, 0x80, 0x00, 0x00, 0x04, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xff, 0xfb, 0x10,
    0xc4, 0xa6, 0x03, 0xc0, 0x00, 0x01, 0xa4, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x34, 0x80, 0x00, 0x00, 0x04, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, playMp3Stream) {
    auto ao = engine::systemAudio::AudioOutputImpl::create(0, "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    auto mockAOEI = std::make_shared<MockAudioOutputEngineInterface>();
    mockAOEI->setupDefaultExpectation();
    ao->setEngineInterface(mockAOEI);

    auto stream = std::make_shared<MockAudioStream>();
    stream->setupAsMp3Stream(blank_mp3, sizeof(blank_mp3));

    ASSERT_TRUE(ao->prepare(stream, false));
    ASSERT_TRUE(ao->play());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    ASSERT_TRUE(ao->pause());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);

    ASSERT_TRUE(ao->getPosition() >= 0);
    ASSERT_TRUE(ao->getDuration() > 0);
    ASSERT_TRUE(ao->getNumBytesBuffered() >= 0);

    ASSERT_TRUE(ao->setPosition(0));
    ASSERT_TRUE(ao->getPosition() == 0);

    ASSERT_TRUE(ao->resume());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    // wait until EOS
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);
}

#endif  // TEST_AAL_MODULES

class MockAAL {
    static aal_module_t module;
    static int module_id;

    struct Player {
        aal_player_ops_t ops{};
        aal_common_context_t context{};
        aal_attributes_t attributes{};
        aal_audio_parameters_t parameters{};

        virtual ~Player() = default;
        virtual void play() = 0;
        virtual void pause() = 0;
        virtual void stop() = 0;
        virtual int64_t get_position() = 0;
        virtual int64_t get_duration() = 0;
        virtual int64_t get_num_bytes_buffered() = 0;
        virtual void seek(int64_t position) = 0;
        virtual void set_volume(double volume) = 0;
        virtual void set_mute(bool mute) = 0;
        virtual ssize_t write(const char* data, size_t size) = 0;
        virtual void notify_end_of_stream() = 0;
        virtual void destroy() = 0;
    };
    static Player* player;

    struct Recorder {
        aal_recorder_ops_t ops{};
        aal_common_context_t context{};
        aal_attributes_t attributes{};
        aal_lpcm_parameters_t parameters{};

        virtual ~Recorder() = default;
        virtual void play() = 0;
        virtual void stop() = 0;
        virtual void destroy() = 0;
    };
    static Recorder* recorder;

    static int setUp(Player* mockPlayer, Recorder* mockRecorder) {
        player = mockPlayer;
        recorder = mockRecorder;

        module.name = "MockAAL";
        module.capabilities =
            AAL_MODULE_CAP_STREAM_PLAYBACK | AAL_MODULE_CAP_URL_PLAYBACK | AAL_MODULE_CAP_LPCM_PLAYBACK;
        module.initialize = nullptr;
        module.deinitialize = nullptr;

        player->ops.create = [](const aal_attributes_t* attr, aal_audio_parameters_t* params) -> aal_handle_t {
            if (attr) {
                player->attributes = *attr;
            }
            if (params) {
                player->parameters = *params;
            }
            return &player->context;
        };
        player->ops.play = [](aal_handle_t) { player->play(); };
        player->ops.pause = [](aal_handle_t) { player->pause(); };
        player->ops.stop = [](aal_handle_t) { player->stop(); };
        player->ops.get_position = [](aal_handle_t) -> int64_t { return player->get_position(); };
        player->ops.get_duration = [](aal_handle_t) -> int64_t { return player->get_duration(); };
        player->ops.get_num_bytes_buffered = [](aal_handle_t) -> int64_t { return player->get_num_bytes_buffered(); };
        player->ops.seek = [](aal_handle_t, int64_t position) { player->seek(position); };
        player->ops.set_volume = [](aal_handle_t, double volume) { player->set_volume(volume); };
        player->ops.set_mute = [](aal_handle_t, bool mute) { player->set_mute(mute); };
        player->ops.write = [](aal_handle_t, const char* data, size_t size) -> ssize_t {
            return player->write(data, size);
        };
        player->ops.notify_end_of_stream = [](aal_handle_t) { player->notify_end_of_stream(); };
        player->ops.destroy = [](aal_handle_t) { player->destroy(); };

        recorder->ops.create = [](const aal_attributes_t* attr, aal_lpcm_parameters_t* params) -> aal_handle_t {
            if (attr) {
                recorder->attributes = *attr;
            }
            if (params) {
                recorder->parameters = *params;
            }
            return &recorder->context;
        };
        recorder->ops.play = [](aal_handle_t) { recorder->play(); };
        recorder->ops.stop = [](aal_handle_t) { recorder->stop(); };
        recorder->ops.destroy = [](aal_handle_t) { recorder->destroy(); };

        module.player_ops = &player->ops;
        module.recorder_ops = &recorder->ops;

        module_id = aal_install_module(&module);
        aal_initialize(module_id);
        return module_id;
    }

    static void tearDown() {
        aal_deinitialize(module_id);
        aal_uninstall_module(&module);
    }

public:
    explicit MockAAL(Player* mockPlayer = nullptr, Recorder* mockRecorder = nullptr) {
        setUp(mockPlayer, mockRecorder);
    }

    static int moduleId() {
        return module_id;
    }

    ~MockAAL() {
        tearDown();
    }

    struct MockPlayer : Player {
        MOCK_METHOD0(play, void());
        MOCK_METHOD0(pause, void());
        MOCK_METHOD0(stop, void());
        MOCK_METHOD0(get_position, int64_t());
        MOCK_METHOD0(get_duration, int64_t());
        MOCK_METHOD0(get_num_bytes_buffered, int64_t());
        MOCK_METHOD1(seek, void(int64_t position));
        MOCK_METHOD1(set_volume, void(double volume));
        MOCK_METHOD1(set_mute, void(bool mute));
        MOCK_METHOD2(write, ssize_t(const char* data, size_t size));
        MOCK_METHOD0(notify_end_of_stream, void());
        MOCK_METHOD0(destroy, void());
    };

    struct MockRecorder : Recorder {
        MOCK_METHOD0(play, void());
        MOCK_METHOD0(stop, void());
        MOCK_METHOD0(destroy, void());
    };
};

aal_module_t MockAAL::module;
int MockAAL::module_id;
MockAAL::Player* MockAAL::player = nullptr;
MockAAL::Recorder* MockAAL::recorder = nullptr;

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioOutputImplTest, takeLongTimeToStart) {
    MockAAL::MockPlayer player;
    EXPECT_CALL(player, set_volume(0.5)).Times(Exactly(1));
    EXPECT_CALL(player, set_mute(false)).Times(Exactly(1));
    EXPECT_CALL(player, destroy()).Times(Exactly(1));
    EXPECT_CALL(player, play()).WillRepeatedly(Invoke([&player] {
        auto future = std::async([&player]() {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            player.context.listener->on_start(player.context.user_data);
        });
        future.get();
    }));
    EXPECT_CALL(player, pause()).WillRepeatedly(Invoke([&player] {
        auto future =
            std::async([&player]() { player.context.listener->on_stop(AAL_SUCCESS, player.context.user_data); });
        future.get();
    }));
    EXPECT_CALL(player, stop()).WillRepeatedly(Invoke([&player] {
        auto future =
            std::async([&player]() { player.context.listener->on_stop(AAL_SUCCESS, player.context.user_data); });
        future.get();
    }));

    MockAAL::MockRecorder recorder;
    MockAAL mockAAL(&player, &recorder);

    auto ao = engine::systemAudio::AudioOutputImpl::create(MockAAL::moduleId(), "", "AudioOutputUnderTest");
    ASSERT_TRUE(ao != nullptr);

    auto mockAOEI = std::make_shared<MockAudioOutputEngineInterface>();
    mockAOEI->setupDefaultExpectation();
    ao->setEngineInterface(mockAOEI);

    ASSERT_TRUE(ao->prepare("url", false));
    ASSERT_TRUE(ao->play());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    ASSERT_TRUE(ao->pause());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);

    ASSERT_TRUE(ao->resume());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::PLAYING);

    ASSERT_TRUE(ao->stop());
    mockAOEI->wait(AudioOutputEngineInterface::MediaState::STOPPED);
}

}  // namespace unit
}  // namespace test
}  // namespace aace
