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

#include <AACE/Engine/Core/EngineMacros.h>

#include "AACE/Engine/Connectivity/ConnectivityMessageRequest.h"

namespace aace {
namespace engine {
namespace connectivity {

using namespace alexaClientSDK::avsCommon::avs;
using namespace alexaClientSDK::avsCommon::sdkInterfaces;

/// String to identify log entries originating from this file.
static const std::string TAG("aace.connectivity.ConnectivityMessageRequest");

/**
 * Helper function to determine whether the status is deemed successful.
 *
 * @param status The status from message sent.
 * @return @c true if successful, otherwise @c false.
 */
static bool isCompletedSuccessfully(MessageRequestObserverInterface::Status status) {
    switch (status) {
        case MessageRequestObserverInterface::Status::SUCCESS:
        case MessageRequestObserverInterface::Status::SUCCESS_ACCEPTED:
        case MessageRequestObserverInterface::Status::SUCCESS_NO_CONTENT:
            return true;
        case MessageRequestObserverInterface::Status::PENDING:
        case MessageRequestObserverInterface::Status::THROTTLED:
        case MessageRequestObserverInterface::Status::NOT_CONNECTED:
        case MessageRequestObserverInterface::Status::NOT_SYNCHRONIZED:
        case MessageRequestObserverInterface::Status::TIMEDOUT:
        case MessageRequestObserverInterface::Status::PROTOCOL_ERROR:
        case MessageRequestObserverInterface::Status::INTERNAL_ERROR:
        case MessageRequestObserverInterface::Status::SERVER_INTERNAL_ERROR_V2:
        case MessageRequestObserverInterface::Status::REFUSED:
        case MessageRequestObserverInterface::Status::CANCELED:
        case MessageRequestObserverInterface::Status::INVALID_AUTH:
        case MessageRequestObserverInterface::Status::BAD_REQUEST:
        case MessageRequestObserverInterface::Status::SERVER_OTHER_ERROR:
        default:
            return false;
    }
}

ConnectivityMessageRequest::ConnectivityMessageRequest(
    const std::string& jsonContent,
    std::promise<bool> messageCompletePromise) :
        MessageRequest{jsonContent},
        m_isPromiseSet{false},
        m_messageCompletePromise{std::move(messageCompletePromise)} {
}

void ConnectivityMessageRequest::sendCompleted(MessageRequestObserverInterface::Status status) {
    AACE_DEBUG(LX(TAG));
    MessageRequest::sendCompleted(status);

    if (!m_isPromiseSet.exchange(true)) {
        bool success = isCompletedSuccessfully(status);

        AACE_INFO(LX(TAG).d("status", status).d("success", success));
        m_messageCompletePromise.set_value(success);
    } else {
        // Should never be called multiple times.
        AACE_ERROR(LX(TAG).m("setPromiseFailed").d("reason", "promiseAlreadySet").d("status", status));
    }
}

}  // namespace connectivity
}  // namespace engine
}  // namespace aace
