/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_NAVIGATION_DISPLAY_MANAGER_CAPABILITY_AGENT_H
#define AACE_ENGINE_NAVIGATION_DISPLAY_MANAGER_CAPABILITY_AGENT_H

#include <memory>
#include <unordered_set>

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>

#include "DisplayHandlerInterface.h"

namespace aace {
namespace engine {
namespace navigation {

class DisplayManagerCapabilityAgent
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<DisplayManagerCapabilityAgent> {
public:
    static std::shared_ptr<DisplayManagerCapabilityAgent> create(
        std::shared_ptr<aace::engine::navigation::DisplayHandlerInterface> displayHandler,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    /**
     * Destructor.
     */
    virtual ~DisplayManagerCapabilityAgent() = default;

    /// @name CapabilityAgent functions.
    /// @{
    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    /// @};

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;

    /**
     * @c DisplayHandlerInterface successfully handled @c controlDisplay(). Send a @c ControlDisplaySucceeded event to
     * AVS.
     * 
     * @param displayMode The requested display control action that was completed on the device.
     */
    void controlDisplaySucceeded(aace::engine::navigation::DisplayMode displayMode);

    /**
     * @c DisplayHandlerInterface failed to handle @c controlDisplay(). Send a @c ControlDisplayFailed event to AVS.
     *
     * @param displayMode The requested display control action that could not be completed on the device.
     * @param error The reason the requested action was not completed.
     * @param description An optional description of the error.
     */
    void controlDisplayFailed(
        aace::engine::navigation::DisplayMode displayMode,
        aace::engine::navigation::DisplayControlError error,
        const std::string& description = "");

    /**
     * @c DisplayHandlerInterface successfully handled @c showAlternativeRoutes(). Send a
     * @c ShowAlternativeRoutesSucceeded event to AVS.
     *
     * @param queryType The type of alternate route that was requested and displayed to the user.
     * @param route Details of the displayed alternate route. Alexa may speak the route details to the user.
     */
    void showAlternativeRoutesSucceeded(
        aace::engine::navigation::AlternativeRoutesQueryType queryType,
        const aace::engine::navigation::AlternateRoute& route);

    /**
     * @c DisplayHandlerInterface failed to handle @c showAlternativeRoutes(). Send a @c ShowAlternativeRoutesFailed
     * event to AVS.
     *
     * @param queryType The type of alternate route that was requested but not displayed to the user.
     * @param error The reason the requested alternate route was not shown.
     * @param description An optional description of the error.
     */
    void showAlternativeRoutesFailed(
        aace::engine::navigation::AlternativeRoutesQueryType queryType,
        aace::engine::navigation::AlternativeRoutesQueryError error,
        const std::string& description = "");

private:
    DisplayManagerCapabilityAgent(
        std::shared_ptr<aace::engine::navigation::DisplayHandlerInterface> displayHandler,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager);

    // @name RequiresShutdown Functions
    /// @{
    void doShutdown() override;
    /// @}

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
     * This function handles a @c ControlDisplay directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleControlDisplayDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles a @c ShowAlternativeRoutes directive.
     *
     * @param [in] info The @c DirectiveInfo containing the @c AVSDirective and the @c DirectiveHandlerResultInterface.
     */
    void handleShowAlternativeRoutesDirective(std::shared_ptr<DirectiveInfo> info);

    /**
     * This function handles any unknown directives received by the @c DisplayManagerCapabilityAgent.
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
    /// A handler to be notified when a @c ControlDisplay or @c ShowAlternativeRoutes directive is received.
    std::shared_ptr<DisplayHandlerInterface> m_displayHandler;
    /// @}

    /// Set of capability configurations that will get published using the Capabilities API
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    /// This is the worker thread for the @c Navigation CA.
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;

    /// The @c ContextManager that needs to be updated of the state.
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;
};

}  // namespace navigation
}  // namespace engine
}  // namespace aace
#endif  // AACE_ENGINE_NAVIGATION_DISPLAY_MANAGER_CAPABILITY_AGENT_H
