/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_APL_APL_ENGINE_IMPL_H
#define AACE_ENGINE_APL_APL_ENGINE_IMPL_H

#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AlexaPresentation/AlexaPresentation.h>
#include <Endpoints/EndpointBuilder.h>
#include <SmartScreenSDKInterfaces/AlexaPresentationObserverInterface.h>
#include <SmartScreenSDKInterfaces/VisualStateProviderInterface.h>

#include "AACE/APL/APL.h"
#include "AACE/APL/APLEngineInterface.h"

namespace aace {
namespace engine {
namespace apl {

class APLEngineImpl
        : public alexaSmartScreenSDK::smartScreenSDKInterfaces::AlexaPresentationObserverInterface
        , public alexaSmartScreenSDK::smartScreenSDKInterfaces::VisualStateProviderInterface
        , public aace::apl::APLEngineInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<APLEngineImpl> {
private:
    APLEngineImpl(std::shared_ptr<aace::apl::APL> aplPlatformInterface);

    bool initialize(
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator);

public:
    static std::shared_ptr<APLEngineImpl> create(
        std::shared_ptr<aace::apl::APL> aplPlatformInterface,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> dialogUXStateAggregator);

    // AlexaPresentationObserverInterface
    virtual void renderDocument(const std::string& jsonPayload, const std::string& token, const std::string& windowId)
        override;
    virtual void clearDocument() override;
    virtual void executeCommands(const std::string& jsonPayload, const std::string& token) override;
    virtual void interruptCommandSequence() override;

    // VisualStateProviderInterface
    virtual void provideState(const unsigned int stateRequestToken) override;

    // APLEngineInterface
    virtual void onClearCard() override;
    virtual void onClearAllExecuteCommands() override;
    virtual void onSendUserEvent(const std::string& payload) override;
    virtual void onSetAPLMaxVersion(const std::string& aplMaxVersion) override;
    virtual void onSetDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout) override;
    virtual void onRenderDocumentResult(const std::string& token, bool result, const std::string& error) override;
    virtual void onExecuteCommandsResult(const std::string& token, bool result, const std::string& error) override;
    virtual void onProcessActivityEvent(const std::string& source, ActivityEvent event) override;

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::apl::APL> m_aplPlatformInterface;
    std::shared_ptr<alexaSmartScreenSDK::smartScreenCapabilityAgents::alexaPresentation::AlexaPresentation>
        m_aplCapabilityAgent;
};

}  // namespace apl
}  // namespace engine
}  // namespace aace

#endif
