/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>

#include "NavigationHandlerInterface.h"

namespace aace {
namespace engine {
namespace navigation {

class NavigationCapabilityAgent
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<NavigationCapabilityAgent> {
public:
    static std::shared_ptr<NavigationCapabilityAgent> create(
        std::shared_ptr<aace::engine::navigation::NavigationHandlerInterface> navigationHandler,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        const std::string& navigationProviderName);

    /**
     * Destructor.
     */
    virtual ~NavigationCapabilityAgent() = default;

    /// @name CapabilityAgent/DirectiveHandlerInterface Functions
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @}};

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;

    // StateProviderInterface
    void provideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        const unsigned int stateRequestToken) override;

    /**
     * Send success event corresponding to the @c EventName
     *
     * @param [in] event The @c EventName corresponding to the type of event to be sent.
     */
    void navigationEvent(aace::navigation::NavigationEngineInterface::EventName event);

    /**
     * Send failure event corresponding to the @c ErrorType
     *
     * @param [in] type The @c ErrorType corresponding to the type of error to be sent.
     * @param [in] code The @c ErrorCode corresponding to the code of error to be sent.
     * @param [in] description The @c string corresponding to the description of error to be sent.
     */
    void navigationError(
        aace::navigation::NavigationEngineInterface::ErrorType type,
        aace::navigation::NavigationEngineInterface::ErrorCode code,
        const std::string& description);

private:
    NavigationCapabilityAgent(
        std::shared_ptr<aace::engine::navigation::NavigationHandlerInterface> navigationHandler,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        const std::string& navigationProviderName);

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
        alexaClientSDK::avsCommon::avs::ExceptionErrorType type =
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);

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
     * This function handles a @c startNavigation directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleStartNavigationDirective(std::shared_ptr<DirectiveInfo> info);
    /**
     * This function handles a @c ShowPreviousWaypoints directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleShowPreviousWaypointsDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c NavigateToPreviousWaypoint directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleNavigateToPreviousWaypointDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c CancelNavigation directive.
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
     * Exectuor function for provideState
     */
    void executeProvideState(
        const alexaClientSDK::avsCommon::avs::NamespaceAndName& stateProviderName,
        const unsigned int stateRequestToken);

    // Executor functions for navigation event handling
    void executeNavigationEvent(aace::navigation::NavigationEngineInterface::EventName event);
    void executeNavigationError(
        aace::navigation::NavigationEngineInterface::ErrorType type,
        aace::navigation::NavigationEngineInterface::ErrorCode code,
        const std::string& description);

    // Convert ErrorCode enum to string based on the error type
    std::string getNavigationErrorCode(aace::navigation::NavigationEngineInterface::ErrorCode code);
    std::string getWaypointErrorCode(aace::navigation::NavigationEngineInterface::ErrorCode code);

    // Sends event upon success event
    void startNavigationSuccess();
    void showPreviousWaypointsSuccess();
    void navigateToPreviousWaypointSuccess();

    // Sends event upon fail event
    void startNavigationError(std::string code, std::string description);
    void showPreviousWaypointsError(std::string code, std::string description);
    void navigateToPreviousWaypointError(std::string code, std::string description);

    /**
     * Check Navigation State for validity
     */
    bool isNavigationStateValid(std::string navigationState);

    /**
     * @name Executor Thread Variables
     *
     * These member variables are only accessed by functions in the @c m_executor worker thread, and do not require any
     * synchronization.
     */
    /// @{
    /// A set of observers to be notified when a @c StartNavigation directive is received
    std::shared_ptr<NavigationHandlerInterface> m_navigationHandler;
    /// @}

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// This is the worker thread for the @c Navigation CA.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// The @c ContextManager that needs to be updated of the state.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;

    /// The last known NavigationState payload
    std::string m_navigationStatePayload;
};

}  // namespace navigation
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_NAVIGATION_NAVIGATION_CAPABILITY_AGENT_H
