/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

#include <AACE/Engine/Connectivity/AlexaConnectivityEngineImpl.h>
#include <AACE/Test/Alexa/AlexaMockComponentFactory.h>
#include <AACE/Test/Alexa/AlexaTestHelper.h>
#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "AACE/Test/Connectivity/MockAlexaConnectivity.h"

namespace aace {
namespace test {
namespace connectivity {

std::string RFC3339(int offsetInHours = 0) {
    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now + std::chrono::hours(offsetInHours));
    std::stringstream ss;
    // The time zone provided by std::put_time %z doesn't include the colon separator specified in
    // RFC 3339 (https://tools.ietf.org/html/rfc3339#section-5.6). We either need to split the
    // [+-] hours from the minutes and rejoin with colon, or use std::gmtime and append Z.
    ss << std::put_time(std::gmtime(&t_c), "%FT%T");
    std::string value = ss.str() + "Z";
    return value;
}

// using namespace ::testing;
// using namespace aace::engine::connectivity;
// using namespace aace::test::connectivity;

using ::testing::_;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::Test;
using ::testing::TestWithParam;
using ::testing::Values;

class AlexaConnectivityEngineImplTest : public TestWithParam<std::pair<std::string, bool>> {
public:
    void SetUp() override {
        m_alexaMockComponentFactory = aace::test::alexa::AlexaTestHelper::createAlexaMockComponentFactory();

        // Initialize the AVS Device SDK.
        ASSERT_TRUE(alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize(
            {aace::test::alexa::AlexaTestHelper::getAVSConfig()}))
            << "Initialize AVS Device SDK Failed!";

        // Mock the ContextManagerInterface.
        m_mockContextManager = m_alexaMockComponentFactory->getContextManagerInterfaceMock();
        EXPECT_CALL(*m_mockContextManager, addStateProvider(_, NotNull())).WillRepeatedly(Return());
        EXPECT_CALL(*m_mockContextManager, removeStateProvider(_)).WillRepeatedly(Return());
        EXPECT_CALL(*m_mockContextManager, reportStateChange(_, _, _)).WillRepeatedly(Return());

        // Mock the EndpointBuilder.
        m_mockEndpointBuilder = m_alexaMockComponentFactory->getEndpointBuilderMock();

        // Mock the ConnectivityHandler and NetworkIdentifier.
        m_mockConnectivityHandler = std::make_shared<StrictMock<MockAlexaConnectivity>>();
        m_mockNetworkIdentifier = "DummyNetworkIdentifier";

        // Initialization succeeded.
        m_initialized = true;
    }

    void TearDown() override {
        if (m_initialized) {
            m_alexaMockComponentFactory->shutdown();

            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();

            m_initialized = false;
        }
    }

protected:
    void configure() {
        if (m_configured == false) {
            m_configured = true;
        }
    }

