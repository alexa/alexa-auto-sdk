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

#ifndef AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_CAPABILITY_AGENT_H
#define AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_CAPABILITY_AGENT_H

#include <memory>
#include <unordered_map>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageRequestObserverInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>
#include "CustomDomainHandlerInterface.h"

namespace aace {
namespace engine {
namespace customDomain {

class CustomDomainCapabilityAgent
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::MessageRequestObserverInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<CustomDomainCapabilityAgent> {
public:
    /**
     * Destructor.
     */
    virtual ~CustomDomainCapabilityAgent() = default;

    /**
     * Factory method to create a @c CustomDomainCapabilityAgent instance.
     *
     * @param interfaceNamespace The namespace of the interface.
     * @param interfaceVersion The version of the interface.
     * @param states The vector of the state names this interface reports in its context
     * @param customDomainHandler Handler to handle directives and getContext() requests.
     * @param exceptionSender Interface to report exceptions to AVS.
     * @param contextManager Interface to provide custom state to AVS.
     * @param messageSender Interface to send events to AVS.
     * @return A new instance of @c CustomDomainCapabilityAgent on success, @c nullptr otherwise.
     */
    static std::shared_ptr<CustomDomainCapabilityAgent> create(
        const std::string& interfaceNamespace,
        const std::string& interfaceVersion,
        const std::vector<std::string>& states,
        std::shared_ptr<CustomDomainHandlerInterface> customDomainHandler,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /// @name CapabilityAgent Functions
    /// @{
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<CapabilityAgent::DirectiveInfo> info) override;
    /// @}

    /// @name CapabilityConfigurationInterface method
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name RequiresShutdown method
    /// @{
    void doShutdown() override;
    /// @}

    /// @name StateProviderInterface method
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::CapabilityTag& stateProviderName,
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken contextRequestToken) override;
    bool canStateBeRetrieved() override;
    /// @}

    /// @name MessageRequestObserverInterface methods
    /// @{
    void onSendCompleted(MessageRequestObserverInterface::Status status) override;
    void onExceptionReceived(const std::string& exceptionMessage) override;
    /// @}

    /// @name ContextRequesterInterface methods
    /// @{
    void onContextAvailable(
        const alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointIdentifier& endpointId,
        const alexaClientSDK::avsCommon::avs::AVSContext& endpointContext,
        alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken requestToken) override;
    void onContextFailure(
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestError error,
        alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken token) override;
    /// @}

    /**
     * If the event does not require context, sends the event directly.
     * Otherwise, retrieve or query the context.
     * @param name The name of the event.
     * @param payload An opaque JSON payload sent to the cloud with the event.
     * @param requiresContext A boolean indicating if this event must be sent with context.
     * @param correlationToken An opaque token that must be included in any events responding to previous directives.
     * @param customContext A string of the context of this interface. 
     */
    void sendEvent(
        const std::string& name,
        const std::string& payload,
        bool requiresContext,
        const std::string& correlationToken = "",
        const std::string& customContext = "");

    /**
     * Callback method on the result of directive handling from platform.
     * @param messageId An ID that uniquely identifies the directive.
     * @param succeeded A boolean that indicates if the handling is successful.
     * @param errorType The type of the handling error. 
     */
    void reportDirectiveHandlingResult(
        const std::string& messageId,
        bool succeeded,
        alexaClientSDK::avsCommon::avs::ExceptionErrorType errorType =
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);

private:
    /**
     * Constructor.
     * @param exceptionSender Interface to report exceptions to AVS.
     * @param contextManager Interface to provide state to AVS.
     * @param messageSender Interface to send events to AVS.
     * @param customDomainHandler Handler to handle the directives and getContext() requests.
     * @param interfaceNamespace The namespace of the capability interface.
     * @param interfaceVersion The version of the capability interface. 
     */
    CustomDomainCapabilityAgent(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<CustomDomainHandlerInterface> customDomainHandler,
        const std::string& interfaceNamespace,
        const std::string& interfaceVersion);

    /**
     * Initialize capability configuration & set provider for the states.
     * @param states The vector of the state names this interface reports in its context.
     */
    void initialize(const std::vector<std::string>& states);

    /**
     * Get context from device/cache conditionally and provide the state when requested.
     * @param stateProviderName The name of state provider
     * @param contextRequestToken The token of the current context request
     */
    void executeProvideState(
        const alexaClientSDK::avsCommon::avs::CapabilityTag& stateProviderName,
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken contextRequestToken);

    /**
     * Parse the given context in string and update the cache.
     * @param customContext The context for this namespace in string
     * @param contextRequestToken The token of the current context request
     */
    bool parseAndUpdateContext(
        const std::string& customContext,
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken contextRequestToken);

    /**
     * Check if the context is available in candidate cache.
     * @param contextRequestToken The token of the current context request
     */
    bool getCandidateContextIfAvailable(
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken contextRequestToken);

    /**
     * Remove a directive from the map of message IDs to @c DirectiveInfo instances.
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective whose message ID is to be removed.
     */
    void removeDirective(std::shared_ptr<DirectiveInfo> info);

    struct EventInfo {
        std::string name;
        std::string payload;
        std::string correlationToken;
    };

    /// The ContextManager object.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// The regular MessageSender object.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// Set of configurations of the capabilities implemented by this capability agent.
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// An executor used for serializing requests.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// Handler to handle directives and getContext() requests.
    std::shared_ptr<CustomDomainHandlerInterface> m_customDomainHandler;

    /// The version of the interface
    const std::string m_interfaceVersion;

    /** 
     * Maps CapabilityTag (representing a state name) to the corresponding state.
     */
    using StatesMap = std::
        unordered_map<alexaClientSDK::avsCommon::avs::CapabilityTag, alexaClientSDK::avsCommon::avs::CapabilityState>;

    using ContextRequestState = std::pair<alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken, StatesMap>;

    /**
     * Caches the context request token with its states being queried by Context Manager. It will be updated every time when Context Manager
     * queries a new context request token.
     */
    ContextRequestState m_stateProviderCache;

    /**
     * Maps context request tokens  to candidate context (sent with sendEvent() call) for this namespace. When querying context, this map
     * will be firstly checked before query from the device.
     */
    std::unordered_map<alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken, std::string>
        m_sendEventStateCache;

    /**
     * Maps context request tokens to pending event information in tuples. 
     * This map will be looked up in onContextAvailable() callback for sending the events.
     */
    std::unordered_map<alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken, EventInfo> m_pendingEvents;

    /**
     * Maps message Ids to in-progress directives.
     * This map will be looked up when reportDirectiveHandlingResult callback is called. 
     */
    std::unordered_map<std::string, std::shared_ptr<CapabilityAgent::DirectiveInfo>> m_pendingDirectives;

    /// Vector of the state names this interface reports in its context
    std::unordered_set<alexaClientSDK::avsCommon::avs::NamespaceAndName> m_states;
};

}  // namespace customDomain
}  // namespace engine
}  // namespace aace
#endif
