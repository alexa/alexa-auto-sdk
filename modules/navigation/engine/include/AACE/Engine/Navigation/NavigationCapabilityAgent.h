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

#ifndef AACE_ENGINE_NAVIGATION_NAVIGATION_CAPABILITY_AGENT_H
#define AACE_ENGINE_NAVIGATION_NAVIGATION_CAPABILITY_AGENT_H

#include <memory>
#include <unordered_set>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>


#include "NavigationObserverInterface.h"

namespace aace {
namespace engine {
namespace navigation {

class NavigationCapabilityAgent :
    public alexaClientSDK::avsCommon::avs::CapabilityAgent,
    public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<NavigationCapabilityAgent> {
    
public:
    static std::shared_ptr<NavigationCapabilityAgent> create( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );

    /**
     * Destructor.
     */
    virtual ~NavigationCapabilityAgent() = default;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) override;
    void preHandleDirective( std::shared_ptr<DirectiveInfo> info ) override;
    void handleDirective( std::shared_ptr<DirectiveInfo> info ) override;
    void cancelDirective( std::shared_ptr<DirectiveInfo> info ) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @}};

    /**
     * This function adds an observer to @c NavigationCapabilityAgent so that it will get notified for 
     * @c SetDestination callbacks
     * 
     * @param [in] observer The @c NavigationObserverInterface
     */
    void addObserver( std::shared_ptr<NavigationObserverInterface> observer );

    /**
     * This function removes an observer from @c NavigationCapabilityAgent so that it will no longer be notified of
     * @c SetDestination callbacks.
     *
     * @param [in] observer The @c NavigationObserverInterface
     */
    void removeObserver( std::shared_ptr<NavigationObserverInterface> observer );

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> getCapabilityConfigurations() override;

private:
    NavigationCapabilityAgent( std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender );

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
     * This function handles a @c SetDestination directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleSetDestinationDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c SetDestination directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleCancelNavigationDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles any unknown directives received by the @c Navigation capability agent.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleUnknownDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * @name Executor Thread Variables
     *
     * These member variables are only accessed by functions in the @c m_executor worker thread, and do not require any
     * synchronization.
     */
    /// @{
    /// A set of observers to be notified when a @c SetDestination directive is received
    std::unordered_set<std::shared_ptr<NavigationObserverInterface>> m_observers;
    /// @}

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> m_capabilityConfigurations;

    /// This is the worker thread for the @c Navigation CA.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

} // aace::engine::navigation
} // aace::engine
} // aace

#endif // AACE_ENGINE_NAVIGATION_NAVIGATION_CAPABILITY_AGENT_H
