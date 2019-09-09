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

#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AACE/Test/Core/MockEngineConfiguration.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

namespace aace {
namespace test {
namespace alexa {

static const std::string ALEXA_CONFIG =
    "{"
    "    \"aace.alexa\":{"
    "        \"avsDeviceSDK\":{"
    "            \"cblAuthDelegate\":{"
    "                \"databaseFilePath\":\"cblAuthDelegate.db\""
    "            },"
    "            \"deviceInfo\":{"
    "                \"deviceSerialNumber\":\"12345678\", "
    "                \"clientId\":\"DummyClientId\","
    "                \"productId\":\"DummyProductID\""
    "            },"
    "            \"capabilitiesDelegate\":{"
    "            },"
    "            \"miscDatabase\":{"
    "                \"databaseFilePath\":\"miscDatabase.db\""
    "            },"
    "            \"alertsCapabilityAgent\":{"
    "                \"databaseFilePath\":\"alerts.db\""
    "            },"
    "            \"settings\":{"
    "                \"databaseFilePath\":\"settings.db\","
    "                \"defaultAVSClientSettings\":{"
    "                    \"locale\":\"en-US\""
    "                }"
    "            },"
    "            \"bluetooth\" : {"
    "                \"databaseFilePath\":\"bluetooth.db\""
    "            },"
    "            \"certifiedSender\":{"
    "                \"databaseFilePath\":\"certifiedSender.db\""
    "            },"
    "            \"notifications\":{ "
    "                \"databaseFilePath\":\"notifications.db\""
    "            }"
    "        }"
    "    }"
    "}";
    
static const std::string AVS_DEVICE_SDK_CONFIG =
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
    "    }"
    "}";


std::shared_ptr<aace::core::config::EngineConfiguration> AlexaTestHelper::createAlexaConfiguration( bool withExpectCall )
{
    auto configuration = std::make_shared<aace::test::core::MockEngineConfiguration>();
    auto stream = std::make_shared<std::stringstream>( ALEXA_CONFIG );
    
    if( withExpectCall ) {
        EXPECT_CALL( *configuration.get(), getStream() ).WillOnce( testing::Return( stream ) );
    }
    
    return configuration;
}

std::shared_ptr<AlexaMockComponentFactory> AlexaTestHelper::createAlexaMockComponentFactory() {
    return std::make_shared<AlexaMockComponentFactory>();
}

std::shared_ptr<std::istream> AlexaTestHelper::getAVSConfig() {
    return std::make_shared<std::stringstream>( AVS_DEVICE_SDK_CONFIG );
}


} // aace::test::alexa
} // aace::test
} // aace
