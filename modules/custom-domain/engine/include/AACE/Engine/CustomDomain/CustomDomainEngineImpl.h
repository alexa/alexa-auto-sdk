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

#ifndef AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_ENGINE_IMPL_H
#define AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesRegistrarInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>

#include "AACE/Engine/CustomDomain/CustomDomainCapabilityAgent.h"
#include "AACE/Engine/CustomDomain/CustomDomainHandlerInterface.h"
#include "AACE/CustomDomain/CustomDomain.h"
#include "AACE/CustomDomain/CustomDomainEngineInterface.h"
#include "AACE/Engine/Core/EngineService.h"

namespace aace {
namespace engine {
namespace customDomain {

/**
 * This handles the instantiation of the @c CustomDomainCapabilityAgent instances and the interaction 
 * with them to send custom events and receive custom directives. 
 */
class CustomDomainEngineImpl
        : public aace::engine::customDomain::CustomDomainHandlerInterface
        , public aace::customDomain::CustomDomainEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<CustomDomainEngineImpl> {
private:
    /**
     * Constructor.
     */
    CustomDomainEngineImpl(std::shared_ptr<aace::customDomain::CustomDomain> customDomainPlatformInterface);

    /**
     * Initializes the @c CustomDomainEngineImpl and @c CustomDomainCapabilityAgent. 
     * JSON Format for customInterfaceMetadata:
     * @code{.json}        
     * "interfaces": [
     *   {
     *      "namespace": {{String}},
     *      "version": {{String}},
     *      "states": [{{String}}, {{String}}, ...]      
     *   },
     *   {
     *      "namespace": {{String}},
     *      "version": {{String}},
     *      "states": [{{String}}, {{String}}, ...]       
     *   },
     *   ...
     * ]
     */
    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        const std::string& customInterfaceMetadata);

public:
    /**
     * Factory method for creating instance of @c CustomDomainEngineImpl which handles
     * instantiation of @c CustomDomainCapabilityAgent.
     */
    static std::shared_ptr<CustomDomainEngineImpl> create(
        std::shared_ptr<aace::customDomain::CustomDomain> customDomainPlatformInterface,
        std::shared_ptr<aace::engine::core::EngineContext> engineContext,
        const std::string& customInterfaceMetadata);

    /// @name CustomDomainEngineInterface Function
    /// @{
    void onSendEvent(
        const std::string& eventNamespace,
        const std::string& name,
        const std::string& payload,
        bool requiresContext,
        const std::string& correlationToken = "",
        const std::string& customContext = "") override;
    void onReportDirectiveHandlingResult(
        const std::string& directiveNamespace,
        const std::string& messageId,
        ResultType result) override;
    /// @}

    /// @name CustomDomainHandlerInterface
    /// @{
    void handleDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& payload,
        const std::string& correlationToken,
        const std::string& messageId) override;
    void cancelDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& correlationToken,
        const std::string& messageId) override;
    std::string getContext(const std::string& contextNamespace) override;
    /// @}

protected:
    /// @name RequiresShutdown
    /// @{
    void doShutdown() override;
    /// @}

private:
    /// Auto SDK Custom Domain platform interface handler instance.
    std::shared_ptr<aace::customDomain::CustomDomain> m_customDomainPlatformInterface;

    /// Map of namespace to Custom Domain Capability Agent instance
    std::unordered_map<std::string, std::shared_ptr<CustomDomainCapabilityAgent>> m_capabilityAgentMap;

    /**
     * Convert @c CustomDomainEngineInterface::ResultType to 
     * @c alexaClientSDK::avsCommon::avs::ExceptionErrorType
     * 
     * @param [in] result The handling result to be converted
     * @return The converted result in ExceptionErrorType
     */
    inline alexaClientSDK::avsCommon::avs::ExceptionErrorType convertErrorType(
        const CustomDomainEngineInterface::ResultType& result) {
        switch (result) {
            case CustomDomainEngineInterface::ResultType::UNEXPECTED_INFORMATION_RECEIVED:
                return alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNEXPECTED_INFORMATION_RECEIVED;
            case CustomDomainEngineInterface::ResultType::UNSUPPORTED_OPERATION:
                return alexaClientSDK::avsCommon::avs::ExceptionErrorType::UNSUPPORTED_OPERATION;
            case CustomDomainEngineInterface::ResultType::INTERNAL_ERROR:
                return alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR;
            default:
                break;
        }
        return alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR;
    }
};

}  // namespace customDomain
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_ENGINE_IMPL_H
