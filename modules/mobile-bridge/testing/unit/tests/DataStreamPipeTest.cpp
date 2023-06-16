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
#include <cstdint>
#include <thread>

#include "AACE/Engine/MobileBridge/DataStreamPipe.h"
#include "gmock/gmock-actions.h"
#include "gmock/gmock-spec-builders.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace aace {
namespace test {
namespace unit {
namespace mobileBridge {

using namespace aace::engine::mobileBridge;

class DataStreamPipeTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(DataStreamPipeTest, closeMultipleTimes) {
    constexpr size_t QUEUE_SIZE = 16;
    DataStreamPipe pipe(QUEUE_SIZE);

    pipe.close();
    ASSERT_EQ(pipe.size(), 0U);
    pipe.close();  // Can be closed multiple times
}

TEST_F(DataStreamPipeTest, usingClosedPipe) {
    constexpr size_t QUEUE_SIZE = 16;
    DataStreamPipe pipe(QUEUE_SIZE);

    pipe.close();
    EXPECT_ANY_THROW(pipe.getInput());
    EXPECT_ANY_THROW(pipe.getOutput());
    EXPECT_ANY_THROW(pipe.waitForAvailableBytes(0));
}

TEST_F(DataStreamPipeTest, nonblockingOperations) {
    constexpr size_t QUEUE_SIZE = 16;
    DataStreamPipe pipe(QUEUE_SIZE);

    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        output->writeByte(i);
    }

    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        auto v = input->readByte();
        ASSERT_EQ(v, static_cast<uint8_t>(i));
    }
}

TEST_F(DataStreamPipeTest, blockedProducer) {
    constexpr size_t QUEUE_SIZE = 4;
    DataStreamPipe pipe(QUEUE_SIZE);

    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    auto producer = std::thread([output]() {
        for (size_t i = 0; i < QUEUE_SIZE * 2; ++i) {
            output->writeByte(i);
        }
    });

    for (size_t i = 0; i < QUEUE_SIZE * 2; ++i) {
        auto v = input->readByte();
        ASSERT_EQ(v, static_cast<uint8_t>(i));
    }

    producer.join();
    pipe.close();
}

TEST_F(DataStreamPipeTest, abortBlockedProducer) {
    constexpr size_t QUEUE_SIZE = 4;
    DataStreamPipe pipe(QUEUE_SIZE);

    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    auto producer = std::thread([output]() {
        EXPECT_ANY_THROW({
            for (size_t i = 0; i < QUEUE_SIZE * 2; ++i) {
                // should be blocked when i >= QUEUE_SIZE and throw exception
                // when the pipe is closed.
                output->writeByte(i);
            }
        });
    });

    pipe.close();     // This should abort the waiting.
    producer.join();  // The producer thread should finish.
}

TEST_F(DataStreamPipeTest, blockedConsumer) {
    constexpr size_t QUEUE_SIZE = 4;
    DataStreamPipe pipe(QUEUE_SIZE);

    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    auto consumer = std::thread([input]() {
        for (size_t i = 0; i < QUEUE_SIZE * 2; ++i) {
            auto v = input->readByte();
            ASSERT_EQ(v, static_cast<uint8_t>(i));
        }
    });

    for (size_t i = 0; i < QUEUE_SIZE * 2; ++i) {
        output->writeByte(i);
    }

    consumer.join();
    pipe.close();
}

TEST_F(DataStreamPipeTest, abortBlockedConsumer) {
    constexpr size_t QUEUE_SIZE = 4;
    DataStreamPipe pipe(QUEUE_SIZE);

    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    auto consumer = std::thread([input]() {
        EXPECT_ANY_THROW({
            for (size_t i = 0; i < QUEUE_SIZE * 2; ++i) {
                auto v = input->readByte();
                ASSERT_EQ(v, static_cast<uint8_t>(i));
            }
        });
    });

    pipe.close();     // This should abort the waiting.
    consumer.join();  // The consumer thread should finish.
}

TEST_F(DataStreamPipeTest, readingDataLargerThanPipeCapacity) {
    constexpr size_t QUEUE_SIZE = 128;
    DataStreamPipe pipe(QUEUE_SIZE);

    auto input = pipe.getInput();
    auto output = pipe.getOutput();

    uint8_t outBuf[QUEUE_SIZE * 8];
    for (size_t i = 0; i < sizeof(outBuf); ++i) {
        outBuf[i] = static_cast<uint8_t>(i & 0xff);
    }

    auto consumer = std::thread([input, &outBuf]() {
        uint8_t inBuf[QUEUE_SIZE * 8];
        input->readFully(inBuf, sizeof(inBuf));
        for (size_t i = 0; i < sizeof(inBuf); ++i) {
            ASSERT_EQ(inBuf[i], outBuf[i]);
        }
    });

    output->writeBytes(outBuf, sizeof(outBuf));

    consumer.join();
}

}  // namespace mobileBridge
}  // namespace unit
}  // namespace test
}  // namespace aace
