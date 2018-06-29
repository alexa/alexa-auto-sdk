/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/StateProviderInterface.h>
#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/Utils/Threading/Executor.h>

#include <AACE/PhoneCallController/PhoneCallControllerEngineInterfaces.h>
#include "PhoneCallControllerObserverInterface.h"

namespace aace {
namespace engine {
namespace phoneCallController {

class PhoneCallControllerCapabilityAgent :
    public alexaClientSDK::avsCommon::avs::CapabilityAgent,
    public alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<PhoneCallControllerCapabilityAgent> {

public:
    enum class CallState {
        IDLE,
        ACTIVE
    };

    static std::shared_ptr<PhoneCallControllerCapabilityAgent> create (
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender
    );

    virtual ~PhoneCallControllerCapabilityAgent() = default;

    void handleDirectiveImmediately( std::shared_ptr<alexaClientSDK::avsCommon::avs::AVSDirective> directive ) override;
    void preHandleDirective( std::shared_ptr<DirectiveInfo> info ) override;
    void handleDirective( std::shared_ptr<DirectiveInfo> info ) override;
    void cancelDirective( std::shared_ptr<DirectiveInfo> info ) override;
    alexaClientSDK::avsCommon::avs::DirectiveHandlerConfiguration getConfiguration() const override;

    void addObserver ( std::shared_ptr<PhoneCallControllerObserverInterface> observer );
    void removeObserver ( std::shared_ptr<PhoneCallControllerObserverInterface> observer );

    void connectionStateChanged( aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state );
    void callActivated( const std::string& callId );
    void callFailed( const std::string& callId, const std::string& error, const std::string& message );
    void callTerminated( const std::string& callId );

    CallState getCallState( std::string callId );
    bool callExist( std::string callId );

    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> getCapabilityConfigurations() override;

private:
    PhoneCallControllerCapabilityAgent (
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender
    );

    void doShutdown() override;

    void sendExceptionEncounteredAndReportFailed(
        std::shared_ptr<DirectiveInfo> info,
        const std::string& message,
        alexaClientSDK::avsCommon::avs::ExceptionErrorType type = alexaClientSDK::avsCommon::avs::ExceptionErrorType::INTERNAL_ERROR);
        
    void removeDirective( std::shared_ptr<DirectiveInfo> info );
    void setHandlingCompleted( std::shared_ptr<DirectiveInfo> info );
    void handleDialDirective( std::shared_ptr<DirectiveInfo> info );
    void handleUnknownDirective ( std::shared_ptr<DirectiveInfo> info );
        
    void executeCallActivated( const std::string& callId );
    void executeCallFailed( const std::string& callId, const std::string& error, const std::string& message );
    void executeCallTerminated( const std::string& callId );

    void updateContextManager();
    std::string getContextString();
        
    std::string getConnectionState( aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState state );
    std::string getCallState ( CallState state );

    void addCall( std::string callId, CallState state );
    void setCallState( std::string callId, CallState state );
    void removeCall( std::string callId );

    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> m_contextManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> m_messageSender;
        
    std::unordered_set<std::shared_ptr<PhoneCallControllerObserverInterface>> m_observers;
    std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::avs::CapabilityConfiguration>> m_capabilityConfigurations;

    std::string m_currentActiveCallId;
    std::unordered_map<std::string, CallState> m_allCallsMap;
    aace::phoneCallController::PhoneCallControllerEngineInterface::ConnectionState m_connectionState;
    alexaClientSDK::avsCommon::utils::threading::Executor m_executor;
};

} // aace::engine::phoneCallController
} // aace::engine
} // aace

#endif
