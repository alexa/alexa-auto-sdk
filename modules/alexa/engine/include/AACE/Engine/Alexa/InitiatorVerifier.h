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

#ifndef AACE_ENGINE_ALEXA_INITIATOR_VERIFIER_H
#define AACE_ENGINE_ALEXA_INITIATOR_VERIFIER_H

#include <memory>
#include <string>
#include <chrono>

#include <AIP/Initiator.h>

namespace aace {
namespace engine {
namespace alexa {

class InitiatorVerifier {
public:
    InitiatorVerifier() = default;
    virtual ~InitiatorVerifier();

    /**
     * Function used to verify if the detected wakeword should be blocked.
     * @param wakeword The wakeword being detected
     * @param timeout The timeout for the verification
     * @return Returns @c true if the wakeword should be blocked, @c false otherwise
     * @note The use of timeout should be cautious and it should have minimal impact to UPL.
     */
    virtual bool shouldBlock(const std::string& wakeword, const std::chrono::milliseconds& timeout);

    /**
     * Function used to verify if the initiator should be blocked.
     * @param initiator The initiator being used
     * @return Returns @c true if the initiator should be blocked, @c false otherwise
     */
    virtual bool shouldBlock(const alexaClientSDK::capabilityAgents::aip::Initiator& initiator);
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_INITIATOR_VERIFIER_H
