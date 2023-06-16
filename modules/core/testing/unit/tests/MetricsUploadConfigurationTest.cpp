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

#include <sstream>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <nlohmann/json.hpp>

#include <AACE/Engine/Metrics/CounterDataPointBuilder.h>
#include <AACE/Engine/Metrics/DurationDataPointBuilder.h>
#include <AACE/Engine/Metrics/MetricEventBuilder.h>
#include <AACE/Engine/Metrics/MetricsUploadConfiguration.h>
#include <AACE/Engine/Metrics/StringDataPointBuilder.h>
#include <AACE/Engine/Utils/Agent/AgentId.h>

using namespace aace::engine::metrics;
using json = nlohmann::json;

static const std::string UPLOAD_CONFIG = R"([
    {
        "program": "AlexaAutoSDK",
        "source": "CBL-CblRequest",
        "uploadRules": [
            {
                "assistantId": 2,
                "groupId": "abcdefg",
                "schemaId": "abcd/1/00000000"
            }
        ]
    },
    {
        "program": "AlexaAutoSDK",
        "source": "ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest",
        "uploadRules": [
            {
                "assistantId": 2,
                "groupId": "abcdefg",
                "schemaId": "abcd/1/00000001",
                "names": ["RequestSuccessCount", "RequestLatency", "RequestFailureCount"]
            }
        ]
    },
    {
        "program": "AlexaAutoSDK",
        "source": "EMP-ExternalMediaPlayer",
        "uploadRules": [
            {
                "assistantId": 2,
                "groupId": "abcdefg",
                "schemaId": "abcd/1/00000002",
                "names": [".*Count$", ".*Latency$"]
            }
        ]
    },
    {
        "program": "AlexaAutoSDK",
        "source": "AIP-INITIATOR_TAP",
        "uploadRules": [
            {
                "assistantId": 2,
                "groupId": "abcdefg",
                "schemaId": "abcd/1/00000003",
                "names": ["TestRuleThatWontMatchAnything"]
            }
        ]
    },
    {
        "program": "AlexaAutoSDK",
        "source": "CUSTOM-THINKING_TIMEOUT_EXPIRES",
        "uploadRules": [
            {
                "assistantId": 2,
                "groupId": "abcdefg",
                "schemaId": "abcd/1/00000004"
            },
            {
                "assistantId": 100,
                "groupId": "gfedcba",
                "schemaId": "gfed/1/00000000"
            }
        ]
    }
])";

/// Test harness for @c MetricsUploadConfiguration class
class MetricsUploadConfigurationTest : public ::testing::Test {
public:
    void SetUp() override {
        json configJ = json::parse(UPLOAD_CONFIG);
        auto configMap = MetricsUploadConfiguration::createConfigurations(configJ);
        EXPECT_EQ(true, configMap.size() == 2)
            << "Upload config includes 2 agents. createConfigurations() map should have 2 entries but has "
            << configMap.size();
        const auto& agent2Itr = configMap.find(2);
        const auto& agent100Itr = configMap.find(100);
        EXPECT_NE(configMap.end(), agent2Itr) << "MetricsUploadConfiguration for agent 2 is not present";
        EXPECT_NE(configMap.end(), agent100Itr) << "MetricsUploadConfiguration for agent 100 is not present";
        m_configurationAgent2 = std::make_shared<MetricsUploadConfiguration>(agent2Itr->second);
        m_configurationAgent100 = std::make_shared<MetricsUploadConfiguration>(agent100Itr->second);
        EXPECT_EQ(true, m_configurationAgent2->size() == 5)
            << "MetricsUploadConfiguration for agent 2 has unexpected size; expected 5, actual "
            << m_configurationAgent2->size();
        EXPECT_EQ(true, m_configurationAgent100->size() == 1)
            << "MetricsUploadConfiguration for agent 100 has unexpected size; expected 1, actual "
            << m_configurationAgent100->size();
    }

protected:
    std::shared_ptr<MetricsUploadConfiguration> m_configurationAgent2;
    std::shared_ptr<MetricsUploadConfiguration> m_configurationAgent100;
};

TEST_F(MetricsUploadConfigurationTest, noDataPointFilter) {
    auto metricBuilder =
        MetricEventBuilder()
            .withSourceName("CBL-CblRequest")
            .withAlexaAgentId()
            .addDataPoint(CounterDataPointBuilder{}.withName("CblRequestSuccessCount").increment(1).build())
            .addDataPoint(StringDataPointBuilder{}.withName("CblRequestType").withValue("CodePairRequest").build());
    MetricEvent metric = metricBuilder.build();
    EXPECT_TRUE(m_configurationAgent2->isAllowed(metric)) << "CBL-CblRequest expected to be allowed for agent 2";
    EXPECT_FALSE(m_configurationAgent100->isAllowed(metric))
        << "CBL-CblRequest not expected to be allowed for agent 100";
}

