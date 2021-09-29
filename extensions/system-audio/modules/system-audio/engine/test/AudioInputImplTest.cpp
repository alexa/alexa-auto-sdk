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
#include <AACE/Engine/SystemAudio/AudioInputImpl.h>
#include <AACE/Audio/AudioEngineInterfaces.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace aace {
namespace test {
namespace unit {

using namespace ::testing;
using namespace ::aace::audio;

class AudioInputImplTest : public Test {
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

struct MockAudioInputEngineInterface : public AudioInputEngineInterface {
    MOCK_METHOD2(write, ssize_t(const int16_t* data, size_t size));

    std::mutex mutex;
    std::condition_variable cv;
    size_t writtenBytes;

    void setupDefaultExpectation() {
        writtenBytes = 0;
        EXPECT_CALL(*this, write(_, _)).WillRepeatedly(Invoke([this](const int16_t* data, size_t size) -> ssize_t {
            std::unique_lock<std::mutex> lock(mutex);
            writtenBytes += size;
            cv.notify_one();
            return (ssize_t)size;
        }));
    }

    void waitUntilBytesWritten(size_t bytes) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this, bytes] { return writtenBytes >= bytes; });
    }
};

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioInputImplTest, createByModuleId) {
    auto ai = engine::systemAudio::AudioInputImpl::create(-1, "", 0, "AudioInputUnderTest");
    ASSERT_TRUE(ai == nullptr);

    ai = engine::systemAudio::AudioInputImpl::create(0, "", 0, "AudioInputUnderTest");
    ASSERT_TRUE(ai != nullptr);
}

#ifdef TEST_AAL_MODULES

// NOLINTNEXTLINE(cert-err58-cpp)
TEST_F(AudioInputImplTest, startAudioInput) {
    auto ai = engine::systemAudio::AudioInputImpl::create(0, "", 0, "AudioInputUnderTest");
    ASSERT_TRUE(ai != nullptr);

    auto mockAIEI = std::make_shared<MockAudioInputEngineInterface>();
    mockAIEI->setupDefaultExpectation();
    ai->setEngineInterface(mockAIEI);

    ASSERT_TRUE(ai->startAudioInput());
    mockAIEI->waitUntilBytesWritten(16000 * 2);

    ASSERT_TRUE(ai->stopAudioInput());
}

// TODO: install a new AAL module for testing purpose

#endif  // TEST_AAL_MODULES

}  // namespace unit
}  // namespace test
}  // namespace aace
