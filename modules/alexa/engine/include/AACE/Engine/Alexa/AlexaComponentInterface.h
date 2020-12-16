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

#ifndef AACE_ENGINE_ALEXA_INTERFACE_ALEXA_COMPONENT_INTERFACE_H
#define AACE_ENGINE_ALEXA_INTERFACE_ALEXA_COMPONENT_INTERFACE_H

#include <ACL/AVSConnectionManager.h>
#include <ACL/Transport/TransportFactoryInterface.h>
#include <ADSL/MessageInterpreter.h>
#include <AFML/FocusManager.h>
#include <AVSCommon/AVS/Attachment/AttachmentManager.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <AVSCommon/AVS/ExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/AuthDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/AVSGatewayAssignerInterface.h>
#include <AVSCommon/SDKInterfaces/AVSGatewayManagerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/DialogUXStateObserverInterface.h>
#include <AVSCommon/SDKInterfaces/DirectiveSequencerInterface.h>
#include <AVSCommon/SDKInterfaces/SoftwareInfoSenderObserverInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <AVSCommon/Utils/DeviceInfo.h>
#include <ContextManager/ContextManager.h>
#include <Endpoints/EndpointBuilder.h>
#include <RegistrationManager/RegistrationManager.h>
#include <Settings/Storage/DeviceSettingStorageInterface.h>
#include <SpeechSynthesizer/SpeechSynthesizer.h>

#include "AuthorizationManager.h"
#include "EndpointBuilderFactory.h"
#include "ExternalMediaPlayer.h"

namespace aace {
namespace engine {
namespace alexa {

class AlexaComponentInterface {
public:
    virtual ~AlexaComponentInterface();

public:
    virtual std::shared_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentManager> getAttachmentManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> getAudioFocusManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> getAuthDelegate() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayAssignerInterface>
    getAVSGatewayAssigner() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSGatewayManagerInterface>
    getAVSGatewayManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface>
    getCapabilitiesDelegate() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface>
    getConnectionManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> getContextManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> getCustomerDataManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> getDefaultEndpointBuilder() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> getDeviceInfo() = 0;
    virtual std::shared_ptr<alexaClientSDK::settings::storage::DeviceSettingStorageInterface>
    getDeviceSettingStorage() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregator() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::DirectiveSequencerInterface>
    getDirectiveSequencer() = 0;
    virtual std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> getEndpointBuilderFactory() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>
    getExceptionEncounteredSender() = 0;
    virtual std::shared_ptr<aace::engine::alexa::ExternalMediaPlayer> getExternalMediaPlayer() = 0;
    virtual std::shared_ptr<alexaClientSDK::adsl::MessageInterpreter> getMessageInterpreter() = 0;
    virtual std::shared_ptr<alexaClientSDK::acl::MessageRouterInterface> getMessageRouter() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> getMessageSender() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> getSpeakerManager() = 0;
    virtual std::shared_ptr<alexaClientSDK::capabilityAgents::speechSynthesizer::SpeechSynthesizer>
    getSpeechSynthesizer() = 0;
    virtual std::shared_ptr<alexaClientSDK::acl::TransportFactoryInterface> getTransportFactory() = 0;
    virtual std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface>
    getVisualFocusManager() = 0;
    virtual std::shared_ptr<AuthorizationManager> getAuthorizationManager() = 0;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_INTERFACE_ALEXA_COMPONENT_INTERFACE_H
