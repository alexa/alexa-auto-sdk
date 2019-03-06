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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AVSCommon/AVS/Initialization/AlexaClientSDKInit.h>

#include "AACE/ContactUploader/ContactUploader.h"
#include "AACE/Engine/ContactUploader/ContactUploaderEngineImpl.h"

namespace aace {
namespace test {
namespace unit {

class MockContactUploaderPlatformInterface : public aace::contactUploader::ContactUploader {
public:
    MOCK_METHOD2(contactsUploaderStatusChanged, void( ContactUploaderStatus status, const std::string& info ) );
};

class MockAuthDelegateInterface : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface
{
public:
    MOCK_METHOD1(addAuthObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer));
    MOCK_METHOD1(removeAuthObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer));
    MOCK_METHOD0(getAuthToken,std::string() );
    MOCK_METHOD1(onAuthFailure, void(const std::string& token));
};

// clang-format off
static const std::string CAPABILITIES_CONFIG_JSON =
    "{"
    "    \"deviceInfo\":{"
    "        \"deviceSerialNumber\":\"MyFirstAddressBook\", "
    "        \"clientId\":\"DummyClientId\","
    "        \"productId\":\"DummyProductID\""
    "    }"
    " }";

 static const std::string srcjson = "{\n"
    "    \"id\" : \"contactId\",\n"
    "    \"firstName\" : \"firstname\",\n"
    "    \"lastName\" : \"lastname\",\n"
    "    \"nickName\" : \"nickname\",\n"
    "    \"company\" : \"Amazon.com\",\n"
    "    \"addresses\" : [\n"
    "        {\n"
    "           \"type\" : \"phonenumber\",\n"
    "           \"value\" : \"1111111111\",\n"
    "           \"label\" : \"phonerawtype\"\n"
    "        },\n"
    "        {\n"
    "           \"type\" : \"phonenumber\",\n"
    "           \"value\" : \"222222222\",\n"
    "           \"label\" : \"phonerawtype\"\n"
    "        }\n"
    "      ]\n"
    "}";


class ContactUploaderEngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});
        
        m_mockPlatformInterface = std::make_shared<testing::StrictMock<MockContactUploaderPlatformInterface>>();
        m_mockAuthDelegate = std::make_shared<testing::StrictMock<MockAuthDelegateInterface>>();

        // create device info
        auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();
        m_deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create( config );

        EXPECT_CALL( *m_mockAuthDelegate, addAuthObserver(testing::_)).WillOnce(testing::Return());

        m_engineImpl = aace::engine::contactUploader::ContactUploaderEngineImpl::create(
            m_mockPlatformInterface, m_mockAuthDelegate, m_deviceInfo );
    }

    void TearDown() override {
        m_engineImpl->shutdown();
        if( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::isInitialized() ) {
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }
    }

    std::shared_ptr<aace::engine::contactUploader::ContactUploaderEngineImpl> m_engineImpl;
    std::shared_ptr<testing::StrictMock<MockContactUploaderPlatformInterface>> m_mockPlatformInterface;
    std::shared_ptr<testing::StrictMock<MockAuthDelegateInterface>> m_mockAuthDelegate;
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> m_deviceInfo;
};

TEST_F( ContactUploaderEngineImplTest, create ) {
    ASSERT_NE(nullptr, m_engineImpl);
}

} // aace::test::unit
} // aace::test
} // aace
