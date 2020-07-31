/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_TEST_ALEXA_MOCK_COMPONENT_FACTORY_H
#define AACE_ENGINE_TEST_ALEXA_MOCK_COMPONENT_FACTORY_H

#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/Audio/MockAlertsAudioFactory.h>
#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockCapabilitiesDelegate.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockSpeakerManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockPlaybackRouter.h>
#include <AVSCommon/SDKInterfaces/test/MockUserInactivityMonitor.h>
#include <AVSCommon/SDKInterfaces/test/MockLocaleAssetsManager.h>

#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <CertifiedSender/CertifiedSender.h>
#include <Endpoints/EndpointBuilder.h>
#include <ACL/AVSConnectionManager.h>

#include <AACE/Test/AVS/MockNotificationsAudioFactoryInterface.h>
#include <AACE/Test/AVS/MockAttachmentManager.h>
#include <AACE/Test/AVS/MockAudioPlayerInterface.h>
#include <AACE/Test/AVS/MockAudioPlayerObserverInterface.h>

#include <AACE/Test/AVS/MockAuthDelegateInterface.h>
#include <AACE/Test/AVS/MockDeviceSettingsManager.h>
#include <AACE/Test/AVS/MockEndpointRegistrationManagerInterface.h>
#include <AACE/Test/AVS/MockAlexaInterfaceMessageSenderInternalInterface.h>
#include <AACE/Test/AVS/MockSpeechConfirmationSetting.h>
#include <AACE/Test/AVS/MockSpeechEncoder.h>
#include <AACE/Test/AVS/MockSystemSoundPlayerInterface.h>
#include <AACE/Test/AVS/MockInternetConnectionMonitorInterface.h>
#include <AACE/Test/AVS/MockWakeWordConfirmationSetting.h>
#include <AACE/Test/AVS/MockWakeWordsSetting.h>
#include <AACE/Test/AVS/MockRenderPlayerInfoCardsProviderInterface.h>

#include <AACE/Test/Alexa/MockWakewordEngineAdapter.h>
#include <AACE/Test/Alexa/MockWakewordVerifier.h>
#include <AACE/Test/Alexa/MockDeviceSettingsDelegate.h>
#include <AACE/Engine/Alexa/DeviceSettingsDelegate.h>

#include <AACE/Test/AVS/MockConnectionStatusObserver.h>
#include <AACE/Test/AVS/MockCustomerDataManager.h>
#include <AACE/Test/AVS/MockMessageObserver.h>
#include <AACE/Test/AVS/MockMessageRouter.h>
#include <AACE/Test/AVS/MockMessageStorage.h>
#include <AACE/Test/AVS/MockAttachmentManager.h>

#include <AACE/Test/Audio/MockAudioManagerInterface.h>
#include <AACE/Test/Audio/MockAudioOutputChannelInterface.h>
#include <AACE/Test/Audio/MockAudioInputChannelInterface.h>

#include "MockAlerts.h"
#include "MockNotifications.h"
#include "MockAudioPlayer.h"
#include "MockAuthProvider.h"
#include "MockSpeechSynthesizer.h"
#include "MockSpeechRecognizer.h"
#include "MockAlexaClient.h"
#include "MockPlaybackController.h"
#include "MockTemplateRuntime.h"

#include <gtest/gtest.h>
#include <memory>

