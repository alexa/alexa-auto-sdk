/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_VPA_VPA_CAPABILITY_AGENT_H
#define AACE_ENGINE_VPA_VPA_CAPABILITY_AGENT_H

#include <memory>
#include <unordered_set>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>


#include "VPAObserverInterface.h"

namespace aace {
namespace engine {
namespace vpa {

class VPACapabilityAgent :
    public alexaClientSDK::avsCommon::avs::CapabilityAgent,
    public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<VPACapabilityAgent> {
    
public:
    static std::shared_ptr<VPACapabilityAgent> create(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender);

    /**
     * Destructor.
     */
    virtual ~VPACapabilityAgent() = default;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) override;
    void preHandleDirective( std::shared_ptr<DirectiveInfo> info ) override;
    void handleDirective( std::shared_ptr<DirectiveInfo> info ) override;
    void cancelDirective( std::shared_ptr<DirectiveInfo> info ) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @}};

    /**
     * @param [in] observer The @c VPAObserverInterface
     */
    void addObserver( std::shared_ptr<VPAObserverInterface> observer );

    /**
     * @param [in] observer The @c VPAObserverInterface
     */
    void removeObserver( std::shared_ptr<VPAObserverInterface> observer );

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> getCapabilityConfigurations() override;

private:
    VPACapabilityAgent( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );

    // @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

    /**
     * Send @c ExceptionEncountered and report a failure to handle the @c AVSDirective.
     *
     * @param [in] info The @c AVSDirective that encountered the error and ancillary information.
     * @param [in] type The type of Exception that was encountered.
     * @param [in] message The error message to include in the ExceptionEncountered message.
     */
    void sendExceptionEncounteredAndReportFailed(
        std::shared_ptr<DirectiveInfo> info,
        const std::string& message,
        alexaClientSDK::avsCommon::avs::ExceptionErrorType type = alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);

    /**
     * Remove a directive from the map of message IDs to @c DirectiveInfo instances.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective whose message ID is to be removed.
     */
    void removeDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * Send the handling completed notification and clean up the resources.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void setHandlingCompleted(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c SendDirective directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleSendDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * @name Executor Thread Variables
     *
     * These member variables are only accessed by functions in the @c m_executor worker thread, and do not require any
     * synchronization.
     */
    /// @{
    /// A set of observers to be notified when a @c SetDestination directive is received
    std::unordered_set<std::shared_ptr<VPAObserverInterface>> m_observers;
    /// @}

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> m_capabilityConfigurations;

    /// This is the worker thread for the @c VPA CA.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

} // aace::engine::VPA
} // aace::engine
} // aace

#endif // AACE_ENGINE_VPA_VPA_CAPABILITY_AGENT_H
