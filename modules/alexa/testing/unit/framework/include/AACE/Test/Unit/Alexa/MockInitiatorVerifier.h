/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_AVS_MOCK_INITIATOR_VERIFIER_H
#define AACE_TEST_UNIT_AVS_MOCK_INITIATOR_VERIFIER_H

#include <AACE/Engine/Alexa/InitiatorVerifier.h>

namespace aace {
namespace test {
namespace unit {
namespace alexa {

class MockInitiatorVerifier : public aace::engine::alexa::InitiatorVerifier {
public:
    MOCK_METHOD2(shouldBlock, bool(const std::string& wakeword, const std::chrono::milliseconds& timeout));
    MOCK_METHOD1(shouldBlock, bool(const alexaClientSDK::capabilityAgents::aip::Initiator& initiator));
};

}  // namespace alexa
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_UNIT_AVS_MOCK_INITIATOR_VERIFIER_H
