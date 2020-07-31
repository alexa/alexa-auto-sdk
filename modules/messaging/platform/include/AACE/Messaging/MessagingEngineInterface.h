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

#ifndef AACE_MESSAGING_MESSAGINGENGINEINTERFACE_H
#define AACE_MESSAGING_MESSAGINGENGINEINTERFACE_H

namespace aace {
namespace messaging {

class MessagingEngineInterface {
public:
    /**
     * An enum representing @c ErrorCode status error codes.
     */
    enum class ErrorCode {
        /// Generic failure occurred during request processing.
        GENERIC_FAILURE,

        /// No connection was found during request processing.
        NO_CONNECTIVITY,

        /// Messaging endpoint does not have needed permissions.
        NO_PERMISSION
    };

    /**
     * An enum representing the @c ConnectionState.
     */
    enum class ConnectionState {
        /// Messaging endpoint is disconnected.
        DISCONNECTED,

        /// Messaging endpoint is connected.
        CONNECTED
    };

    /**
     * An enum representing the @c Permission for @sa ConversationsReport.
     */
    enum class PermissionState {
        /// Permission is turned off.
        OFF,

        /// Permission is turned on.
        ON
    };

    virtual void onConversationsReport(const std::string& token, const std::string& conversations) = 0;
    virtual void onSendMessageFailed(const std::string& token, ErrorCode code, const std::string& message) = 0;
    virtual void onSendMessageSucceeded(const std::string& token) = 0;
    virtual void onUpdateMessagesStatusFailed(const std::string& token, ErrorCode code, const std::string& message) = 0;
    virtual void onUpdateMessagesStatusSucceeded(const std::string& token) = 0;
    virtual void onUpdateMessagingEndpointState(
        ConnectionState connectionState,
        PermissionState sendPermission,
        PermissionState readPermission) = 0;
};

}  // namespace messaging
}  // namespace aace

#endif  // AACE_MESSAGING_MESSAGINGENGINEINTERFACE_H
