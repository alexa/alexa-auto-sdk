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

#ifndef AACE_TEST_AVS_MOCK_ALEXA_INTERFACE_MESSAGE_SENDER_INTERNAL_INTERFACE_H
#define AACE_TEST_AVS_MOCK_ALEXA_INTERFACE_MESSAGE_SENDER_INTERNAL_INTERFACE_H

#include <Alexa/AlexaInterfaceMessageSenderInternalInterface.h>

namespace aace {
namespace test {
namespace avs {

class MockAlexaInterfaceMessageSenderInternalInterface
        : public alexaClientSDK::capabilityAgents::alexa::AlexaInterfaceMessageSenderInternalInterface {
public:
    MOCK_METHOD3(
        sendStateReportEvent,
        bool(
            const std::string& instance,
            const std::string& correlationToken,
            const alexaClientSDK::avsCommon::avs::AVSMessageEndpoint& endpoint));
    MOCK_METHOD4(
        sendResponseEvent,
        bool(
            const std::string& instance,
            const std::string& correlationToken,
            const alexaClientSDK::avsCommon::avs::AVSMessageEndpoint& endpoint,
            const std::string& jsonPayload));
    MOCK_METHOD5(
        sendErrorResponseEvent,
        bool(
            const std::string& instance,
            const std::string& correlationToken,
            const alexaClientSDK::avsCommon::avs::AVSMessageEndpoint& endpoint,
            const ErrorResponseType errorType,
            const std::string& errorMessage));
    MOCK_METHOD3(
        sendDeferredResponseEvent,
        bool(const std::string& instance, const std::string& correlationToken, const int estimatedDeferralInSeconds));
    MOCK_METHOD1(
        alexaResponseTypeToErrorType,
        ErrorResponseType(const alexaClientSDK::avsCommon::avs::AlexaResponseType& responseType));
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  //AACE_TEST_AVS_MOCK_ALEXA_INTERFACE_MESSAGE_SENDER_INTERNAL_INTERFACE_H
