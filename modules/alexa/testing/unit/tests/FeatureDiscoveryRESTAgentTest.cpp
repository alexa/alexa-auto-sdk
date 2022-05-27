/*
 * Copyright 2018-2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include <gtest/gtest.h>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#include <AACE/Test/Unit/Alexa/AlexaTestHelper.h>
#include <AACE/Engine/Alexa/FeatureDiscoveryRESTAgent.h>
#include <regex>

using namespace aace::test::unit::alexa;
using HTTPResponse = alexaClientSDK::avsCommon::utils::libcurlUtils::HTTPResponse;

static const std::string TEST_ENDPOINT = "https://api.amazonalexa.com";
static const std::string TEST_DOMAIN = "TEST_DOMAIN";
static const std::string TEST_EVENT_TYPE = "TEST_EVENT_TYPE";
static const std::string TEST_LOCALE_EN_US = "en-US";
static const std::string TEST_LOCALE_FR_CA = "fr-CA";
static const std::string TEST_UTTERANCE_1 = "Alexa, how many days until New Years?";
static const std::string TEST_UTTERANCE_2 = "Alexa, what's trending?";

static std::string TEST_RESPONSE = R"(
{
    "_links": {
        "self": {
            "href": "/v1/hints?..."
        }
    },
    "_embedded": {
        "hints": [
        {
            "_links": {
                "self": {
                    "href": "/v1/hints/amzn1.alexa.hint.0"
                }
            },
            "categoryId": null,
                    "hintTags": null,
                    "id": "amzn1.alexa.hint.0",
                    "localizedContent": {
                "en_US": {
                    "captionText": "Try \"<TEST_UTTERANCE_1>\"",
                            "utteranceText": "<TEST_UTTERANCE_1>",
                            "hintText": "\"How many days until New Years?\"",
                            "descriptionText": "",
                            "ssml": "Alexa, how many days until New Years?"
                }
            },
            "touchTargets": null
        },
        {
            "_links": {
                "self": {
                    "href": "/v1/hints/amzn1.alexa.hint.1"
                }
            },
            "categoryId": null,
                    "hintTags": null,
                    "id": "amzn1.alexa.hint.1",
                    "localizedContent": {
                "en_US": {
                    "captionText": "Try \"<TEST_UTTERANCE_2>\"",
                            "utteranceText": "<TEST_UTTERANCE_2>",
                            "hintText": "\"What's trending?\"",
                            "descriptionText": "",
                            "ssml": "Alexa, what's trending?"
                }
            },
            "touchTargets": null
        }
        ]
    }
})";

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
        return TEST_ENDPOINT;
    }
};

class FeatureDiscoveryRESTAgentTest : public ::testing::Test {
public:
    void SetUp() override {
        m_alexaMockFactory = AlexaTestHelper::createAlexaMockComponentFactory();
        m_alexaEndpointInterface = std::make_shared<DummyAlexaEndpointInterface>();
        TEST_RESPONSE = std::regex_replace(TEST_RESPONSE, std::regex("<TEST_UTTERANCE_1>"), TEST_UTTERANCE_1);
        TEST_RESPONSE = std::regex_replace(TEST_RESPONSE, std::regex("<TEST_UTTERANCE_2>"), TEST_UTTERANCE_2);

        // initialize the avs device SDK
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        // initialized succeeded
        m_initialized = true;
    }

    void TearDown() override {
        if (m_initialized) {
            m_alexaMockFactory->shutdown();

            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

            m_initialized = false;
        }
    }

protected:
    std::shared_ptr<aace::engine::alexa::FeatureDiscoveryRESTAgent> createFeatureDiscoveryRESTAgent() {
        auto featureDiscoveryRESTAgent = aace::engine::alexa::FeatureDiscoveryRESTAgent::create(
            m_alexaMockFactory->getAuthDelegateInterfaceMock(), m_alexaEndpointInterface);

        return featureDiscoveryRESTAgent;
    }

protected:
    std::shared_ptr<AlexaMockComponentFactory> m_alexaMockFactory;
    std::shared_ptr<aace::engine::alexa::AlexaEndpointInterface> m_alexaEndpointInterface;

private:
    bool m_initialized = false;
};

TEST_F(FeatureDiscoveryRESTAgentTest, generateUrlFromValidParams) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";
    std::string url = featureDiscoveryRESTAgent->createUrlFromParameters(TEST_DOMAIN, TEST_EVENT_TYPE, 3);

    ASSERT_TRUE(url.find(TEST_ENDPOINT + "/v1/hints?") != std::string::npos);
    ASSERT_TRUE(url.find("domain=TEST_DOMAIN") != std::string::npos);
    ASSERT_TRUE(url.find("eventType=TEST_EVENT_TYPE") != std::string::npos);
    ASSERT_TRUE(url.find("limit=3") != std::string::npos);
}

TEST_F(FeatureDiscoveryRESTAgentTest, generateUrlFromInvalidParams) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";
    ASSERT_TRUE(featureDiscoveryRESTAgent->createUrlFromParameters(TEST_DOMAIN, "", 1).empty());
    ASSERT_TRUE(featureDiscoveryRESTAgent->createUrlFromParameters("", TEST_EVENT_TYPE, 2).empty());
    ASSERT_TRUE(featureDiscoveryRESTAgent->createUrlFromParameters(TEST_DOMAIN, TEST_EVENT_TYPE, 0).empty());
}

TEST_F(FeatureDiscoveryRESTAgentTest, getFeaturesFromSuccessHTTPResponse) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";

    HTTPResponse httpResponse;
    httpResponse.code = 200;
    httpResponse.body = TEST_RESPONSE;

    std::vector<aace::engine::alexa::FeatureDiscoveryRESTAgent::LocalizedFeature> features =
        featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(httpResponse, TEST_LOCALE_EN_US);
    ASSERT_TRUE(features.size() == 2);

    ASSERT_EQ(features[0].utteranceText, TEST_UTTERANCE_1);
    ASSERT_EQ(features[0].descriptionText, "");
    ASSERT_EQ(features[1].utteranceText, TEST_UTTERANCE_2);
    ASSERT_EQ(features[1].descriptionText, "");
}

TEST_F(FeatureDiscoveryRESTAgentTest, getFeaturesFromEmptyHTTPResponse) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";

    HTTPResponse httpResponse;
    httpResponse.code = 204;
    httpResponse.body = "";

    std::vector<aace::engine::alexa::FeatureDiscoveryRESTAgent::LocalizedFeature> features =
        featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(httpResponse, TEST_LOCALE_EN_US);
    ASSERT_TRUE(features.empty());
}

TEST_F(FeatureDiscoveryRESTAgentTest, getFeaturesFromMalformedHTTPResponse) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";

    HTTPResponse httpResponse;
    httpResponse.code = 200;
    httpResponse.body = "{malformed";

    std::vector<aace::engine::alexa::FeatureDiscoveryRESTAgent::LocalizedFeature> features =
        featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(httpResponse, TEST_LOCALE_EN_US);
    ASSERT_TRUE(features.empty());
}

TEST_F(FeatureDiscoveryRESTAgentTest, getFeaturesFromHTTPResponseWithWrongLocale) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";

    HTTPResponse httpResponse;
    httpResponse.code = 200;
    httpResponse.body = TEST_RESPONSE;

    std::vector<aace::engine::alexa::FeatureDiscoveryRESTAgent::LocalizedFeature> features =
        featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(httpResponse, TEST_LOCALE_FR_CA);
    ASSERT_TRUE(features.empty());
}

TEST_F(FeatureDiscoveryRESTAgentTest, getFeaturesFromHTTPResponseWithInvalidLocale) {
    auto featureDiscoveryRESTAgent = createFeatureDiscoveryRESTAgent();
    ASSERT_NE(featureDiscoveryRESTAgent, nullptr) << "featureDiscoveryRESTAgent pointer expected to be not null";

    HTTPResponse httpResponse;
    httpResponse.code = 200;
    httpResponse.body = TEST_RESPONSE;

    std::vector<aace::engine::alexa::FeatureDiscoveryRESTAgent::LocalizedFeature> features =
        featureDiscoveryRESTAgent->getFeaturesFromHTTPResponse(httpResponse, "  ");
    ASSERT_TRUE(features.empty());
}
