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

#include <AACE/Test/Alexa/AlexaMockComponentFactory.h>

namespace aace {
namespace test {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aace.test.alexa.AlexaMockComponentFactory");

AlexaMockComponentFactory::AlexaMockComponentFactory( ) : alexaClientSDK::avsCommon::utils::RequiresShutdown( TAG ) {
}

void AlexaMockComponentFactory::doShutdown()
{
    if( m_mockDirectiveSequencerInterface != nullptr ) {
        m_mockDirectiveSequencerInterface->shutdown();
    }

    if( m_mockCertifiedSender != nullptr ) {
        m_mockCertifiedSender->shutdown();
    }
}

std::shared_ptr<aace::test::avs::MockCapabilitiesDelegateInterface> AlexaMockComponentFactory::getCapabilitiesDelegateInterfaceMock()
{
    if( m_mockCapabilitiesDelegateInterface == nullptr ) {
        m_mockCapabilitiesDelegateInterface = std::make_shared<aace::test::avs::MockCapabilitiesDelegateInterface>();
    }
    
    return m_mockCapabilitiesDelegateInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager> AlexaMockComponentFactory::getContextManagerInterfaceMock()
{
    if( m_mockContextManagerInterface == nullptr ) {
        m_mockContextManagerInterface = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager>();
    }

    return m_mockContextManagerInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager> AlexaMockComponentFactory::getFocusManagerInterfaceMock()
{
    if( m_mockFocusManagerInterface == nullptr ) {
        m_mockFocusManagerInterface = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockFocusManager>();
    }
    
    return m_mockFocusManagerInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender> AlexaMockComponentFactory::getMessageSenderInterfaceMock()
{
    if( m_mockMessageSenderInterface == nullptr ) {
        m_mockMessageSenderInterface = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockMessageSender>();
    }

    return m_mockMessageSenderInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender> AlexaMockComponentFactory::getExceptionEncounteredSenderInterfaceMock()
{
    if( m_mockExceptionEncounteredSenderInterface == nullptr ) {
        m_mockExceptionEncounteredSenderInterface = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockExceptionEncounteredSender>();
    }

    return m_mockExceptionEncounteredSenderInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer> AlexaMockComponentFactory::getDirectiveSequencerInterfaceMock()
{
    if( m_mockDirectiveSequencerInterface == nullptr ) {
        m_mockDirectiveSequencerInterface = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockDirectiveSequencer>();
    }
    
    return m_mockDirectiveSequencerInterface;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager> AlexaMockComponentFactory::getSpeakerManagerInterfaceMock()
{
    if( m_mockSpeakerManagerInterface == nullptr ) {
        m_mockSpeakerManagerInterface = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockSpeakerManager>();
    }
    
    return m_mockSpeakerManagerInterface;
}

std::shared_ptr<aace::test::avs::MockCustomerDataManager> AlexaMockComponentFactory::getCustomerDataManagerMock()
{
    if( m_mockCustomerDataManager == nullptr ) {
        return std::make_shared<aace::test::avs::MockCustomerDataManager>();
    }
    
    return m_mockCustomerDataManager;
}

std::shared_ptr<aace::test::avs::MockAlertsAudioFactoryInterface> AlexaMockComponentFactory::getAlertsAudioFactoryInterfaceMock()
{
    if( m_mockAlertsAudioFactoryInterface == nullptr ) {
        m_mockAlertsAudioFactoryInterface = std::make_shared<aace::test::avs::MockAlertsAudioFactoryInterface>();
    }
    
    return m_mockAlertsAudioFactoryInterface;
}

std::shared_ptr<aace::test::avs::MockNotificationsAudioFactoryInterface> AlexaMockComponentFactory::getNotificationsAudioFactoryInterfaceMock()
{
    if( m_mockNotificationsAudioFactoryInterface == nullptr ) {
        m_mockNotificationsAudioFactoryInterface = std::make_shared<aace::test::avs::MockNotificationsAudioFactoryInterface>();
    }
    
    return m_mockNotificationsAudioFactoryInterface;
}

std::shared_ptr<alexaClientSDK::certifiedSender::CertifiedSender> AlexaMockComponentFactory::getCertifiedSenderMock()
{
    if( m_mockCertifiedSender == nullptr )
    {
        m_mockCertifiedSender = alexaClientSDK::certifiedSender::CertifiedSender::create(
            getMessageSenderInterfaceMock(),
            getAVSConnectionManagerMock(),
            getMessageStorageMock(),
            getCustomerDataManagerMock() );
    }
    
    return m_mockCertifiedSender;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager> AlexaMockComponentFactory::getAVSConnectionManagerMock()
{
    if( m_mockAVSConnectionManager == nullptr ) {
        m_mockAVSConnectionManager = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockAVSConnectionManager>();
    }
    
    return m_mockAVSConnectionManager;
}

std::shared_ptr<aace::test::avs::MockMessageStorage> AlexaMockComponentFactory::getMessageStorageMock()
{
    if( m_mockMessageStorage == nullptr ) {
        m_mockMessageStorage = std::make_shared<aace::test::avs::MockMessageStorage>();
    }
    
    return m_mockMessageStorage;
}

std::shared_ptr<aace::test::avs::MockAttachmentManager> AlexaMockComponentFactory::getAttachmentManagerMock()
{
    if( m_mockAttachmentManager == nullptr ) {
        m_mockAttachmentManager = std::make_shared<aace::test::avs::MockAttachmentManager>();
    }
    
    return m_mockAttachmentManager;
}

std::shared_ptr<aace::test::audio::MockAudioManagerInterface> AlexaMockComponentFactory::getAudioManagerMock()
{
    if( m_mockAudioManager == nullptr ) {
        m_mockAudioManager = std::make_shared<aace::test::audio::MockAudioManagerInterface>();
    }
    
    return m_mockAudioManager;
}

std::shared_ptr<aace::test::audio::MockAudioOutputChannelInterface> AlexaMockComponentFactory::getAudioOutputChannelMock()
{
    if( m_mockAudioOutputChannel == nullptr ) {
        m_mockAudioOutputChannel = std::make_shared<aace::test::audio::MockAudioOutputChannelInterface>();
    }
    
    return m_mockAudioOutputChannel;
}

std::shared_ptr<aace::test::audio::MockAudioInputChannelInterface> AlexaMockComponentFactory::getAudioInputChannelMock()
{
    if( m_mockAudioInputChannel == nullptr ) {
        m_mockAudioInputChannel = std::make_shared<aace::test::audio::MockAudioInputChannelInterface>();
    }
    
    return m_mockAudioInputChannel;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter> AlexaMockComponentFactory::getPlaybackRouterMock()
{
    if( m_mockPlaybackRouter == nullptr ) {
        m_mockPlaybackRouter = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockPlaybackRouter>();
    }
    
    return m_mockPlaybackRouter;
}

std::shared_ptr<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator> AlexaMockComponentFactory::getDialogUXStateAggregatorMock()
{
    if( m_mockDialogUXStateAggregator == nullptr ) {
        m_mockDialogUXStateAggregator = std::make_shared<alexaClientSDK::avsCommon::avs::DialogUXStateAggregator>();
    }
    
    return m_mockDialogUXStateAggregator;
}

std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor> AlexaMockComponentFactory::getUserInactivityMonitorMock()
{
    if( m_mockUserInactivityMonitor == nullptr ) {
        m_mockUserInactivityMonitor = std::make_shared<alexaClientSDK::avsCommon::sdkInterfaces::test::MockUserInactivityMonitor>();
    }
    
    return m_mockUserInactivityMonitor;
}

std::shared_ptr<aace::test::avs::MockAudioPlayerInterface> AlexaMockComponentFactory::getAudioPlayerInterfaceMock()
{
    if( m_mockAudioPlayerInterface == nullptr ) {
        m_mockAudioPlayerInterface = std::make_shared<aace::test::avs::MockAudioPlayerInterface>();
    }
    
    return m_mockAudioPlayerInterface;
}

//
// platform interface mocks
//

std::shared_ptr<MockAlerts> AlexaMockComponentFactory::getAlertsMock()
{
    if( m_mockAlerts == nullptr ) {
        m_mockAlerts = std::make_shared<MockAlerts>();
    }
    
    return m_mockAlerts;
}

std::shared_ptr<MockNotifications> AlexaMockComponentFactory::getNotificationsMock()
{
    if( m_mockNotifications == nullptr ) {
        m_mockNotifications = std::make_shared<MockNotifications>();
    }
    
    return m_mockNotifications;
}

std::shared_ptr<MockAudioPlayer> AlexaMockComponentFactory::getAudioPlayerMock()
{
    if( m_mockAudioPlayer == nullptr ) {
        m_mockAudioPlayer = std::make_shared<MockAudioPlayer>();
    }
    
    return m_mockAudioPlayer;
}

std::shared_ptr<MockAuthProvider> AlexaMockComponentFactory::getAuthProviderMock()
{
    if( m_mockAuthProvider == nullptr ) {
        m_mockAuthProvider = std::make_shared<MockAuthProvider>();
    }
    
    return m_mockAuthProvider;
}

std::shared_ptr<MockSpeechSynthesizer> AlexaMockComponentFactory::getSpeechSynthesizerMock()
{
    if( m_mockSpeechSynthesier == nullptr ) {
        m_mockSpeechSynthesier = std::make_shared<MockSpeechSynthesizer>();
    }
    
    return m_mockSpeechSynthesier;
}

std::shared_ptr<MockSpeechRecognizer> AlexaMockComponentFactory::getSpeechRecognizerMock()
{
    if( m_mockSpeechRecognizer == nullptr ) {
        m_mockSpeechRecognizer = std::make_shared<MockSpeechRecognizer>();
    }
    
    return m_mockSpeechRecognizer;
}

std::shared_ptr<MockAlexaClient> AlexaMockComponentFactory::getAlexaClientMock()
{
    if( m_mockAlexaClient == nullptr ) {
        m_mockAlexaClient = std::make_shared<MockAlexaClient>();
    }
    
    return m_mockAlexaClient;
}

std::shared_ptr<MockPlaybackController> AlexaMockComponentFactory::getPlaybackControllerMock()
{
    if( m_mockPlaybackController == nullptr ) {
        m_mockPlaybackController = std::make_shared<MockPlaybackController>();
    }
    
    return m_mockPlaybackController;
}

std::shared_ptr<MockTemplateRuntime> AlexaMockComponentFactory::getTemplateRuntimeMock()
{
    if( m_mockTemplateRuntime == nullptr ) {
        m_mockTemplateRuntime = std::make_shared<MockTemplateRuntime>();
    }
    
    return m_mockTemplateRuntime;
}

} // aace::test::alexa
} // aace::test
} // aace

