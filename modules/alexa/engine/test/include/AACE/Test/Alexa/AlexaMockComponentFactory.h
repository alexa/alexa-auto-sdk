/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AVSCommon/SDKInterfaces/test/MockAVSConnectionManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockDirectiveSequencer.h>
#include <AVSCommon/SDKInterfaces/test/MockExceptionEncounteredSender.h>
#include <AVSCommon/SDKInterfaces/test/MockFocusManager.h>
#include <AVSCommon/SDKInterfaces/test/MockMessageSender.h>
#include <AVSCommon/SDKInterfaces/test/MockSpeakerManager.h>
#include <AVSCommon/SDKInterfaces/test/MockContextManager.h>
#include <AVSCommon/SDKInterfaces/test/MockPlaybackRouter.h>
#include <AVSCommon/SDKInterfaces/test/MockUserInactivityMonitor.h>

#include <AVSCommon/Utils/RequiresShutdown.h>
#include <AVSCommon/AVS/DialogUXStateAggregator.h>
#include <CertifiedSender/CertifiedSender.h>

#include <AACE/Test/AVS/MockAlertsAudioFactoryInterface.h>
#include <AACE/Test/AVS/MockNotificationsAudioFactoryInterface.h>
#include <AACE/Test/AVS/MockAttachmentManager.h>
#include <AACE/Test/AVS/MockAudioPlayerInterface.h>
#include <AACE/Test/AVS/MockCapabilitiesDelegateInterface.h>
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

namespace aace {
namespace test {
namespace alexa {

class AlexaMockComponentFactory : public alexaClientSDK::avsCommon::utils::RequiresShutdown /* aace::engine::alexa::AlexaComponentInterface */ {
public:
    AlexaMockComponentFactory();

    std::shared_ptr<aace::test::avs::MockCapabilitiesDelegateInterface> getCapabilitiesDelegateInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager> getContextManagerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager> getFocusManagerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> getMessageSenderInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender> getExceptionEncounteredSenderInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer> getDirectiveSequencerInterfaceMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager> getSpeakerManagerInterfaceMock();
    std::shared_ptr<aace::test::avs::MockCustomerDataManager> getCustomerDataManagerMock();
    std::shared_ptr<aace::test::avs::MockAlertsAudioFactoryInterface> getAlertsAudioFactoryInterfaceMock();
    std::shared_ptr<aace::test::avs::MockNotificationsAudioFactoryInterface> getNotificationsAudioFactoryInterfaceMock();
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> getCertifiedSenderMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager> getAVSConnectionManagerMock();
    std::shared_ptr<aace::test::avs::MockMessageStorage> getMessageStorageMock();
    std::shared_ptr<aace::test::avs::MockAttachmentManager> getAttachmentManagerMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter> getPlaybackRouterMock();
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregator();
    std::shared_ptr<aace::test::audio::MockAudioManagerInterface> getAudioManagerMock();
    std::shared_ptr<aace::test::audio::MockAudioOutputChannelInterface> getAudioOutputChannelMock();
    std::shared_ptr<aace::test::audio::MockAudioInputChannelInterface> getAudioInputChannelMock();
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> getDialogUXStateAggregatorMock();
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor> getUserInactivityMonitorMock();
    std::shared_ptr<aace::test::avs::MockAudioPlayerInterface> getAudioPlayerInterfaceMock();

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
    std::shared_ptr<aace::test::avs::MockCapabilitiesDelegateInterface> m_mockCapabilitiesDelegateInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager> m_mockContextManagerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager> m_mockFocusManagerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> m_mockMessageSenderInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender> m_mockExceptionEncounteredSenderInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer> m_mockDirectiveSequencerInterface;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager> m_mockSpeakerManagerInterface;
    std::shared_ptr<aace::test::avs::MockCustomerDataManager> m_mockCustomerDataManager;
    std::shared_ptr<aace::test::avs::MockAlertsAudioFactoryInterface> m_mockAlertsAudioFactoryInterface;
    std::shared_ptr<aace::test::avs::MockNotificationsAudioFactoryInterface> m_mockNotificationsAudioFactoryInterface;
    std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> m_mockCertifiedSender;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager> m_mockAVSConnectionManager;
    std::shared_ptr<aace::test::avs::MockMessageStorage> m_mockMessageStorage;
    std::shared_ptr<aace::test::avs::MockAttachmentManager> m_mockAttachmentManager;
    std::shared_ptr<aace::test::audio::MockAudioManagerInterface> m_mockAudioManager;
    std::shared_ptr<aace::test::audio::MockAudioOutputChannelInterface> m_mockAudioOutputChannel;
    std::shared_ptr<aace::test::audio::MockAudioInputChannelInterface> m_mockAudioInputChannel;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter> m_mockPlaybackRouter;
    std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> m_mockDialogUXStateAggregator;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor> m_mockUserInactivityMonitor;
    std::shared_ptr<aace::test::avs::MockAudioPlayerInterface> m_mockAudioPlayerInterface;

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

} // aace::test::alexa
} // aace::test
} // aace

#endif // AACE_ENGINE_TEST_ALEXA_MOCK_COMPONENT_FACTORY_H
