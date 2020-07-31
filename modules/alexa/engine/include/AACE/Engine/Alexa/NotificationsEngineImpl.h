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

#ifndef AACE_ENGINE_ALEXA_NOTIFICATIONS_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_NOTIFICATIONS_ENGINE_IMPL_H

#include <memory>
#include <string>

#include <Audio/AudioFactory.h>
#include <AVSCommon/AVS/Attachment/AttachmentManagerInterface.h>
#include <AVSCommon/AVS/IndicatorState.h>
#include <AVSCommon/SDKInterfaces/Audio/NotificationsAudioFactoryInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/FocusManagerInterface.h>
#include <ContextManager/ContextManager.h>
#include <Endpoints/EndpointBuilder.h>
#include <Notifications/NotificationsCapabilityAgent.h>
#include <Notifications/NotificationRenderer.h>
#include <Notifications/SQLiteNotificationsStorage.h>
#include <RegistrationManager/CustomerDataHandler.h>

#include <AACE/Alexa/AlexaEngineInterfaces.h>
#include <AACE/Alexa/Notifications.h>
#include <AACE/Engine/Audio/AudioManagerInterface.h>

#include "AudioChannelEngineImpl.h"

namespace aace {
namespace engine {
namespace alexa {

class NotificationsEngineImpl
        : public AudioChannelEngineImpl
        , public alexaClientSDK::avsCommon::sdkInterfaces::NotificationsObserverInterface {
private:
    NotificationsEngineImpl(std::shared_ptr<aace::alexa::Notifications> notificationsPlatformInterface);

    bool initialize(
        std::shared_ptr<aace::engine::audio::AudioOutputChannelInterface> audioOutputChannel,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface>
            notificationsAudioFactory,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

public:
    static std::shared_ptr<NotificationsEngineImpl> create(
        std::shared_ptr<aace::alexa::Notifications> notificationsPlatformInterface,
        std::shared_ptr<aace::engine::audio::AudioManagerInterface> audioManager,
        std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> defaultEndpointBuilder,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface> exceptionSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::NotificationsAudioFactoryInterface>
            notificationsAudioFactory,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::SpeakerManagerInterface> speakerManager,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManager> dataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::FocusManagerInterface> focusManager);

    // NotificationObserverInterface
    void onSetIndicator(alexaClientSDK::avsCommon::avs::IndicatorState state) override;
    void onNotificationReceived() override;

protected:
    virtual void doShutdown() override;

private:
    std::shared_ptr<aace::alexa::Notifications> m_notificationsPlatformInterface;
    std::shared_ptr<alexaClientSDK::capabilityAgents::notifications::NotificationsCapabilityAgent>
        m_notificationsCapabilityAgent;
    std::shared_ptr<alexaClientSDK::capabilityAgents::notifications::NotificationRenderer> m_notificationRenderer;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_NOTIFICATIONS_ENGINE_IMPL_H
