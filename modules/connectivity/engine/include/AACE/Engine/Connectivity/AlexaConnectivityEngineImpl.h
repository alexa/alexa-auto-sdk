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

#ifndef AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_ENGINE_IMPL_H
#define AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_ENGINE_IMPL_H

#include <mutex>
#include <utility>

#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <Endpoints/EndpointBuilder.h>
#include <nlohmann/json.hpp>

#include "AACE/Connectivity/AlexaConnectivity.h"
#include "AACE/Connectivity/AlexaConnectivityEngineInterface.h"
#include "AACE/Engine/Connectivity/AlexaConnectivityInterface.h"
#include "AACE/Engine/Connectivity/AlexaConnectivityListenerInterface.h"
#include "AACE/Engine/Connectivity/ConnectivityCapabilityAgent.h"

namespace aace {
namespace engine {
namespace connectivity {

/**
 * This handles the instantiation of the @c ConnectivityCapabilityAgent and the interaction 
 * with it for capability discovery, to synchronize state, and to send change reports.
 */
class AlexaConnectivityEngineImpl
        : public aace::engine::connectivity::AlexaConnectivityInterface
        , public aace::connectivity::AlexaConnectivityEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<AlexaConnectivityEngineImpl> {
private:
    /**
     * Constructor.
     */
    AlexaConnectivityEngineImpl(
        std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivityPlatformInterface);

    /**
     * Initialize the @c AlexaConnectivityEngineImpl and @c ConnectivityCapabilityAgent.
     */
    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        const std::string& networkIdentifier);

public:
    /**
     * Factory method for creating instance of @c AlexaConnectivityEngineImpl
     * which handles instantiation of @c ConnectivityCapabilityAgent.
     */
    static std::shared_ptr<AlexaConnectivityEngineImpl> create(
        std::shared_ptr<aace::connectivity::AlexaConnectivity> alexaConnectivityPlatformInterface,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        const std::string& networkIdentifier);

    /// @name AlexaConnectivityEngineInterface Function
    /// @{
    bool onConnectivityStateChange() override;
    /// @}

    /// @name AlexaConnectivityInterface Functions
    /// @{
    AlexaConnectivityInterface::DataPlan getDataPlan() const override;
    AlexaConnectivityInterface::DataPlansAvailable getDataPlansAvailable() const override;
    AlexaConnectivityInterface::ManagedProvider getManagedProvider() const override;
    AlexaConnectivityInterface::NetworkIdentifier getNetworkIdentifier() const override;
    AlexaConnectivityInterface::TermsStatus getTermsStatus() const override;
    /// @}

protected:
    /// @name RequiresShutdown Function
    /// @{
    void doShutdown() override;
    /// @}

private:
    /**
     * Helper method for @ updateConnectivityState to parse the data plan JSON object.
     *
     * Note: @ updateConnectivityState will catch any exceptions.
     */
    AlexaConnectivityInterface::DataPlan parseDataPlan(const nlohmann::json& document) const;

    /**
     * Helper method for @ updateConnectivityState to parse the data plans available JSON array.
     *
     * Note: @ updateConnectivityState will catch any exceptions.
     */
    AlexaConnectivityInterface::DataPlansAvailable parseDataPlansAvailable(const nlohmann::json& document) const;

    /**
     * Helper method for @ updateConnectivityState to parse the managed provider JSON object.
     *
     * Note: @ updateConnectivityState will catch any exceptions.
     */
    AlexaConnectivityInterface::ManagedProvider parseManagedProvider(const nlohmann::json& document) const;

    /**
     * Helper method for @ updateConnectivityState to parse the terms status JSON string.
     *
     * Note: @ updateConnectivityState will catch any exceptions.
     */
    AlexaConnectivityInterface::TermsStatus parseTermsStatus(const nlohmann::json& document) const;

    /**
     * Get the connectivity state JSON payload from the platform implementation on initialization
     * or after a call to @c connectivityStateChange. If the payload is empty, the current state is retained.
     *
     * Note: This method is called by @c initialize and @c onConnectivityStateChange.
     */
    bool updateConnectivityState();

    /// Auto SDK Alexa Connectivity platform interface handler instance.
    std::shared_ptr<aace::connectivity::AlexaConnectivity> m_alexaConnectivityPlatformInterface;

    /// Auto SDK Connectivity capability agent instance.
    std::shared_ptr<ConnectivityCapabilityAgent> m_connectivityCapabilityAgent;

    /// Mutex for properties.
    mutable std::mutex m_mutex;

    /// Configuration properties.
    AlexaConnectivityInterface::NetworkIdentifier m_networkIdentifier;

    /// Supported properties.
    AlexaConnectivityInterface::DataPlan m_dataPlan;
    AlexaConnectivityInterface::DataPlansAvailable m_dataPlansAvailable;
    AlexaConnectivityInterface::ManagedProvider m_managedProvider;
    AlexaConnectivityInterface::TermsStatus m_termsStatus;
};

}  // namespace connectivity
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CONNECTIVITY_CONNECTIVITY_ENGINE_IMPL_H
