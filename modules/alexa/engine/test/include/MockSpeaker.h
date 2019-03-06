/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_MOCK_SPEAKER_H_
#define AACE_TEST_UNIT_MOCK_SPEAKER_H_

#include "AACE/Alexa/Speaker.h"

namespace aace {    
namespace test {
namespace unit {

class MockSpeaker : public aace::alexa::Speaker {
public:
    MOCK_METHOD1(setVolume,
        bool(int8_t volume));
    MOCK_METHOD1(adjustVolume,
        bool(int8_t delta));
    MOCK_METHOD1(setMute,
        bool(bool mute ));
    MOCK_METHOD0(getVolume,
        int8_t());
    MOCK_METHOD0(isMuted,
        bool());
};

}
}
}
#endif //AACE_TEST_UNIT_MOCK_SPEAKER_H_