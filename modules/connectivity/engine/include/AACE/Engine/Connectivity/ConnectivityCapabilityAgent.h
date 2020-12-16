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

#ifndef AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_CAPABILITY_AGENT_H
#define AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_CAPABILITY_AGENT_H

#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/AVS/CapabilityState.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/Timing/Timer.h>

#include "AACE/Engine/Connectivity/AlexaConnectivityInterface.h"
#include "AACE/Engine/Connectivity/AlexaConnectivityListenerInterface.h"

namespace aace {
namespace engine {
namespace connectivity {

/**
 * The @c ConnectivityCapabilityAgent is responsible for handling 
 * Alexa.Networking.Connectivity directives and calls the @c AlexaConnectivityInterface APIs.
 *
 * This class implements a @c CapabilityAgent that handles the @c Alexa.Networking.AlexaConnectivityInterface.
 */
class ConnectivityCapabilityAgent
        : public aace::engine::connectivity::AlexaConnectivityListenerInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::sdkInterfaces::StateProviderInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<ConnectivityCapabilityAgent> {
public:
    /**
     * Create an instance of @c ConnectivityCapabilityAgent.
     *
     * @param connectivity An interface that this object will use to perform the internet data plan operations.
     * @param contextManager An interface to which this object will send property state updates.
     * @return @c nullptr if the inputs are invalid, else a new instance of @c ConnectivityCapabilityAgent.
     */
    static std::shared_ptr<ConnectivityCapabilityAgent> create(
        std::shared_ptr<AlexaConnectivityInterface> connectivity,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    /**
     * Get the connectivity capability configuration for this agent.
     *
     * @return The connectivity capability configuration.
     */
    alexaClientSDK::avsCommon::avs::CapabilityConfiguration getConnectivityCapabilityConfiguration();

    /**
     * Get the internet data plan capability configuration for this agent.
     *
     * @return The internet data plan capability configuration.
     */
    alexaClientSDK::avsCommon::avs::CapabilityConfiguration getInternetDataPlanCapabilityConfiguration();

    /// @name CapabilityConfigurationInterface Function
    /// @{
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;
    /// @}

    /// @name StateProviderInterface Functions
    /// @{
    void provideState(
        const alexaClientSDK::avsCommon::avs::CapabilityTag& stateProviderName,
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken contextRequestToken) override;
    bool canStateBeRetrieved() override;
    /// @}

    /// @name AlexaConnectivityListenerInterface Functions
    /// @{
    void onDataPlanStateChanged(
        const DataPlanState& dataPlanState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) override;
    void onDataPlansAvailableStateChanged(
        const DataPlansAvailableState& dataPlansAvailableState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) override;
    void onManagedProviderStateChanged(
        const ManagedProviderState& managedProviderState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) override;
    void onTermsStatusStateChanged(
        const TermsStatusState& termsStatusState,
        alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType cause) override;
    /// @}

private:
    /**
     * Constructor.
     *
     * @param connectivity An interface that this object will use to perform the internet data plan operations.
     * @param contextManager An interface to which this object will send property state updates.
     */
    ConnectivityCapabilityAgent(
        std::shared_ptr<AlexaConnectivityInterface> connectivity,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    // @name RequiresShutdown Function
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * Initializes the object.
     */
    bool initialize();

    /**
     * Gets the current state from endpoint and notifies @c ContextManager.
     *
     * @param stateProviderName Provides the property name and used in the @c ContextManager methods.
     * @param contextRequestToken The token to be used when providing the response to @c ContextManager.
     */
    void executeProvideState(
        const alexaClientSDK::avsCommon::avs::CapabilityTag& stateProviderName,
        const alexaClientSDK::avsCommon::sdkInterfaces::ContextRequestToken contextRequestToken);

    /**
     * Helper method to build the @c CapabilityState.
     *
     * @param dataPlanState The data plan state defined using @c DataPlanState.
     */
    alexaClientSDK::avsCommon::avs::CapabilityState buildCapabilityState(const DataPlanState& dataPlanState);

    /**
     * Helper method to build the @c CapabilityState.
     *
     * @param dataPlanState The data plans available state defined using @c DataPlansAvailableState.
     */
    alexaClientSDK::avsCommon::avs::CapabilityState buildCapabilityState(
        const DataPlansAvailableState& dataPlansAvailableState);

    /**
     * Helper method to build the @c CapabilityState.
     *
     * @param managedProviderState The managed provider state defined using @c ManagedProviderState.
     */
    alexaClientSDK::avsCommon::avs::CapabilityState buildCapabilityState(
        const ManagedProviderState& managedProviderState);

    /**
     * Helper method to build the @c CapabilityState.
     *
     * @param termsStatusState The terms status state defined using @c TermsStatusState.
     */
    alexaClientSDK::avsCommon::avs::CapabilityState buildCapabilityState(const TermsStatusState& termsStatusState);

    /// Reference to @c AlexaConnectivityInterface.
    std::shared_ptr<AlexaConnectivityInterface> m_connectivity;

    /// The @c ContextManager used to generate system context for events.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    /// Set of capability configurations that will get published using DCF
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// This is the worker thread for the @c ConnectivityCapabilityAgent.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace connectivity
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_CAPABILITY_AGENT_H
