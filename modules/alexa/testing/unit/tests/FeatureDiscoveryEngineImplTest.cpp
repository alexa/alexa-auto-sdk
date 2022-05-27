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

#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#include <AACE/Test/Unit/Alexa/AlexaTestHelper.h>
#include <AACE/Test/Unit/PropertyManager/MockPropertyManagerServiceInterface.h>

#include <AACE/Engine/Alexa/FeatureDiscoveryEngineImpl.h>
#include <AACE/Engine/Alexa/AlexaComponentInterface.h>

using namespace aace::test::unit::alexa;
class MockFeatureDiscoveryPlatformInterface : public aace::alexa::FeatureDiscovery {
public:
    MOCK_METHOD2(featuresReceived, void(const std::string& requestId, const std::string& discoveryResponses));
};

class DummyAlexaEndpointInterface : public aace::engine::alexa::AlexaEndpointInterface {
public:
    std::string getAVSGateway() override {
        return "";
    }
    std::string getLWAEndpoint() override {
        return "";
    }
    std::string getACMSEndpoint() override {
        return "";
    }

    std::string getFeatureDiscoveryEndpoint() override {
        return "";
    }
};

class DummyEngineContext
        : public aace::engine::core::EngineContext
        , public aace::engine::core::EngineService
        , public std::enable_shared_from_this<DummyEngineContext> {
public:
    DummyEngineContext(const aace::engine::core::ServiceDescription& description) : EngineService(description) {
    }

    void setUp() {
        m_serviceContext =
            std::shared_ptr<aace::engine::core::EngineServiceContext>(new aace::engine::core::EngineServiceContext(
                std::dynamic_pointer_cast<aace::engine::core::EngineService>(shared_from_this())));
        m_mockPropertyManager = std::make_shared<aace::test::unit::core::MockPropertyManagerServiceInterface>();
        m_mockAlexaEndpoint = std::make_shared<DummyAlexaEndpointInterface>();
        registerServiceInterface<aace::engine::propertyManager::PropertyManagerServiceInterface>(m_mockPropertyManager);
        registerServiceInterface<aace::engine::alexa::AlexaEndpointInterface>(m_mockAlexaEndpoint);
    }

    std::shared_ptr<aace::engine::core::EngineServiceContext> getService(const std::string& type) override {
        return m_serviceContext;
    }

    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override {
        return true;
    }

    bool shutdown() override {
        m_serviceContext.reset();
        m_mockPropertyManager.reset();
        m_mockAlexaEndpoint.reset();
        return true;
    }

private:
    std::shared_ptr<aace::engine::core::EngineServiceContext> m_serviceContext;
    std::shared_ptr<aace::test::unit::core::MockPropertyManagerServiceInterface> m_mockPropertyManager;
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> m_mockAlexaEndpoint;
};

class FeatureDiscoveryEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockFeatureDiscoveryPlatformInterface>>();
        m_dummyEngineContext =
            std::make_shared<DummyEngineContext>(aace::engine::core::ServiceDescription("aace.alexa", VERSION()));

        // initialize the avs device SDK
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        // initialized succeeded
        m_initialized = true;
    }

    void TearDown() override {
        if (m_initialized) {
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
            m_initialized = false;
        }
        m_dummyEngineContext->shutdown();
        m_dummyEngineContext.reset();
        m_mockPlatformInterface.reset();
    }

protected:
    void configure() {
        if (m_configured == false) {
            m_configured = true;
        }
    }

    std::shared_ptr<aace::engine::alexa::FeatureDiscoveryEngineImpl> createFeatureDiscoveryEngineImpl() {
        if (m_configured == false) {
            configure();
        }

        auto featureDiscoveryEngineImpl =
            aace::engine::alexa::FeatureDiscoveryEngineImpl::create(m_mockPlatformInterface, m_dummyEngineContext);

        return featureDiscoveryEngineImpl;
    }

protected:
    std::shared_ptr<aace::alexa::FeatureDiscovery> m_mockPlatformInterface;
    std::shared_ptr<DummyEngineContext> m_dummyEngineContext;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

TEST_F(FeatureDiscoveryEngineImplTest, createWithNoServiceInterfaceRegistered) {
    auto featureDiscoveryEngineImpl = createFeatureDiscoveryEngineImpl();
    ASSERT_EQ(featureDiscoveryEngineImpl, nullptr) << "FeatureDiscoveryEngineImpl pointer expected to be null!";
}

TEST_F(FeatureDiscoveryEngineImplTest, createWithAlexaComponentAsNull) {
    m_dummyEngineContext->setUp();
    auto featureDiscoveryEngineImpl = createFeatureDiscoveryEngineImpl();
    ASSERT_EQ(featureDiscoveryEngineImpl, nullptr) << "FeatureDiscoveryEngineImpl pointer expected to be null!";
}

TEST_F(FeatureDiscoveryEngineImplTest, createWithPlatformInterfaceAsNull) {
    m_dummyEngineContext->setUp();
    m_mockPlatformInterface = nullptr;
    auto featureDiscoveryEngineImpl = createFeatureDiscoveryEngineImpl();
    ASSERT_EQ(featureDiscoveryEngineImpl, nullptr) << "FeatureDiscoveryEngineImpl pointer expected to be null!";
}
