/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_ALEXA_MOCK_SPEECH_RECOGNIZER_H
#define AACE_TEST_ALEXA_MOCK_SPEECH_RECOGNIZER_H

#include <AACE/Alexa/SpeechRecognizer.h>
#include <gtest/gtest.h>

namespace aace {
namespace test {
namespace alexa {

class MockSpeechRecognizer : public aace::alexa::SpeechRecognizer {
public:
    MockSpeechRecognizer() : aace::alexa::SpeechRecognizer() {
    }
    MOCK_METHOD1(wakewordDetected, bool(const std::string& wakeword));
    MOCK_METHOD0(endOfSpeechDetected, void());
};

}  // namespace alexa
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_ALEXA_MOCK_SPEECH_RECOGNIZER_H
