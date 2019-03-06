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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "AACE/Engine/Alexa/AlexaEngineLogger.h"
#include "AACE/Engine/Core/EngineMacros.h"

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon::avs;

// clang-format off
static const std::string CAPABILITIES_CONFIG_JSON =
    "{"
    "    \"cblAuthDelegate\":{"
    "        \"databaseFilePath\":\"cblAuthDelegate.db\""
    "    },"
    "    \"deviceInfo\":{"
    "        \"deviceSerialNumber\":\"12345678\", "
    "        \"clientId\":\"DummyClientId\","
    "        \"productId\":\"DummyProductID\""
    "    },"
    "    \"capabilitiesDelegate\":{"
    "    },"
    "    \"miscDatabase\":{"
    "        \"databaseFilePath\":\"miscDatabase.db\""
    "    },"
    "    \"alertsCapabilityAgent\":{"
    "        \"databaseFilePath\":\"alerts.db\""
    "    },"
    "    \"settings\":{"
    "        \"databaseFilePath\":\"settings.db\","
    "        \"defaultAVSClientSettings\":{"
    "            \"locale\":\"en-US\""
    "        }"
    "    },"
    "    \"bluetooth\" : {"
    "        \"databaseFilePath\":\"bluetooth.db\""
    "    },"
    "    \"certifiedSender\":{"
    "        \"databaseFilePath\":\"certifiedSender.db\""
    "    },"
    "    \"notifications\":{ "
    "        \"databaseFilePath\":\"notifications.db\""
    "    },"
    "    \"sampleApp\":{"
    "    }"
    " }";

class AlexaEngineLoggerTest : public ::testing::Test {
public:

    void SetUp() override {
        if (DEBUG) {
            std::cout << "Enter " << __PRETTY_FUNCTION__ << std::endl;
        }
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});
        if (DEBUG) {
            std::cout << "Exit " << __PRETTY_FUNCTION__ << std::endl;
        }
    }
    void TearDown() override{
        if( m_alexaEngineLogger != nullptr ){
            m_alexaEngineLogger->shutdown();
        }
    }

    std::shared_ptr<aace::engine::alexa::AlexaEngineLogger> m_alexaEngineLogger;
};

/**
 * Test create() with valid AlexaClientPlatformInterface
 */
TEST_F( AlexaEngineLoggerTest, createCriticalLogLevel ) {
        m_alexaEngineLogger = aace::engine::alexa::AlexaEngineLogger::create( 
            alexaClientSDK::avsCommon::utils::logger::Level::CRITICAL);
    EXPECT_NE(nullptr, m_alexaEngineLogger) << "AlexaLogger pointer is null";
}

TEST_F( AlexaEngineLoggerTest, createErrorLogLevel ) {     
    m_alexaEngineLogger = aace::engine::alexa::AlexaEngineLogger::create( 
            alexaClientSDK::avsCommon::utils::logger::Level::ERROR);
    EXPECT_NE(nullptr, m_alexaEngineLogger) << "AlexaLogger pointer is null";
}

TEST_F(AlexaEngineLoggerTest, createDebug9LogLevel) {
    m_alexaEngineLogger = aace::engine::alexa::AlexaEngineLogger::create(
        alexaClientSDK::avsCommon::utils::logger::Level::DEBUG9);
    EXPECT_NE(nullptr, m_alexaEngineLogger) << "AlexaLogger pointer is null";
}

TEST_F(AlexaEngineLoggerTest, createDebug0LogLevel) {
    m_alexaEngineLogger = aace::engine::alexa::AlexaEngineLogger::create(
        alexaClientSDK::avsCommon::utils::logger::Level::DEBUG0);
    EXPECT_NE(nullptr, m_alexaEngineLogger) << "AlexaLogger pointer is null";
}

TEST_F(AlexaEngineLoggerTest, createUnknownLogLevel) {
    m_alexaEngineLogger = aace::engine::alexa::AlexaEngineLogger::create(
        alexaClientSDK::avsCommon::utils::logger::Level::UNKNOWN);
    EXPECT_NE(nullptr, m_alexaEngineLogger) << "AlexaLogger pointer is null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

