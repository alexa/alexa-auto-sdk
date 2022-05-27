/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_ALEXA_MOCK_LOCAL_MEDIA_SOURCE_H
#define AACE_TEST_UNIT_ALEXA_MOCK_LOCAL_MEDIA_SOURCE_H

#include <AACE/Alexa/LocalMediaSource.h>
#include <gtest/gtest.h>

namespace aace {
namespace test {
namespace unit {
namespace alexa {

class MockLocalMediaSource : public aace::alexa::LocalMediaSource {
public:
    MockLocalMediaSource(Source source) : LocalMediaSource(source) {
    }

    MOCK_METHOD1(playControl, bool(PlayControlType controlType));
    MOCK_METHOD1(seek, bool(std::chrono::milliseconds offset));
    MOCK_METHOD1(adjustSeek, bool(std::chrono::milliseconds deltaOffset));
    MOCK_METHOD0(getState, LocalMediaSourceState());
    MOCK_METHOD1(volumeChanged, bool(float volume));
    MOCK_METHOD1(mutedStateChanged, bool(MutedState state));
};

}  // namespace alexa
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_UNIT_ALEXA_MOCK_LOCAL_MEDIA_SOURCE_H
