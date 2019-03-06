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

#ifndef AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_ENGINE_IMPL_H
#define AACE_ENGINE_PHONECALLCONTROLLER_PHONECALLCONTROLLER_ENGINE_IMPL_H

#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>

#include <AACE/PhoneCallController/PhoneCallController.h>
#include <AACE/PhoneCallController/PhoneCallControllerEngineInterfaces.h>
#include "PhoneCallControllerCapabilityAgent.h"
#include "PhoneCallControllerInterface.h"

namespace aace {
namespace engine {
namespace phoneCallController {

class PhoneCallControllerEngineImpl :
    public PhoneCallControllerInterface,
    public aace::phoneCallController::PhoneCallControllerEngineInterface,
    public alexaClientSDK::avsCommon::utils::RequiresShutdown,
    public std::enable_shared_from_this<PhoneCallControllerEngineImpl> {

private:
    PhoneCallControllerEngineImpl ( std::shared_ptr<aace::phoneCallController::PhoneCallController> phoneCallControllerPlatformInterface );

    bool initialize (
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager );

public:
    static std::shared_ptr<PhoneCallControllerEngineImpl> create (
        std::shared_ptr<aace::phoneCallController::PhoneCallController> phoneCallControllerPlatformInterface, 
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface> directiveSequencer,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager );

    // PhoneCallControllerEngineInterface
    void onConnectionStateChanged( ConnectionState state  ) override;
    void onCallStateChanged ( CallState state, const std::string& callId, const std::string& callerId ) override;
    void onCallFailed( const std::string& callId, CallError code, const std::string& message ) override;
    void onCallerIdReceived( const std::string& callId, const std::string& callerId ) override;
    void onSendDTMFSucceeded( const std::string& callId) override;
    void onSendDTMFFailed( const std::string& callId, DTMFError code, const std::string& message ) override;
    void onDeviceConfigurationUpdated( std::unordered_map<PhoneCallControllerEngineInterface::CallingDeviceConfigurationProperty, bool> configurationMap ) override;
    std::string onCreateCallId() override;

    // PhoneCallControllerInterface
    bool dial( const std::string& payload ) override;
    bool redial( const std::string& payload ) override;
    void answer( const std::string& payload ) override;
    void stop( const std::string& payload ) override;
    void playRingtone( const std::string& payload ) override;
    void sendDTMF( const std::string& payload ) override;

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::phoneCallController::PhoneCallController> m_phoneCallControllerPlatformInterface;
    std::shared_ptr<PhoneCallControllerCapabilityAgent> m_phoneCallControllerCapabilityAgent;
};

} // aace::engine::phoneCallController
} // aace::engine
} // aace

#endif