    std::shared_ptr<aace::engine::connectivity::AlexaConnectivityEngineImpl> createAlexaConnectivityEngineImpl(
        std::shared_ptr<StrictMock<MockAlexaConnectivity>> mockConnectivityHandler) {
        if (m_configured == false) {
            configure();
        }

        auto alexaConnectivityEngineImpl = aace::engine::connectivity::AlexaConnectivityEngineImpl::create(
            mockConnectivityHandler, m_mockEndpointBuilder, m_mockContextManager, m_mockNetworkIdentifier);

        return alexaConnectivityEngineImpl;
    }

protected:
    std::shared_ptr<aace::test::alexa::AlexaMockComponentFactory> m_alexaMockComponentFactory;
    std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::test::MockContextManager> m_mockContextManager;
    std::shared_ptr<alexaClientSDK::endpoints::EndpointBuilder> m_mockEndpointBuilder;  // std::unique_ptr
    std::shared_ptr<StrictMock<MockAlexaConnectivity>> m_mockConnectivityHandler;
    std::string m_mockNetworkIdentifier;

private:
    bool m_initialized = false;
    bool m_configured = false;
};

/**
 * @test createWithDefaultConnectivityState
 */
TEST_F(AlexaConnectivityEngineImplTest, createWithDefaultConnectivityState) {
    EXPECT_CALL(*m_mockConnectivityHandler, getConnectivityState()).WillOnce(Return(std::string()));

    auto alexaConnectivityEngineImpl = createAlexaConnectivityEngineImpl(m_mockConnectivityHandler);
    ASSERT_NE(alexaConnectivityEngineImpl, nullptr) << "AlexaConnectivityEngineImpl pointer expected to be not null!";

    alexaConnectivityEngineImpl->shutdown();
}

/**
 * @test createWithInitialConnectivityState
 */
TEST_F(AlexaConnectivityEngineImplTest, createWithInitialConnectivityState) {
    // clang-format off
    const std::string& value = nlohmann::json({
        {"managedProvider",{
            {"type","MANAGED"},
            {"id","AMAZON"}
        }},
        {"dataPlan",{
            {"type","AMAZON_SPONSORED"},
            {"endDate","2021-12-31T23:59:59.999Z"}
        }},
        {"dataPlansAvailable",{
            "TRIAL","PAID","AMAZON_SPONSORED"
        }},
        {"termsStatus","ACCEPTED"}
    }).dump();
    // clang-format on

    EXPECT_CALL(*m_mockConnectivityHandler, getConnectivityState()).WillOnce(Return(value));

    auto alexaConnectivityEngineImpl = createAlexaConnectivityEngineImpl(m_mockConnectivityHandler);
    ASSERT_NE(alexaConnectivityEngineImpl, nullptr) << "AlexaConnectivityEngineImpl pointer expected to be not null!";

    alexaConnectivityEngineImpl->shutdown();
}

/**
 * @test createWithInitialConnectivityStateError
 */
TEST_F(AlexaConnectivityEngineImplTest, createWithInitialConnectivityStateError) {
    EXPECT_CALL(*m_mockConnectivityHandler, getConnectivityState()).WillOnce(Return(std::string("{}")));

    auto alexaConnectivityEngineImpl = createAlexaConnectivityEngineImpl(m_mockConnectivityHandler);
    ASSERT_EQ(alexaConnectivityEngineImpl, nullptr) << "AlexaConnectivityEngineImpl pointer expected to be null!";
}

/**
 * @test createWithPlatformInterfaceAsNull
 */
TEST_F(AlexaConnectivityEngineImplTest, createWithPlatformInterfaceAsNull) {
    auto alexaConnectivityEngineImpl = createAlexaConnectivityEngineImpl(nullptr);
    ASSERT_EQ(alexaConnectivityEngineImpl, nullptr) << "AlexaConnectivityEngineImpl pointer expected to be null!";
}

/**
 * @test connectivityStateChangeAfterShutdown
 */
TEST_F(AlexaConnectivityEngineImplTest, connectivityStateChangeAfterShutdown) {
    EXPECT_CALL(*m_mockConnectivityHandler, getConnectivityState()).WillOnce(Return(std::string()));

    auto alexaConnectivityEngineImpl = createAlexaConnectivityEngineImpl(m_mockConnectivityHandler);
    ASSERT_NE(alexaConnectivityEngineImpl, nullptr) << "AlexaConnectivityEngineImpl pointer expected to be not null!";

    alexaConnectivityEngineImpl->shutdown();  // intentional shutdown here

    EXPECT_FALSE(m_mockConnectivityHandler->connectivityStateChange())
        << "Call to connectivityStateChange() expected to fail!";
}

/**
 * @test connectivityStateChangeBeforeInitialization
 */
TEST_F(AlexaConnectivityEngineImplTest, connectivityStateChangeBeforeInitialization) {
    EXPECT_FALSE(m_mockConnectivityHandler->connectivityStateChange())
        << "Call to connectivityStateChange() expected to fail!";
}

/**
 * @test connectivityStateChange/<number>
 */
TEST_P(AlexaConnectivityEngineImplTest, connectivityStateChange) {
    const std::string& value = std::get<0>(GetParam());
    bool expected = std::get<1>(GetParam());

    EXPECT_CALL(*m_mockConnectivityHandler, getConnectivityState())
        .Times(2)
        .WillOnce(Return(std::string()))  // initial connectivity state
        .WillOnce(Return(value));

    auto alexaConnectivityEngineImpl = createAlexaConnectivityEngineImpl(m_mockConnectivityHandler);
    ASSERT_NE(alexaConnectivityEngineImpl, nullptr) << "AlexaConnectivityEngineImpl pointer expected to be not null!";
    ASSERT_EQ(expected, m_mockConnectivityHandler->connectivityStateChange()) << "Unexpected result!";

    alexaConnectivityEngineImpl->shutdown();
}

INSTANTIATE_TEST_CASE_P(
    AlexaConnectivityEngineImplTest,  // prefix
    AlexaConnectivityEngineImplTest,  // test case name
    Values(                           // generator

        /**
         * @test connectivityStateChange/1
         *
         * Requires a dataPlan element when managedProvider.type is set to MANAGED.
         * Expecting connectivityStateChange() to log "dataPlanElementNotFound" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"}
                }}
                // invalid (missing mandatory dataPlan element)
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/2
         *
         * Requires dataPlan.type to be a valid DataPlanType enum string value.
         * Expecting connectivityStateChange() to log "dataPlanTypeResultNotValid" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","TPG_TELECOM"}, // invalid type (unknown)
                    {"endDate","2021-12-31T23:59:59.999Z"}
                }},
                {"dataPlansAvailable",{
                    "TRIAL","PAID","AMAZON_SPONSORED"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/3
         *
         * Requires dataPlan.endDate to be a valid RFC3339 string value.
         * Expecting connectivityStateChange() to log "dataPlanEndDateNotRFC3339" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"},
                    {"endDate","2021-12-31T23:59:59.999"} // invalid endDate (missing time zone)
                }},
                {"dataPlansAvailable",{
                    "TRIAL","PAID","AMAZON_SPONSORED"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/4
         *
         * Requires dataPlansAvailable item to be a valid DataPlanType enum string value.
         * Expecting connectivityStateChange() to log "dataPlanAvailableResultNotValid" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"},
                    {"endDate","2021-12-31T23:59:59.999Z"}
                }},
                {"dataPlansAvailable",{
                    "TRIAL","PAID","AMAZON_SPONSORED", "TPG_TELECOM" // invalid type (unknown)
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/5
         *
         * Requires a managedProvider element.
         * Expecting connectivityStateChange() to log "managedProviderElementNotFound" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                // invalid (missing mandatory managedProvider element)
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/6
         *
         * Requires managedProvider.type to be a valid ManagedProviderType enum string value.
         * Expecting connectivityStateChange() to log "managedProviderTypeResultNotValid" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","AMAZON_MANAGED"}, // invalid type (unknown)
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"},
                    {"endDate","2021-12-31T23:59:59.999Z"}
                }},
                {"dataPlansAvailable",{
                    "TRIAL","PAID","AMAZON_SPONSORED"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/7
         *
         * Requires termsStatus item to be a valid TermsStatus enum string value.
         * Expecting connectivityStateChange() to log "termsStatusResultNotValid" and return false.
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"},
                    {"endDate","2021-12-31T23:59:59.999Z"}
                }},
                {"dataPlansAvailable",{
                    "TRIAL","PAID","AMAZON_SPONSORED"
                }},
                {"termsStatus","NOT_ACCEPTED"} // invalid type (unknown)
            }).dump(),
            // clang-format on
            false),

        /**
         * @test connectivityStateChange/8
         *
         * Full Experience (Not Managed)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","NOT_MANAGED"}
                }}
            }).dump(),
            // clang-format on
            true),

        /**
         * @test connectivityStateChange/9
         *
         * Full Experience (Paid)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","PAID"}
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            true),

        /**
         * @test connectivityStateChange/10
         *
         * Full Experience (Trial)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","TRIAL"},
                    {"endDate",RFC3339(30 * 24)} // 30 days from now
                }},
                {"dataPlansAvailable",{
                    "PAID","AMAZON_SPONSORED"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            true),

        /**
         * @test connectivityStateChange/11
         *
         * Full Experience (Trial Expiring in 5 days)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","TRIAL"},
                    {"endDate",RFC3339(5 * 24)} // 5 days from now
                }},
                {"dataPlansAvailable",{
                    "PAID","AMAZON_SPONSORED"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            true),

        /**
         * @test connectivityStateChange/12
         *
         * Partial Experience (Amazon Sponsored)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"}
                }},
                {"dataPlansAvailable",{
                    "PAID","TRIAL"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            true),

        /**
         * @test connectivityStateChange/13
         *
         * Partial Experience (Terms Declined)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"}
                }},
                {"termsStatus","DECLINED"}
            }).dump(),
            // clang-format on
            true),

        /**
         * @test connectivityStateChange/14
         *
         * Partial Experience (Trial Expired)
         */
        std::make_pair(
            // clang-format off
            nlohmann::json({
                {"managedProvider",{
                    {"type","MANAGED"},
                    {"id","AMAZON"}
                }},
                {"dataPlan",{
                    {"type","AMAZON_SPONSORED"},
                }},
                {"dataPlansAvailable",{
                    "PAID"
                }},
                {"termsStatus","ACCEPTED"}
            }).dump(),
            // clang-format on
            true)

        // Values
        ),
    [](const testing::TestParamInfo<AlexaConnectivityEngineImplTest::ParamType>& info) {
        return std::to_string(info.index + 1);
    });

}  // namespace connectivity
}  // namespace test
}  // namespace aace
