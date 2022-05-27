/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <functional>
#include <istream>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "AACE/Alexa/LocalMediaSource.h"
#include "AVSCommon/AVS/Initialization/AlexaClientSDKInit.h"
#include "AACE/Test/Unit/Alexa/AlexaTestHelper.h"
#include "AACE/Test/Unit/Alexa/MockExternalMediaAdapterRegistrationInterface.h"
#include "AACE/Test/Unit/Alexa/MockLocalMediaSource.h"
#include "AACE/Test/Unit/AVS/MockEndpointCapabilitiesRegistrarInterface.h"
#include "AACE/Test/Unit/AVS/MockPlaybackRouterInterface.h"
#include "AACE/Engine/Alexa/ExternalMediaPlayerEngineImpl.h"

using namespace aace::test::unit::alexa;
using namespace aace::test::unit::avs;

// The main test

class ExternalMediaPlayerEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();

        // initialize the avs device SDK
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";
    }

    void TearDown() override {
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
};

TEST_F(ExternalMediaPlayerEngineImplTest, getAdapterStatesAndShutdown) {
    auto mockEndpointCapabilitiesRegistrarInterface = std::make_shared<MockEndpointCapabilitiesRegistrarInterface>();
    EXPECT_CALL(
        *mockEndpointCapabilitiesRegistrarInterface,
        withCapability(
            testing::Matcher<
                const std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilityConfigurationInterface>&>(
                testing::_),
            testing::_))
        .WillOnce(testing::ReturnRef(*mockEndpointCapabilitiesRegistrarInterface));
    auto empEngineImpl = aace::engine::alexa::ExternalMediaPlayerEngineImpl::create(
        "Test",
        mockEndpointCapabilitiesRegistrarInterface,                            // EndpointCapabilitiesRegistrarInterface
        m_alexaMockFactory->getSpeakerManagerInterfaceMock(),                  // SpeakerManagerInterface
        m_alexaMockFactory->getMessageSenderInterfaceMock(),                   // MessageSenderInterface
        m_alexaMockFactory->getCertifiedSenderMock(),                          // certifiedMessageSender
        m_alexaMockFactory->getFocusManagerInterfaceMock(),                    // FocusManagerInterface
        m_alexaMockFactory->getContextManagerInterfaceMock(),                  // ContextManagerInterface
        m_alexaMockFactory->getExceptionEncounteredSenderInterfaceMock(),      // ExceptionEncounteredSenderInterface
        m_alexaMockFactory->getPlaybackRouterMock(),                           // PlaybackRouterInterface
        std::make_shared<aace::engine::alexa::AudioPlayerObserverDelegate>(),  // AudioPlayerObserverDelegate
        std::make_shared<MockExternalMediaAdapterRegistrationInterface>(),  // ExternalMediaAdapterRegistrationInterface
        false                                                               // duckingEnabled
    );
    ASSERT_NE(empEngineImpl, nullptr) << "ExternalMediaPlayerEngineImpl pointer expected to be not null!";

    for (auto source : {aace::alexa::LocalMediaSource::Source::BLUETOOTH,
                        aace::alexa::LocalMediaSource::Source::USB,
                        aace::alexa::LocalMediaSource::Source::FM_RADIO,
                        aace::alexa::LocalMediaSource::Source::AM_RADIO,
                        aace::alexa::LocalMediaSource::Source::SATELLITE_RADIO,
                        aace::alexa::LocalMediaSource::Source::LINE_IN,
                        aace::alexa::LocalMediaSource::Source::COMPACT_DISC,
                        aace::alexa::LocalMediaSource::Source::SIRIUS_XM,
                        aace::alexa::LocalMediaSource::Source::DAB}) {
        auto mockLMS = std::make_shared<MockLocalMediaSource>(source);
        empEngineImpl->registerPlatformMediaAdapter(mockLMS);
    }

    std::thread t1([empEngineImpl] {
        for (int i = 0; i < 100; ++i) {
            empEngineImpl->getAdapterStates(true);
        }
    });
    empEngineImpl->shutdown();
    t1.join();
}
