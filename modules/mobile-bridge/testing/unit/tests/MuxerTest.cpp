/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <cstddef>
#include <thread>

#include "AACE/Engine/MobileBridge/DataStreamPipe.h"
#include "AACE/Engine/MobileBridge/Muxer.h"
#include "AACE/Engine/MobileBridge/Util.h"
#include "gmock/gmock-actions.h"
#include "gmock/gmock-spec-builders.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

using namespace aace::engine::mobileBridge;

class MuxerTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(MuxerTest, demuxErrorHandling) {
    Muxer muxer;

    EXPECT_ANY_THROW(muxer.demux(nullptr));
    auto emptyDataInput = std::make_shared<DataInputStreamUnique>(std::unique_ptr<uint8_t[]>(new uint8_t[0]), 0);
    EXPECT_ANY_THROW(muxer.demux(emptyDataInput));

    auto incompleteHeader =
        std::make_shared<DataInputStreamUnique>(std::unique_ptr<uint8_t[]>(new uint8_t[15]{'A', 'M', 'B', '1'}), 15);
    EXPECT_ANY_THROW(muxer.demux(incompleteHeader));

    auto incorrectMagic =
        std::make_shared<DataInputStreamUnique>(std::unique_ptr<uint8_t[]>(new uint8_t[16]{'A', 'M', 'B', '0'}), 16);
    EXPECT_ANY_THROW(muxer.demux(incorrectMagic));

    auto incorrectLength = std::make_shared<DataInputStreamUnique>(
        std::unique_ptr<uint8_t[]>(new uint8_t[16]{'A', 'M', 'B', '1', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}), 16);
    EXPECT_ANY_THROW(muxer.demux(incorrectLength));
}

TEST_F(MuxerTest, muxErrorHandling) {
    Muxer muxer;

    EXPECT_ANY_THROW(muxer.muxTo(nullptr, 0, 0, nullptr, 0, 0));

    auto emptyDataOutput = std::make_shared<DataOutputStreamUnique>(0);
    EXPECT_ANY_THROW(muxer.muxTo(emptyDataOutput, 0, 0, nullptr, 0, 0));

    auto insufficientSpace = std::make_shared<DataOutputStreamUnique>(15);
    EXPECT_ANY_THROW(muxer.muxTo(insufficientSpace, 0, 0, nullptr, 0, 0));
}

TEST_F(MuxerTest, singleThreadDemuxing) {
    Muxer muxer;

    auto headerOnlyData = std::make_shared<DataInputStreamUnique>(
        std::unique_ptr<uint8_t[]>(new uint8_t[16]{'A', 'M', 'B', '1', 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 0}), 16);
    {
        auto frame = muxer.demux(headerOnlyData);
        ASSERT_EQ(frame.id, 0x01020304U);
        ASSERT_EQ(frame.flags, 0x05060708U);
        ASSERT_EQ(frame.len, 0U);
        ASSERT_EQ(frame.payload, nullptr);
    }

    auto testData1 = std::make_shared<DataInputStreamUnique>(
        std::unique_ptr<uint8_t[]>(
            new uint8_t[20]{'A', 'M', 'B', '1', 1, 2, 3, 4, 5, 6, 7, 8, 0, 0, 0, 4, 0x12, 0x34, 0x56, 0x78}),
        20);
    {
        auto frame = muxer.demux(testData1);
        ASSERT_EQ(frame.id, 0x01020304U);
        ASSERT_EQ(frame.flags, 0x05060708U);
        ASSERT_EQ(frame.len, 4U);
        ASSERT_EQ(frame.payload[0], 0x12U);
        ASSERT_EQ(frame.payload[1], 0x34U);
        ASSERT_EQ(frame.payload[2], 0x56U);
        ASSERT_EQ(frame.payload[3], 0x78U);
    }
}

TEST_F(MuxerTest, multiThreadDemuxing) {
    constexpr int FRAME_COUNT = 1024;
    constexpr uint32_t FRAME_FLAGS = 0x05060708U;
    constexpr size_t STREAM_SIZE = 1024;
    Muxer muxer;

    DataStreamPipe streamPipe(STREAM_SIZE);
    auto input = streamPipe.getInput();
    auto output = streamPipe.getOutput();

    auto producer = std::thread([output] {
        setThreadName("Producer");
        uint8_t magic[] = {'A', 'M', 'B', '1'};
        for (int i = 0; i < FRAME_COUNT; ++i) {
            output->writeBytes(magic, sizeof(magic));
            output->writeInt(i);            // id
            output->writeInt(FRAME_FLAGS);  // flags
            output->writeInt(0);            // len
        }
    });

    for (int i = 0; i < FRAME_COUNT; ++i) {
        auto frame = muxer.demux(input);
        ASSERT_EQ(frame.id, static_cast<uint32_t>(i));
        ASSERT_EQ(frame.flags, FRAME_FLAGS);
        ASSERT_EQ(frame.len, 0U);
        ASSERT_EQ(frame.payload, nullptr);
    }

    producer.join();
}

TEST_F(MuxerTest, singleThreadMuxing) {
    constexpr uint32_t FRAME_ID = 0x01234567;
    constexpr uint32_t FRAME_FLAGS = 0x89abcdef;
    constexpr int FRAME_SIZE = 16;
    Muxer muxer;

    auto output = std::make_shared<DataOutputStreamUnique>(FRAME_SIZE);
    muxer.muxTo(output, FRAME_ID, FRAME_FLAGS, nullptr, 0, 0);
    auto data = output->detach();
    ASSERT_EQ(data[0], 'A');
    ASSERT_EQ(data[1], 'M');
    ASSERT_EQ(data[2], 'B');
    ASSERT_EQ(data[3], '1');
    ASSERT_EQ(data[4], 0x01);
    ASSERT_EQ(data[5], 0x23);
    ASSERT_EQ(data[6], 0x45);
    ASSERT_EQ(data[7], 0x67);
    ASSERT_EQ(data[8], 0x89);
    ASSERT_EQ(data[9], 0xab);
    ASSERT_EQ(data[10], 0xcd);
    ASSERT_EQ(data[11], 0xef);
    ASSERT_EQ(data[12], 0);
    ASSERT_EQ(data[13], 0);
    ASSERT_EQ(data[14], 0);
    ASSERT_EQ(data[15], 0);

    auto input = std::make_shared<DataInputStreamUnique>(std::move(data), FRAME_SIZE);
    auto frame = muxer.demux(input);
    ASSERT_EQ(frame.id, FRAME_ID);
    ASSERT_EQ(frame.flags, FRAME_FLAGS);
}

TEST_F(MuxerTest, multiThreadMuxing) {
    constexpr int FRAME_COUNT = 1024;
    constexpr uint32_t FRAME_FLAGS = 0x89abcdefU;
    constexpr size_t STREAM_SIZE = 1024;
    Muxer muxer;

    DataStreamPipe pipe(STREAM_SIZE);
    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    auto producer = std::thread([&muxer, output] {
        setThreadName("Producer");
        for (int i = 0; i < FRAME_COUNT; ++i) {
            muxer.muxTo(output, static_cast<uint32_t>(i), FRAME_FLAGS, nullptr, 0, 0);
        }
    });

    for (int i = 0; i < FRAME_COUNT; ++i) {
        auto frame = muxer.demux(input);
        ASSERT_EQ(frame.id, static_cast<uint32_t>(i));
        ASSERT_EQ(frame.flags, FRAME_FLAGS);
        ASSERT_EQ(frame.len, 0U);
        ASSERT_EQ(frame.payload, nullptr);
    }

    producer.join();
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