namespace aace {
namespace test {
namespace alexa {

class AlexaMockComponentFactory
        : public alexaClientSDK::avsCommon::utils::RequiresShutdown /* aace::engine::alexa::AlexaComponentInterface */ {
public:
    AlexaMockComponentFactory();

    std::shared_ptr<aace::test::avs::MockAlexaInterfaceMessageSenderInternalInterface>
    getAlexaInterfaceMessageSenderInternalInterfaceMock();
    std::shared_ptr<aace::test::avs::MockAuthDelegateInterface> getAuthDelegateInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockCapabilitiesDelegate>
    getCapabilitiesDelegateInterfaceMock();
    std::shared_ptr<aace::test::avs::MockWakeWordConfirmationSetting> getWakeWordConfirmationSettingMock();
    std::shared_ptr<aace::test::avs::MockSpeechConfirmationSetting> getSpeechConfirmationSettingMock();
    std::shared_ptr<aace::test::avs::MockSystemSoundPlayerInterface> getSystemSoundPlayerInterfaceMock();
    std::shared_ptr<aace::test::avs::MockInternetConnectionMonitorInterface>
    getInternetConnectionMonitorInterfaceMock();
    std::shared_ptr<aace::test::avs::MockWakeWordsSetting> getWakeWordsSettingMock();
    std::shared_ptr<aace::test::avs::MockSpeechEncoder> getSpeechEncoderMock();
    std::shared_ptr<aace::test::avs::MockRenderPlayerInfoCardsProviderInterface>
    getRenderPlayerInfoCardsProviderInterfaceMock();
    std::shared_ptr<aace::test::avs::MockDeviceSettingsManager> getDeviceSettingsManagerMock();
    std::shared_ptr<aace::test::avs::MockEndpointRegistrationManagerInterface>
    getEndpointRegistrationManagerInterfaceMock();
    std::shared_ptr<aace::test::alexa::MockWakewordEngineAdapter> getWakewordEngineAdapterMock();
    std::shared_ptr<aace::test::alexa::MockWakewordVerifier> getWakewordVerifierMock();
    std::shared_ptr<aace::test::alexa::MockDeviceSettingsDelegate> getDeviceSettingsDelegateMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockLocaleAssetsManager>
    getLocaleAssetsManagerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>
    getContextManagerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager> getFocusManagerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> getMessageSenderInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>
    getExceptionEncounteredSenderInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>
    getDirectiveSequencerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>
    getSpeakerManagerInterfaceMock();
    std::shared_ptr<aace::test::avs::MockCustomerDataManager> getCustomerDataManagerMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::test::MockAlertsAudioFactory>
    getAlertsAudioFactoryInterfaceMock();
    std::shared_ptr<aace::test::avs::MockNotificationsAudioFactoryInterface>
    getNotificationsAudioFactoryInterfaceMock();
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> getCertifiedSenderMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager>
    getAVSConnectionManagerInterfaceMock();
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> getAVSConnectionManagerMock();
    std::shared_ptr<aace::test::avs::MockMessageStorage> getMessageStorageMock();
    std::shared_ptr<aace::test::avs::MockMessageRouter> getMessageRouterMock();
    std::shared_ptr<aace::test::avs::MockAttachmentManager> getAttachmentManagerMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter> getPlaybackRouterMock();
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregator();
    std::shared_ptr<aace::test::audio::MockAudioManagerInterface> getAudioManagerMock();
    std::shared_ptr<aace::test::audio::MockAudioOutputChannelInterface> getAudioOutputChannelMock();
    std::shared_ptr<aace::test::audio::MockAudioInputChannelInterface> getAudioInputChannelMock();
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregatorMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor>
    getUserInactivityMonitorMock();
    std::shared_ptr<aace::test::avs::MockAudioPlayerInterface> getAudioPlayerInterfaceMock();
    std::shared_ptr<aace::test::avs::MockAudioPlayerObserverInterface> getAudioPlayerObserverInterfaceMock();
    std::unique_ptr<alexaClientSDK::endpoints::EndpointBuilder> getEndpointBuilderMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface> getAuthDelegate();

    // platform interface mocks
    std::shared_ptr<MockAlerts> getAlertsMock();
    std::shared_ptr<MockNotifications> getNotificationsMock();
    std::shared_ptr<MockAudioPlayer> getAudioPlayerMock();
    std::shared_ptr<MockAuthProvider> getAuthProviderMock();
    std::shared_ptr<MockSpeechSynthesizer> getSpeechSynthesizerMock();
    std::shared_ptr<MockSpeechRecognizer> getSpeechRecognizerMock();
    std::shared_ptr<MockAlexaClient> getAlexaClientMock();
    std::shared_ptr<MockPlaybackController> getPlaybackControllerMock();
    std::shared_ptr<MockTemplateRuntime> getTemplateRuntimeMock();

protected:
    void doShutdown() override;

private:
    std::vector<std::shared_ptr<alexaClientSDK::avsCommon::utils::RequiresShutdown>> m_shutdownList;
    std::shared_ptr<aace::test::avs::MockAlexaInterfaceMessageSenderInternalInterface>
        m_mockAlexaInterfaceMessageSenderInternalInterface;
    std::shared_ptr<aace::test::avs::MockAuthDelegateInterface> m_mockAuthDelegateInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockCapabilitiesDelegate>
        m_mockCapabilitiesDelegateInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockLocaleAssetsManager>
        m_mockLocaleAssetsManagerInterface;
    std::shared_ptr<aace::test::avs::MockWakeWordConfirmationSetting> m_mockWakeWordConfirmationSetting;
    std::shared_ptr<aace::test::avs::MockSpeechConfirmationSetting> m_mockSpeechConfirmationSetting;
    std::shared_ptr<aace::test::avs::MockSystemSoundPlayerInterface> m_mockSystemSoundPlayerInterface;
    std::shared_ptr<aace::test::avs::MockInternetConnectionMonitorInterface> m_mockInternetConnectionMonitorInterface;
    std::shared_ptr<aace::test::avs::MockWakeWordsSetting> m_mockWakeWordsSetting;
    std::shared_ptr<aace::test::avs::MockSpeechEncoder> m_mockSpeechEncoder;
    std::shared_ptr<aace::test::avs::MockRenderPlayerInfoCardsProviderInterface>
        m_mockRenderPlayerInfoCardsProviderInterface;
    std::shared_ptr<aace::test::avs::MockDeviceSettingsManager> m_mockDeviceSettingsManager;
    std::shared_ptr<aace::test::avs::MockEndpointRegistrationManagerInterface>
        m_mockEndpointRegistrationManagerInterface;
    std::shared_ptr<aace::test::alexa::MockWakewordEngineAdapter> m_mockWakewordEngineAdapter;
    std::shared_ptr<aace::test::alexa::MockWakewordVerifier> m_mockWakewordVerifier;
    std::shared_ptr<aace::test::alexa::MockDeviceSettingsDelegate> m_mockDeviceSettingsDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager> m_mockContextManagerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager> m_mockFocusManagerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> m_mockMessageSenderInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>
        m_mockExceptionEncounteredSenderInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>
        m_mockDirectiveSequencerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager> m_mockSpeakerManagerInterface;
    std::shared_ptr<aace::test::avs::MockCustomerDataManager> m_mockCustomerDataManager;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::test::MockAlertsAudioFactory>
        m_mockAlertsAudioFactory;
    std::shared_ptr<aace::test::avs::MockNotificationsAudioFactoryInterface> m_mockNotificationsAudioFactoryInterface;
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> m_mockCertifiedSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager>
        m_mockAVSConnectionManagerInterface;
    std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> m_mockAVSConnectionManager;
    std::shared_ptr<aace::test::avs::MockMessageStorage> m_mockMessageStorage;
    std::shared_ptr<aace::test::avs::MockMessageRouter> m_mockMessageRouter;
    std::shared_ptr<aace::test::avs::MockAttachmentManager> m_mockAttachmentManager;
    std::shared_ptr<aace::test::audio::MockAudioManagerInterface> m_mockAudioManager;
    std::shared_ptr<aace::test::audio::MockAudioOutputChannelInterface> m_mockAudioOutputChannel;
    std::shared_ptr<aace::test::audio::MockAudioInputChannelInterface> m_mockAudioInputChannel;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter> m_mockPlaybackRouter;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_mockDialogUXStateAggregator;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor>
        m_mockUserInactivityMonitor;
    std::shared_ptr<aace::test::avs::MockAudioPlayerInterface> m_mockAudioPlayerInterface;
    std::shared_ptr<aace::test::avs::MockAudioPlayerObserverInterface> m_mockAudioPlayerObserverInterface;
    std::unique_ptr<alexaClientSDK::endpoints::EndpointBuilder> m_mockEndpointBuilder;

    // platform interface mocks
    std::shared_ptr<MockAlerts> m_mockAlerts;
    std::shared_ptr<MockNotifications> m_mockNotifications;
    std::shared_ptr<MockAudioPlayer> m_mockAudioPlayer;
    std::shared_ptr<MockAuthProvider> m_mockAuthProvider;
    std::shared_ptr<MockSpeechSynthesizer> m_mockSpeechSynthesier;
    std::shared_ptr<MockSpeechRecognizer> m_mockSpeechRecognizer;
    std::shared_ptr<MockAlexaClient> m_mockAlexaClient;
    std::shared_ptr<MockPlaybackController> m_mockPlaybackController;
    std::shared_ptr<MockTemplateRuntime> m_mockTemplateRuntime;
};

}  // namespace alexa
}  // namespace test
}  // namespace aace

#endif  // AACE_ENGINE_TEST_ALEXA_MOCK_COMPONENT_FACTORY_H
