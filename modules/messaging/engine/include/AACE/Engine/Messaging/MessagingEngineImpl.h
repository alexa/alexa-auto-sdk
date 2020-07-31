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

#ifndef AACE_ENGINE_MESSAGING_MESSAGING_ENGINE_IMPL_H
#define AACE_ENGINE_MESSAGING_MESSAGING_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/Messaging/MessagingObserverInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <Endpoints/EndpointBuilder.h>
#include <Messaging/MessagingCapabilityAgent.h>

#include "AACE/Messaging/Messaging.h"
#include "AACE/Messaging/MessagingEngineInterface.h"

namespace aace {
namespace engine {
namespace messaging {

/**
 * Messaging endpoint.
 */
using MessagingEndpoint =
    alexaClientSDK::avsCommon::sdkInterfaces::messaging::MessagingObserverInterface::MessagingEndpoint;

/**
 * This handles the instantiation of the @c MessagingCapabilityAgent and the interaction 
 * with it to send events and receive directives. 
 */
class MessagingEngineImpl
        : public alexaClientSDK::avsCommon::sdkInterfaces::messaging::MessagingObserverInterface
        , public aace::messaging::MessagingEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<MessagingEngineImpl> {
private:
    /**
     * Constructor.
     */
    MessagingEngineImpl(std::shared_ptr<aace::messaging::Messaging> messagingPlatformInterface);

    /**
     * Initialize the @c MessagingEngineImpl and @c MessagingCapabilityAgent.
     */
    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

public:
    /**
     * Factory method for creating instance of @c MessagingEngineImpl which handles
     * instantiation of @c MessagingCapabilityAgent as well as adding itself as an observer
     * for messaging directives.
     */
    static std::shared_ptr<MessagingEngineImpl> create(
        std::shared_ptr<aace::messaging::Messaging> messagingPlatformInterface,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /// @name MessagingEngineInterface
    /// @{
    void onConversationsReport(const std::string& token, const std::string& conversations) override;
    void onSendMessageFailed(const std::string& token, ErrorCode code, const std::string& message) override;
    void onSendMessageSucceeded(const std::string& token) override;
    void onUpdateMessagesStatusFailed(const std::string& token, ErrorCode code, const std::string& message) override;
    void onUpdateMessagesStatusSucceeded(const std::string& token) override;
    void onUpdateMessagingEndpointState(
        ConnectionState connectionState,
        PermissionState sendPermission,
        PermissionState readPermission) override;
    /// @}

    /// @name MessagingObserverInterface
    /// @{
    void sendMessage(const std::string& token, MessagingEndpoint endpoint, const std::string& jsonPayload) override;
    void uploadConversations(const std::string& token, MessagingEndpoint endpoint, const std::string& jsonPayload)
        override;
    void updateMessagesStatus(const std::string& token, MessagingEndpoint endpoint, const std::string& jsonPayload)
        override;
    /// @}

protected:
    /// @name RequiresShutdown
    /// @{
    void doShutdown() override;
    /// @}

private:
    /**
     * Convert from @c ErrorCode to capability agent @c StatusErrorCode 
     * by explicitly comparing enum values instead of static casting.
     * 
     * @param code The code to be converted.
     */
    alexaClientSDK::capabilityAgents::messaging::MessagingCapabilityAgent::StatusErrorCode convertErrorCode(
        ErrorCode code);

    /**
     * Convert from @c ConnectionState to capability agent @c ConnectionState 
     * by explicitly comparing enum values instead of static casting.
     * 
     * @param state The code to be converted.
     */
    alexaClientSDK::capabilityAgents::messaging::MessagingCapabilityAgent::ConnectionState convertConnectionState(
        ConnectionState state);

    /**
     * Convert from @c PermissionState to capability agent @c PermissionState 
     * by explicitly comparing enum values instead of static casting.
     * 
     * @param permission The code to be converted.
     */
    alexaClientSDK::capabilityAgents::messaging::MessagingCapabilityAgent::PermissionState convertPermissionState(
        PermissionState permission);

    /// Auto SDK Messaging platform interface handler instance
    std::shared_ptr<aace::messaging::Messaging> m_messagingPlatformInterface;

    /// AVS MessagingCapabilityAgent instance
    std::shared_ptr<alexaClientSDK::capabilityAgents::messaging::MessagingCapabilityAgent> m_messagingCapabilityAgent;
};

}  // namespace messaging
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MESSAGING_MESSAGING_ENGINE_IMPL_H