TEST_F(MetricsUploadConfigurationTest, multipleAgentsAllowed) {
    auto metricBuilder =
        MetricEventBuilder()
            .withSourceName("CUSTOM-THINKING_TIMEOUT_EXPIRES")
            .addDataPoint(CounterDataPointBuilder{}.withName("THINKING_TIMEOUT_EXPIRES").increment(1).build());
    MetricEvent metric = metricBuilder.build();
    EXPECT_TRUE(m_configurationAgent2->isAllowed(metric))
        << "CUSTOM-THINKING_TIMEOUT_EXPIRES expected to be allowed for agent 2";
    EXPECT_TRUE(m_configurationAgent100->isAllowed(metric))
        << "CUSTOM-THINKING_TIMEOUT_EXPIRES expected to be allowed for agent 100";
}

TEST_F(MetricsUploadConfigurationTest, dataPointFilterExactNameMatch) {
    auto metricBuilder1 =
        MetricEventBuilder()
            .withSourceName("ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest")
            .withAlexaAgentId()
            .addDataPoint(CounterDataPointBuilder{}.withName("RequestFailureCount").increment(1).build())
            .addDataPoint(
                StringDataPointBuilder{}.withName("AddressBookRequestType").withValue("Request.Add.Contacts").build());
    MetricEvent metric1 = metricBuilder1.build();
    EXPECT_TRUE(m_configurationAgent2->isAllowed(metric1))
        << "ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest: RequestFailureCount data point expected to be allowed for "
           "agent 2";
    EXPECT_FALSE(m_configurationAgent100->isAllowed(metric1))
        << "ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest not expected to be allowed for agent 100";

    auto metricBuilder2 =
        MetricEventBuilder()
            .withSourceName("ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest")
            .withAlexaAgentId()
            .addDataPoint(CounterDataPointBuilder{}.withName("ANameThatWontMatch").increment(1).build())
            .addDataPoint(StringDataPointBuilder{}
                              .withName("AddressBookRequestType")
                              .withValue("Request.Remove.Contacts")
                              .build());
    MetricEvent metric2 = metricBuilder2.build();
    EXPECT_FALSE(m_configurationAgent2->isAllowed(metric2))
        << "ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest: ANameThatWontMatch data point not expected to be allowed "
           "for agent 2";
    EXPECT_FALSE(m_configurationAgent100->isAllowed(metric2))
        << "ADDRESS_BOOK_CLOUD_UPLOADER-AddressBookRequest: ANameThatWontMatch data point not expected to be allowed "
           "for agent 100";
}

TEST_F(MetricsUploadConfigurationTest, dataPointFilterEndPatternMatch) {
    auto metricBuilder1 =
        MetricEventBuilder()
            .withSourceName("EMP-ExternalMediaPlayer")
            .addDataPoint(CounterDataPointBuilder{}.withName("EMPRequestCount").increment(1).build())
            .addDataPoint(StringDataPointBuilder{}.withName("RequestType").withValue("PlayPreset").build())
            .addDataPoint(StringDataPointBuilder{}.withName("PlayerID").withValue("FM_RADIO").build());
    MetricEvent metric1 = metricBuilder1.build();
    EXPECT_TRUE(m_configurationAgent2->isAllowed(metric1))
        << "EMP-ExternalMediaPlayer: EMPRequestCount data point expected to be allowed for agent 2";
    EXPECT_FALSE(m_configurationAgent100->isAllowed(metric1))
        << "EMP-ExternalMediaPlayer not expected to be allowed for agent 100";

    auto metricBuilder2 =
        MetricEventBuilder()
            .withSourceName("EMP-ExternalMediaPlayer")
            .addDataPoint(CounterDataPointBuilder{}.withName("TestCountThatWontMatch").increment(1).build())
            .addDataPoint(StringDataPointBuilder{}.withName("RequestType").withValue("PlayPreset").build())
            .addDataPoint(StringDataPointBuilder{}.withName("PlayerID").withValue("FM_RADIO").build());
    MetricEvent metric2 = metricBuilder2.build();
    EXPECT_FALSE(m_configurationAgent2->isAllowed(metric2))
        << "EMP-ExternalMediaPlayer: TestCountThatWontMatch not expected to be allowed for agent 2";
    EXPECT_FALSE(m_configurationAgent100->isAllowed(metric2))
        << "EMP-ExternalMediaPlayer: TestCountThatWontMatch not expected to be allowed for agent 100";
}

TEST_F(MetricsUploadConfigurationTest, dataPointFilterNoNameMatch) {
    auto metricBuilder1 = MetricEventBuilder()
                              .withSourceName("AIP-INITIATOR_TAP")
                              .withAlexaAgentId()
                              .addDataPoint(CounterDataPointBuilder{}.withName("INITIATOR_TAP").increment(1).build());
    MetricEvent metric1 = metricBuilder1.build();
    EXPECT_FALSE(m_configurationAgent2->isAllowed(metric1))
        << "AIP-INITIATOR_TAP not expected to be allowed for agent 2";
    EXPECT_FALSE(m_configurationAgent100->isAllowed(metric1))
        << "AIP-INITIATOR_TAP not expected to be allowed for agent 100";
}