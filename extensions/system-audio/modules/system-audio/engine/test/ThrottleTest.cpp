/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <gtest/gtest.h>
#include "AACE/Engine/SystemAudio/Throttle.h"

#include <atomic>
#include <chrono>

using namespace aace::engine::systemAudio;

constexpr size_t default_frag_size = 320;

// NOLINTNEXTLINE(cert-err58-cpp)
TEST(ThrottleTest, writeNormalFragment) {
    int16_t buf[default_frag_size];
    Throttle<int16_t> t(default_frag_size, std::chrono::milliseconds(20), [](const int16_t* data, size_t length) {
        ASSERT_EQ(length, default_frag_size);
    });

    t.write(buf, default_frag_size);
}

// NOLINTNEXTLINE(cert-err58-cpp)
TEST(ThrottleTest, writeLargeFragment) {
    const int size_multiplier = 8;
    int16_t buf[default_frag_size * size_multiplier];
    for (int i = 0; i < size_multiplier; ++i) {
        buf[i * default_frag_size] = i;
    }
    std::atomic<int> write_count(0);

    Throttle<int16_t> t(
        default_frag_size, std::chrono::milliseconds(20), [&write_count](const int16_t* data, size_t length) {
            ASSERT_EQ(length, default_frag_size);
            ASSERT_EQ(data[0], write_count);
            ++write_count;
        });

    t.write(buf, default_frag_size * size_multiplier);
    std::this_thread::sleep_for(std::chrono::milliseconds(20 * size_multiplier));
    ASSERT_EQ(write_count, size_multiplier);
}

// NOLINTNEXTLINE(cert-err58-cpp)
TEST(ThrottleTest, writeIrregularlly) {
    const int buf_count = 8;
    const int size_multiplier = 4;
    const int frag_size = default_frag_size * size_multiplier;
    int16_t buf[frag_size];
    std::atomic<int> write_count(0);

    Throttle<int16_t> t(
        default_frag_size, std::chrono::milliseconds(20), [&write_count](const int16_t* data, size_t length) {
            ASSERT_EQ(length, default_frag_size);
            ASSERT_EQ(data[0], write_count);
            ++write_count;
        });

    int frag_count = 0;
    int sleep_count = 0;
    for (int i = 0; i < buf_count; ++i) {
        for (int j = 0; j < size_multiplier; ++j, ++frag_count) {
            buf[j * default_frag_size] = frag_count;
        }
        t.write(buf, frag_size);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        ++sleep_count;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20 * (frag_count - sleep_count)));
    ASSERT_EQ(write_count, buf_count * size_multiplier);
}
