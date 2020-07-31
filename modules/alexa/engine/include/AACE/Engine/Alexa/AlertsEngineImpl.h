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

#ifndef AACE_ENGINE_ALEXA_ALERTS_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_ALERTS_ENGINE_IMPL_H

#include <memory>
#include <string>

#include <Alerts/AlertsCapabilityAgent.h>
#include <Alerts/Storage/SQLiteAlertStorage.h>
#include <Audio/AudioFactory.h>
#include <AVSCommon/AVS/Attachment/AttachmentManagerInterface.h>
#include <AVSCommon/SDKInterfaces/Audio/AlertsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/AVSConnectionManagerInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilityConfigurationInterface.h>
#include <AVSCommon/SDKInterfaces/ConnectionStatusObserverInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/SDKInterfaces/SpeakerManagerInterface.h>
#include <ContextManager/ContextManager.h>
#include <Endpoints/EndpointBuilder.h>
#include <RegistrationManager/CustomerDataHandler.h>

#include <AACE/Alexa/Alerts.h>
#include <AACE/Alexa/AlexaEngineInterfaces.h>
#include <AACE/Engine/Alexa/DeviceSettingsDelegate.h>
#include <AACE/Engine/Audio/AudioManagerInterface.h>

#include "AudioChannelEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

class AlertsEngineImpl
        : public AudioChannelEngineImpl
        , public aace::alexa::AlertsEngineInterface
        , public alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface {
private:
    AlertsEngineImpl(
        std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager);

    bool initialize(
        std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
        std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface>
            alertsAudioFactory,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
        class DeviceSettingsDelegate& deviceSettingsDelegate);

public:
    static std::shared_ptr<AlertsEngineImpl> create(
        std::shared_ptr<aace::alexa::Alerts> alertsPlatformInterface,
        std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> connectionManager,
        std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> certifiedSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::AlertsAudioFactoryInterface>
            alertsAudioFactory,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
        class DeviceSettingsDelegate& deviceSettingsDelegate);

    // AlertsEngineInterface
    void onLocalStop() override;
    void removeAllAlerts() override;

    // AlertObserverInterface
    void onAlertStateChange(
        const std::string& alertToken,
        const std::string& alertType,
        alexaClientSDK::capabilityAgents::alerts::AlertObserverInterface::State state,
        const std::string& reason) override;

    void onAlertCreated(const std::string& alertToken, const std::string& detailedInfo) override;

    void onAlertDeleted(const std::string& alertToken) override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::Alerts> m_alertsPlatformInterface;
    std::shared_ptr<alexaClientSDK::capabilityAgents::alerts::AlertsCapabilityAgent> m_alertsCapabilityAgent;
    std::weak_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AVSConnectionManagerInterface> m_connectionManager;
    std::shared_ptr<alexaClientSDK::capabilityAgents::alerts::renderer::Renderer> m_alertRenderer;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_ALERTS_ENGINE_IMPL_H
