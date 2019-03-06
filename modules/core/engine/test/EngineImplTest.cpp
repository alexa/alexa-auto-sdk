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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

#include "AACE/Core/Engine.h"
#include "AACE/Core/EngineConfiguration.h"

namespace aace {
namespace test {
namespace unit {

/**
 * This class allows us to test EngineConfiguration interaction
 */
class MockEngineConfiguration : public aace::core::config::EngineConfiguration
{
public:
    MockEngineConfiguration() : aace::core::config::EngineConfiguration{} {
    }
    MOCK_METHOD0(getStream, std::shared_ptr<std::istream>());
};

/// Test harness for @c EngineImpl class
class EngineImplTest : public::testing::Test {
public:
    void SetUp() override {
    m_engineImpl = aace::core::Engine::create();
    m_sstream = std::make_shared<std::stringstream>();
    }

    void TearDown() override {
        m_engineImpl.reset();// dispose pointer
        m_sstream.reset();
        EXPECT_EQ(nullptr, m_engineImpl) << "Engine creation did not return null";
    }

    std::shared_ptr<aace::core::Engine> m_engineImpl;
    std::shared_ptr<std::istream> m_sstream;
    
    /**
     * Configure engine
     */
    void configureEngine() {
        std::shared_ptr<MockEngineConfiguration> engineConfiguration = std::make_shared<MockEngineConfiguration>();
        EXPECT_CALL(*engineConfiguration.get(), getStream()).Times(1).WillRepeatedly(testing::Return(m_sstream));
        ASSERT_TRUE(m_engineImpl->configure(engineConfiguration)) << "Configure did not return True";
    }

};
    


/**
 * Test create() expecting a valid EngineImpl to be returned.
 */
TEST_F(EngineImplTest, create) {
    EXPECT_NE(nullptr, m_engineImpl) << "Engine creation returned null";
}

/**
 * Test configure() with correct config file.
 */
TEST_F(EngineImplTest, configure) {
    configureEngine();
}

/**
 * Test configure() with null pointer.
 */
TEST_F(EngineImplTest, configureNullPointer) {
    EXPECT_FALSE(m_engineImpl->configure(nullptr)) << "Configure did not return True";
}

/**
 * Test configure() with correct config list.
 */
TEST_F(EngineImplTest, configureList) {
    //TODO: use list to configure engine
    configureEngine();
}

/**
 * Test configure() with empty config list.
 */
TEST_F(EngineImplTest, configureListEmpty) {
    std::shared_ptr<MockEngineConfiguration> engineConfiguration = std::make_shared<MockEngineConfiguration>();
    EXPECT_CALL(*engineConfiguration.get(), getStream()).Times(0);
    EXPECT_TRUE(m_engineImpl->configure({})) << "Configure did not return True";
}

/**
 * Test engine start().
 */
TEST_F(EngineImplTest, engineStart) {
    configureEngine();
    ASSERT_TRUE(m_engineImpl->start()) << "engine should be started successfully";
}

/**
 * Test engine start before engine configure.
 */
TEST_F(EngineImplTest, engineStartBeforeConfigure) {
    ASSERT_FALSE(m_engineImpl->start()) << "engine should be congfigured before start";
}

/**
 * Test engine start twice without stopping the engine.
 */
TEST_F(EngineImplTest, engineStartTwice) {
    configureEngine();
    ASSERT_TRUE(m_engineImpl->start()) << "engine should be started successfully";
    ASSERT_FALSE(m_engineImpl->start()) << "engine started before stopping";

}

/**
 * Test engine stop.
 */
TEST_F(EngineImplTest, engineStop) {
    configureEngine();
    ASSERT_TRUE(m_engineImpl->start()) << "engine should be started successfully";
    ASSERT_TRUE(m_engineImpl->stop()) << "engine did not stop successfully";
}

/**
 * Test engine stop without start.
 */
TEST_F(EngineImplTest, engineStopWithoutStart) {
    ASSERT_TRUE(m_engineImpl->stop()) << "engine did not stop successfully";
}

/**
 * Test register platform interface with null pointer.
 */
TEST_F(EngineImplTest, registerPlatformInterfaceNull) {
    configureEngine();
    ASSERT_TRUE(m_engineImpl->start()) << "engine should be started successfully";
    ASSERT_FALSE(m_engineImpl->registerPlatformInterface(nullptr)) << "Platform Interface not registered";
}

/**
 * Test setProperty with empty key and value.
 */
TEST_F(EngineImplTest, setPropertyEmptyKey) {
    ASSERT_FALSE(m_engineImpl->setProperty("", "")) << "Set Property did not return False";
}


}  // namespace unit
}  // namespace test
}  // namespace aace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
