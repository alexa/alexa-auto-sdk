/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/Engine/Arbitrator/ArbitratorEngineImpl.h>
#include <AACE/Test/Unit/Core/MockEngineConfiguration.h>


using namespace aace::arbitrator;
using namespace aace::engine::arbitrator;
using namespace ::testing;

class MockArbitratorPlatformInterface : public Arbitrator {
public:
    MOCK_METHOD3(onAgentStateUpdated, void(const std::string& assistantId, const std::string& name, AgentState state));

    MOCK_METHOD3(onDialogTerminated, void(const std::string& assistantId,const std::string& dialogId,const std::string& reason));
    MOCK_METHOD4(
        startDialogReply,
        void( const std::string& assistantId, const std::string& dialogId, const std::string& reason, const std::string& token));

};


/// Test harness for @c AuthorizationEngineImpl class
class ArbitratorEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_mockArbitratorPlatformInterface = std::make_shared<StrictMock<MockArbitratorPlatformInterface>>();
        m_arbitratorEngineImpl = createArbitratorEngineImpl();
    }

    void TearDown() override {

        if(m_arbitratorEngineImpl != nullptr) {
            m_arbitratorEngineImpl->doShutDown();
            m_arbitratorEngineImpl.reset();
        }

	 if(m_mockArbitratorPlatformInterface != nullptr) {
            m_mockArbitratorPlatformInterface.reset();
        }
    }

protected:

	std::string arbitratorConfig =
		"{"
		"		 \"Arbitrator\": {"
		"			 \"WakewordInterruption\": \"ACTIVE_AGENT_ONLY\","
		"			 \"GestureInterruption\": \"ANY_AGENT\""
		"		 }"
		"}";

    std::shared_ptr<ArbitratorEngineImpl> createArbitratorEngineImpl() {

        auto arbitratorEngineImpl =
            ArbitratorEngineImpl::create(m_mockArbitratorPlatformInterface, arbitratorConfig );
        return arbitratorEngineImpl;
    }



protected:
    /// Used to check callbacks to the platform interface.
    std::shared_ptr<MockArbitratorPlatformInterface> m_mockArbitratorPlatformInterface;


    /// Object used in the test
    std::shared_ptr<ArbitratorEngineImpl> m_arbitratorEngineImpl;
};




/**
 * Test create() with nullptrs
 */
TEST_F(ArbitratorEngineImplTest, create) {
    EXPECT_NE(nullptr, m_arbitratorEngineImpl);

}

TEST_F(ArbitratorEngineImplTest, createWithNullPlatform) {
    std::shared_ptr<aace::engine::arbitrator::ArbitratorEngineImpl> testArbitratorEngineImpl;
    testArbitratorEngineImpl = aace::engine::arbitrator::ArbitratorEngineImpl::create(
                                   nullptr,
                                   arbitratorConfig);
    EXPECT_EQ(nullptr, testArbitratorEngineImpl);
}

TEST_F(ArbitratorEngineImplTest, createWithEmptyConfig) {

    std::shared_ptr<aace::engine::arbitrator::ArbitratorEngineImpl> testArbitratorEngineImpl;
    testArbitratorEngineImpl = aace::engine::arbitrator::ArbitratorEngineImpl::create(
                                   m_mockArbitratorPlatformInterface,
                                   "");
    EXPECT_NE(nullptr, testArbitratorEngineImpl);

    testArbitratorEngineImpl->doShutDown();
    testArbitratorEngineImpl.reset();

}




