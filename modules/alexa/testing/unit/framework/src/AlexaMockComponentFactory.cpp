/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Test/Unit/Alexa/AlexaMockComponentFactory.h>
#include <acsdkShutdownManager/ShutdownManager.h>

namespace aace {
namespace test {
namespace unit {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.test.alexa.AlexaMockComponentFactory");

AlexaMockComponentFactory::AlexaMockComponentFactory() : alexaClientSDK::avsCommon::utils::RequiresShutdown(TAG) {
}

void AlexaMockComponentFactory::doShutdown() {
    for (auto& rs : m_shutdownList) {
        rs->shutdown();
        rs.reset();
    }

    // ShutdownManager does not extend the RequiresShutdown class
    if (m_mockShutDownManager != nullptr) {
        m_mockShutDownManager->shutdown();
    }
}

std::shared_ptr<aace::test::unit::avs::MockAlexaInterfaceMessageSenderInternalInterface> AlexaMockComponentFactory::
    getAlexaInterfaceMessageSenderInternalInterfaceMock() {
    if (m_mockAlexaInterfaceMessageSenderInternalInterface == nullptr) {
        m_mockAlexaInterfaceMessageSenderInternalInterface =
            std::make_shared<aace::test::unit::avs::MockAlexaInterfaceMessageSenderInternalInterface>();
    }

    return m_mockAlexaInterfaceMessageSenderInternalInterface;
}

std::shared_ptr<aace::test::unit::avs::MockAuthDelegateInterface> AlexaMockComponentFactory::
    getAuthDelegateInterfaceMock() {
    if (m_mockAuthDelegateInterface == nullptr) {
        m_mockAuthDelegateInterface = std::make_shared<aace::test::unit::avs::MockAuthDelegateInterface>();
    }

    return m_mockAuthDelegateInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockCapabilitiesDelegate> AlexaMockComponentFactory::
    getCapabilitiesDelegateInterfaceMock() {
    if (m_mockCapabilitiesDelegateInterface == nullptr) {
        m_mockCapabilitiesDelegateInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockCapabilitiesDelegate>();
    }

    return m_mockCapabilitiesDelegateInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockLocaleAssetsManager> AlexaMockComponentFactory::
    getLocaleAssetsManagerInterfaceMock() {
    if (m_mockLocaleAssetsManagerInterface == nullptr) {
        m_mockLocaleAssetsManagerInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockLocaleAssetsManager>();
    }

    return m_mockLocaleAssetsManagerInterface;
}

std::shared_ptr<aace::test::unit::avs::MockWakeWordConfirmationSetting> AlexaMockComponentFactory::
    getWakeWordConfirmationSettingMock() {
    if (m_mockWakeWordConfirmationSetting == nullptr) {
        m_mockWakeWordConfirmationSetting = std::make_shared<aace::test::unit::avs::MockWakeWordConfirmationSetting>();
    }

    return m_mockWakeWordConfirmationSetting;
}

std::shared_ptr<aace::test::unit::avs::MockSpeechConfirmationSetting> AlexaMockComponentFactory::
    getSpeechConfirmationSettingMock() {
    if (m_mockSpeechConfirmationSetting == nullptr) {
        m_mockSpeechConfirmationSetting = std::make_shared<aace::test::unit::avs::MockSpeechConfirmationSetting>();
    }

    return m_mockSpeechConfirmationSetting;
}

std::shared_ptr<aace::test::unit::avs::MockSystemSoundPlayerInterface> AlexaMockComponentFactory::
    getSystemSoundPlayerInterfaceMock() {
    if (m_mockSystemSoundPlayerInterface == nullptr) {
        m_mockSystemSoundPlayerInterface = std::make_shared<aace::test::unit::avs::MockSystemSoundPlayerInterface>();
    }

    return m_mockSystemSoundPlayerInterface;
}

std::shared_ptr<aace::test::unit::avs::MockInternetConnectionMonitorInterface> AlexaMockComponentFactory::
    getInternetConnectionMonitorInterfaceMock() {
    if (m_mockInternetConnectionMonitorInterface == nullptr) {
        m_mockInternetConnectionMonitorInterface =
            std::make_shared<aace::test::unit::avs::MockInternetConnectionMonitorInterface>();
    }

    return m_mockInternetConnectionMonitorInterface;
}

std::shared_ptr<aace::test::unit::avs::MockWakeWordsSetting> AlexaMockComponentFactory::getWakeWordsSettingMock() {
    if (m_mockWakeWordsSetting == nullptr) {
        m_mockWakeWordsSetting = std::make_shared<aace::test::unit::avs::MockWakeWordsSetting>();
    }

    return m_mockWakeWordsSetting;
}

std::shared_ptr<aace::test::unit::avs::MockSpeechEncoder> AlexaMockComponentFactory::getSpeechEncoderMock() {
    if (m_mockSpeechEncoder == nullptr) {
        m_mockSpeechEncoder = std::make_shared<aace::test::unit::avs::MockSpeechEncoder>(nullptr);
    }

    return m_mockSpeechEncoder;
}
std::shared_ptr<aace::test::unit::avs::MockRenderPlayerInfoCardsProviderInterface>
getRenderPlayerInfoCardsProviderInterfaceMock();

std::shared_ptr<aace::test::unit::avs::MockRenderPlayerInfoCardsProviderInterface> AlexaMockComponentFactory::
    getRenderPlayerInfoCardsProviderInterfaceMock() {
    if (m_mockRenderPlayerInfoCardsProviderInterface == nullptr) {
        m_mockRenderPlayerInfoCardsProviderInterface =
            std::make_shared<aace::test::unit::avs::MockRenderPlayerInfoCardsProviderInterface>();
    }

    return m_mockRenderPlayerInfoCardsProviderInterface;
}

std::shared_ptr<aace::test::unit::alexa::MockWakewordEngineAdapter> AlexaMockComponentFactory::
    getWakewordEngineAdapterMock() {
    if (m_mockWakewordEngineAdapter == nullptr) {
        m_mockWakewordEngineAdapter = std::make_shared<aace::test::unit::alexa::MockWakewordEngineAdapter>();
    }

    return m_mockWakewordEngineAdapter;
}

std::shared_ptr<aace::test::unit::alexa::MockInitiatorVerifier> AlexaMockComponentFactory::getInitiatorVerifierMock() {
    if (m_mockInitiatorVerifier == nullptr) {
        m_mockInitiatorVerifier = std::make_shared<aace::test::unit::alexa::MockInitiatorVerifier>();
    }

    return m_mockInitiatorVerifier;
}

std::shared_ptr<aace::test::unit::alexa::MockDeviceSettingsDelegate> AlexaMockComponentFactory::
    getDeviceSettingsDelegateMock() {
    if (m_mockDeviceSettingsDelegate == nullptr) {
        m_mockDeviceSettingsDelegate = std::make_shared<aace::test::unit::alexa::MockDeviceSettingsDelegate>();
    }

    return m_mockDeviceSettingsDelegate;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager> AlexaMockComponentFactory::
    getContextManagerInterfaceMock() {
    if (m_mockContextManagerInterface == nullptr) {
        m_mockContextManagerInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>();
    }

    return m_mockContextManagerInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager> AlexaMockComponentFactory::
    getFocusManagerInterfaceMock() {
    if (m_mockFocusManagerInterface == nullptr) {
        m_mockFocusManagerInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>();
    }

    return m_mockFocusManagerInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> AlexaMockComponentFactory::
    getMessageSenderInterfaceMock() {
    if (m_mockMessageSenderInterface == nullptr) {
        m_mockMessageSenderInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>();
    }

    return m_mockMessageSenderInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>
AlexaMockComponentFactory::getExceptionEncounteredSenderInterfaceMock() {
    if (m_mockExceptionEncounteredSenderInterface == nullptr) {
        m_mockExceptionEncounteredSenderInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>();
    }

    return m_mockExceptionEncounteredSenderInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer> AlexaMockComponentFactory::
    getDirectiveSequencerInterfaceMock() {
    if (m_mockDirectiveSequencerInterface == nullptr) {
        m_mockDirectiveSequencerInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>();
        m_shutdownList.emplace_back(m_mockDirectiveSequencerInterface);
    }

    return m_mockDirectiveSequencerInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager> AlexaMockComponentFactory::
    getSpeakerManagerInterfaceMock() {
    if (m_mockSpeakerManagerInterface == nullptr) {
        m_mockSpeakerManagerInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>();
    }

    return m_mockSpeakerManagerInterface;
}

std::shared_ptr<aace::test::unit::avs::MockCustomerDataManager> AlexaMockComponentFactory::
    getCustomerDataManagerMock() {
    if (m_mockCustomerDataManager == nullptr) {
        m_mockCustomerDataManager = std::make_shared<aace::test::unit::avs::MockCustomerDataManager>();
    }

    return m_mockCustomerDataManager;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::audio::test::MockAlertsAudioFactory>
AlexaMockComponentFactory::getAlertsAudioFactoryInterfaceMock() {
    if (m_mockAlertsAudioFactory == nullptr) {
        m_mockAlertsAudioFactory =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::audio::test::MockAlertsAudioFactory>();
    }

    return m_mockAlertsAudioFactory;
}

std::shared_ptr<aace::test::unit::avs::MockNotificationsAudioFactoryInterface> AlexaMockComponentFactory::
    getNotificationsAudioFactoryInterfaceMock() {
    if (m_mockNotificationsAudioFactoryInterface == nullptr) {
        m_mockNotificationsAudioFactoryInterface =
            std::make_shared<aace::test::unit::avs::MockNotificationsAudioFactoryInterface>();
    }

    return m_mockNotificationsAudioFactoryInterface;
}

std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> AlexaMockComponentFactory::getCertifiedSenderMock() {
    if (m_mockCertifiedSender == nullptr) {
        auto mockMessageStorage = std::make_shared<aace::test::unit::avs::MockMessageStorage>();
        EXPECT_CALL(*mockMessageStorage, open()).WillOnce(::testing::Return(false));
        EXPECT_CALL(*mockMessageStorage, createDatabase()).WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockMessageStorage, load(::testing::_)).WillOnce(::testing::Return(true));

        m_mockCertifiedSender = alexaClientSDK::certifiedSender::CertifiedSender::create(
            getMessageSenderInterfaceMock(),
            getAVSConnectionManagerInterfaceMock(),
            mockMessageStorage,
            getCustomerDataManagerMock());
        m_shutdownList.emplace_back(m_mockCertifiedSender);
    }

    return m_mockCertifiedSender;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager> AlexaMockComponentFactory::
    getAVSConnectionManagerInterfaceMock() {
    if (m_mockAVSConnectionManagerInterface == nullptr) {
        m_mockAVSConnectionManagerInterface =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager>();
    }

    return m_mockAVSConnectionManagerInterface;
}

std::shared_ptr<aace::test::unit::avs::MockMetricRecorder> AlexaMockComponentFactory::getMetricRecorder() {
    if (m_mockMetricRecorder == nullptr) {
        m_mockMetricRecorder = std::make_shared<aace::test::unit::avs::MockMetricRecorder>();
    }

    return m_mockMetricRecorder;
}

std::shared_ptr<alexaClientSDK::acl::AVSConnectionManager> AlexaMockComponentFactory::getAVSConnectionManagerMock() {
    if (m_mockAVSConnectionManager == nullptr) {
        auto messageRouter = getMessageRouterMock();
        auto internetConnectionMonitorInterface = getInternetConnectionMonitorInterfaceMock();
        m_mockAVSConnectionManager = alexaClientSDK::acl::AVSConnectionManager::create(
            messageRouter,
            true,
            std::unordered_set<
                std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ConnectionStatusObserverInterface>>(),
            std::unordered_set<std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageObserverInterface>>(),
            internetConnectionMonitorInterface);
        m_shutdownList.emplace_back(m_mockAVSConnectionManager);
    }

    return m_mockAVSConnectionManager;
}

std::shared_ptr<aace::test::unit::avs::MockDeviceSettingsManager> AlexaMockComponentFactory::
    getDeviceSettingsManagerMock() {
    if (m_mockDeviceSettingsManager == nullptr) {
        m_mockDeviceSettingsManager = std::make_shared<aace::test::unit::avs::MockDeviceSettingsManager>();
    }

    return m_mockDeviceSettingsManager;
}

std::shared_ptr<aace::test::unit::avs::MockEndpointRegistrationManagerInterface> AlexaMockComponentFactory::
    getEndpointRegistrationManagerInterfaceMock() {
    if (m_mockEndpointRegistrationManagerInterface == nullptr) {
        m_mockEndpointRegistrationManagerInterface =
            std::make_shared<aace::test::unit::avs::MockEndpointRegistrationManagerInterface>();
    }

    return m_mockEndpointRegistrationManagerInterface;
}

std::unique_ptr<alexaClientSDK::endpoints::EndpointBuilder> AlexaMockComponentFactory::getEndpointBuilderMock() {
    if (m_mockEndpointBuilder == nullptr) {
        auto deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create(
            "clientId", "productId", "1234", "manufacturer", "my device");
        auto contextManager = getContextManagerInterfaceMock();
        auto exceptionSender = getExceptionEncounteredSenderInterfaceMock();
        auto alexaInternalInterface = getAlexaInterfaceMessageSenderInternalInterfaceMock();
        m_mockEndpointBuilder = alexaClientSDK::endpoints::EndpointBuilder::create(
            std::move(deviceInfo), contextManager, exceptionSender, alexaInternalInterface);
    }

    return std::move(m_mockEndpointBuilder);
}

std::shared_ptr<aace::test::unit::avs::MockMessageStorage> AlexaMockComponentFactory::getMessageStorageMock() {
    if (m_mockMessageStorage == nullptr) {
        m_mockMessageStorage = std::make_shared<aace::test::unit::avs::MockMessageStorage>();
    }

    return m_mockMessageStorage;
}

std::shared_ptr<aace::test::unit::avs::MockMessageRouter> AlexaMockComponentFactory::getMessageRouterMock() {
    if (m_mockMessageRouter == nullptr) {
        m_mockMessageRouter = std::make_shared<aace::test::unit::avs::MockMessageRouter>();
        m_shutdownList.emplace_back(m_mockMessageRouter);
    }

    return m_mockMessageRouter;
}

std::shared_ptr<aace::test::unit::avs::MockAttachmentManager> AlexaMockComponentFactory::getAttachmentManagerMock() {
    if (m_mockAttachmentManager == nullptr) {
        m_mockAttachmentManager = std::make_shared<aace::test::unit::avs::MockAttachmentManager>();
    }

    return m_mockAttachmentManager;
}

std::shared_ptr<aace::test::unit::audio::MockAudioManagerInterface> AlexaMockComponentFactory::getAudioManagerMock() {
    if (m_mockAudioManager == nullptr) {
        m_mockAudioManager = std::make_shared<aace::test::unit::audio::MockAudioManagerInterface>();
    }

    return m_mockAudioManager;
}

std::shared_ptr<aace::test::unit::audio::MockAudioOutputChannelInterface> AlexaMockComponentFactory::
    getAudioOutputChannelMock() {
    if (m_mockAudioOutputChannel == nullptr) {
        m_mockAudioOutputChannel = std::make_shared<aace::test::unit::audio::MockAudioOutputChannelInterface>();
    }

    return m_mockAudioOutputChannel;
}

std::shared_ptr<aace::test::unit::audio::MockAudioInputChannelInterface> AlexaMockComponentFactory::
    getAudioInputChannelMock() {
    if (m_mockAudioInputChannel == nullptr) {
        m_mockAudioInputChannel = std::make_shared<aace::test::unit::audio::MockAudioInputChannelInterface>();
    }

    return m_mockAudioInputChannel;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter> AlexaMockComponentFactory::
    getPlaybackRouterMock() {
    if (m_mockPlaybackRouter == nullptr) {
        m_mockPlaybackRouter = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter>();
    }

    return m_mockPlaybackRouter;
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> AlexaMockComponentFactory::
    getDialogUXStateAggregatorMock() {
    if (m_mockDialogUXStateAggregator == nullptr) {
        m_mockDialogUXStateAggregator = std::make_shared<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator>();
    }

    return m_mockDialogUXStateAggregator;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor> AlexaMockComponentFactory::
    getUserInactivityMonitorMock() {
    if (m_mockUserInactivityMonitor == nullptr) {
        m_mockUserInactivityMonitor =
            std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor>();
    }

    return m_mockUserInactivityMonitor;
}

std::shared_ptr<aace::test::unit::avs::MockAudioPlayerInterface> AlexaMockComponentFactory::
    getAudioPlayerInterfaceMock() {
    if (m_mockAudioPlayerInterface == nullptr) {
        m_mockAudioPlayerInterface = std::make_shared<aace::test::unit::avs::MockAudioPlayerInterface>();
    }

    return m_mockAudioPlayerInterface;
}

std::shared_ptr<aace::test::unit::avs::MockAudioPlayerObserverInterface> AlexaMockComponentFactory::
    getAudioPlayerObserverInterfaceMock() {
    if (m_mockAudioPlayerObserverInterface == nullptr) {
        m_mockAudioPlayerObserverInterface =
            std::make_shared<aace::test::unit::avs::MockAudioPlayerObserverInterface>();
    }

    return m_mockAudioPlayerObserverInterface;
}

std::shared_ptr<alexaClientSDK::acsdkShutdownManager::ShutdownNotifier> AlexaMockComponentFactory::
    getShutDownNotifierMock() {
    if (m_mockShutDownNotifier == nullptr) {
        m_mockShutDownNotifier = std::make_shared<alexaClientSDK::acsdkShutdownManager::ShutdownNotifier>();
    }
    if (m_mockShutDownManager == nullptr) {
        m_mockShutDownManager = alexaClientSDK::acsdkShutdownManager::ShutdownManager::createShutdownManagerInterface(
            m_mockShutDownNotifier);
    }
    return m_mockShutDownNotifier;
}

//
// platform interface mocks
//

std::shared_ptr<MockAlerts> AlexaMockComponentFactory::getAlertsMock() {
    if (m_mockAlerts == nullptr) {
        m_mockAlerts = std::make_shared<MockAlerts>();
    }

    return m_mockAlerts;
}

std::shared_ptr<MockNotifications> AlexaMockComponentFactory::getNotificationsMock() {
    if (m_mockNotifications == nullptr) {
        m_mockNotifications = std::make_shared<MockNotifications>();
    }

    return m_mockNotifications;
}

std::shared_ptr<MockAudioPlayer> AlexaMockComponentFactory::getAudioPlayerMock() {
    if (m_mockAudioPlayer == nullptr) {
        m_mockAudioPlayer = std::make_shared<MockAudioPlayer>();
    }

    return m_mockAudioPlayer;
}

std::shared_ptr<MockAuthProvider> AlexaMockComponentFactory::getAuthProviderMock() {
    if (m_mockAuthProvider == nullptr) {
        m_mockAuthProvider = std::make_shared<MockAuthProvider>();
    }

    return m_mockAuthProvider;
}

std::shared_ptr<MockSpeechSynthesizer> AlexaMockComponentFactory::getSpeechSynthesizerMock() {
    if (m_mockSpeechSynthesier == nullptr) {
        m_mockSpeechSynthesier = std::make_shared<MockSpeechSynthesizer>();
    }

    return m_mockSpeechSynthesier;
}

std::shared_ptr<MockSpeechRecognizer> AlexaMockComponentFactory::getSpeechRecognizerMock() {
    if (m_mockSpeechRecognizer == nullptr) {
        m_mockSpeechRecognizer = std::make_shared<MockSpeechRecognizer>();
    }

    return m_mockSpeechRecognizer;
}

std::shared_ptr<MockAlexaClient> AlexaMockComponentFactory::getAlexaClientMock() {
    if (m_mockAlexaClient == nullptr) {
        m_mockAlexaClient = std::make_shared<MockAlexaClient>();
    }

    return m_mockAlexaClient;
}

std::shared_ptr<MockPlaybackController> AlexaMockComponentFactory::getPlaybackControllerMock() {
    if (m_mockPlaybackController == nullptr) {
        m_mockPlaybackController = std::make_shared<MockPlaybackController>();
    }

    return m_mockPlaybackController;
}

std::shared_ptr<MockTemplateRuntime> AlexaMockComponentFactory::getTemplateRuntimeMock() {
    if (m_mockTemplateRuntime == nullptr) {
        m_mockTemplateRuntime = std::make_shared<MockTemplateRuntime>();
    }

    return m_mockTemplateRuntime;
}

}  // namespace alexa
}  // namespace unit
}  // namespace test
}  // namespace aace
