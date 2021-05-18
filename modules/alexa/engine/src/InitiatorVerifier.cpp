/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <memory>
#include <chrono>

#include "AACE/Engine/Alexa/InitiatorVerifier.h"

namespace aace {
namespace engine {
namespace alexa {

static const std::string TAG("aace.alexa.InitiatorVerifier");

InitiatorVerifier::~InitiatorVerifier() = default;  // key function

bool InitiatorVerifier::shouldBlock(const std::string& wakeword, const std::chrono::milliseconds& timeout) {
    // Should not block by default if this function is not implemented
    return false;
}

bool InitiatorVerifier::shouldBlock(const alexaClientSDK::capabilityAgents::aip::Initiator& initiator) {
    // Should not block by default if this function is not implemented
    return false;
}

}  // namespace alexa
}  // namespace engine
}  // namespace aace
