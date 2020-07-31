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

#ifndef AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_CAPABILITY_AGENT_H
#define AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_CAPABILITY_AGENT_H

#include <AVSCommon/AVS/CapabilityAgent.h>
#include <AVSCommon/AVS/CapabilityConfiguration.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>
#include <AVSCommon/Utils/UUIDGeneration/UUIDGeneration.h>

#include <AACE/PhoneCallController/PhoneCallControllerEngineInterfaces.h>
#include "PhoneCallControllerInterface.h"

namespace aace {
namespace engine {
namespace phoneCallController {

class PhoneCallControllerCapabilityAgent
        : public alexaClientSDK::avsCommon::avs::CapabilityAgent
        , public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<PhoneCallControllerCapabilityAgent> {
public:
    enum class CallState { IDLE, ACTIVE, TRYING, INBOUND_RINGING, OUTBOUND_RINGING, INVITED };

    enum class CallMethod { DIAL, REDIAL };

    static std::shared_ptr<PhoneCallControllerCapabilityAgent> create(
        std::shared_ptr<PhoneCallControllerInterface> phoneCallController,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

    virtual ~PhoneCallControllerCapabilityAgent() = default;

    void handleDirectiveImmediately(std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive) override;
    void preHandleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void handleDirective(std::shared_ptr<DirectiveInfo> info) override;
    void cancelDirective(std::shared_ptr<DirectiveInfo> info) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;
    void onFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior) override;

    void connectionStateChanged(aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state);
    void callStateChanged(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallState state,
        const std::string& callId,
        const std::string& callerId);
    void callFailed(
        const std::string& callId,
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallError code,
        const std::string& message);
    void callerIdReceived(const std::string& callId, const std::string& callerId);
    void sendDTMFSucceeded(const std::string& callId);
    void sendDTMFFailed(
        const std::string& callId,
        aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError code,
        const std::string& message);
    void deviceConfigurationUpdated(
        std::unordered_map<
            aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty,
            bool> configurationMap);
    std::string createCallId();

    CallState getCallState(std::string callId);
    bool callExist(std::string callId);

    static std::string connectionStateToString(
        aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state);
    static std::string callStateToString(CallState state);
    static std::string configurationFeatureToString(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty feature);
    static std::string callErrorToString(
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallError error);
    static std::string dtmfErrorToString(
        aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError error);

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
    getCapabilityConfigurations() override;

private:
    PhoneCallControllerCapabilityAgent(
        std::shared_ptr<PhoneCallControllerInterface> phoneCallController,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

    void doShutdown() override;

    void sendExceptionEncounteredAndReportFailed(
        std::shared_ptr<DirectiveInfo> info,
        const std::string& message,
        alexaClientSDK::avsCommon::avs::ExceptionErrorType type =
            alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);

    void removeDirective(std::shared_ptr<DirectiveInfo> info);
    void setHandlingCompleted(std::shared_ptr<DirectiveInfo> info);
    void handleDialDirective(std::shared_ptr<DirectiveInfo> info);
    void handleRedialDirective(std::shared_ptr<DirectiveInfo> info);
    void handleAnswerDirective(std::shared_ptr<DirectiveInfo> info);
    void handleStopDirective(std::shared_ptr<DirectiveInfo> info);
    void handlePlayRingtoneDirective(std::shared_ptr<DirectiveInfo> info);
    void handleSendDTMFDirective(std::shared_ptr<DirectiveInfo> info);
    void handleUnknownDirective(std::shared_ptr<DirectiveInfo> info);

    void executeCallStateChanged(CallState state, const std::string& callId, const std::string& callerId);
    void executeCallFailed(
        const std::string& callId,
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallError code,
        const std::string& message);
    void executeCallerIdReceived(const std::string& callId, const std::string& callerId);
    void executeSendDTMFSucceeded(const std::string& callId);
    void executeSendDTMFFailed(
        const std::string& callId,
        aace::phoneCallController::PhoneCallControllerEngineInterface::DTMFError code,
        const std::string& message);
    void executeOnFocusChanged(
        alexaClientSDK::avsCommon::avs::FocusState newFocus,
        alexaClientSDK::avsCommon::avs::MixingBehavior behavior);

    void acquireCommunicationsChannelFocus();
    void releaseCommunicationsChannelFocus();

    void updateContextManager();
    std::string getContextString();
    std::string getEventName(CallState state, const std::string& callId);

    void addCall(std::string callId, CallState state);
    void setCallState(std::string callId, CallState state);
    void removeCall(std::string callId);

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> m_focusManager;

    std::shared_ptr<PhoneCallControllerInterface> m_phoneCallController;
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>>
        m_capabilityConfigurations;

    std::string m_currentCallId;
    std::unordered_map<std::string, CallState> m_allCallsMap;
    std::unordered_map<std::string, CallMethod> m_callMethodMap;
    std::unordered_map<
        aace::phoneCallController::PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty,
        bool>
        m_deviceConfigurationMap;
    aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState m_connectionState;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

}  // namespace phoneCallController
}  // namespace engine
}  // namespace aace

#endif
