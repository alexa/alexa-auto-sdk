/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/AddressBook/AddressBook.h>
#include <AACE/Engine/AddressBook/AddressBookCloudUploader.h>

namespace aace {
namespace test {
namespace unit {

class MockAuthDelegateInterface : public alexaClientSDK::avsCommon::sdkInterfaces::AuthDelegateInterface {
public:
    MOCK_METHOD1( addAuthObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) );
    MOCK_METHOD1( removeAuthObserver, void(std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface> observer) );
    MOCK_METHOD0( getAuthToken, std::string() );
    MOCK_METHOD1( onAuthFailure, void(const std::string& token) );
};

class MockNetworkInfoProvider : public aace::network::NetworkInfoProvider {
public:
    MOCK_METHOD0( getNetworkStatus, aace::network::NetworkInfoProvider::NetworkStatus() );
    MOCK_METHOD0( getWifiSignalStrength, int() );
};

class MockNetworkObservableInterface : public aace::engine::network::NetworkObservableInterface {
public:
    MOCK_METHOD1( addObserver, void(std::shared_ptr<aace::engine::network::NetworkInfoObserver> observer) );
    MOCK_METHOD1( removeObserver, void(std::shared_ptr<aace::engine::network::NetworkInfoObserver> observer) );
};

class MockAddressBookServiceInterface : public aace::engine::addressBook::AddressBookServiceInterface {
public:
    MOCK_METHOD1( addObserver, void(std::shared_ptr<aace::engine::addressBook::AddressBookObserver> observer) );
    MOCK_METHOD1( removeObserver, void(std::shared_ptr<aace::engine::addressBook::AddressBookObserver> observer) );
    MOCK_METHOD2( getEntries, bool(const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory) );
};

// clang-format off
static const std::string CAPABILITIES_CONFIG_JSON =
    "{"
    "    \"deviceInfo\":{"
    "        \"deviceSerialNumber\":\"MockAddressBookTest\", "
    "        \"clientId\":\"MockClientId\","
    "        \"productId\":\"MockProductID\""
    "    }"
    " }";

class AddressBookCloudUploaderTest : public ::testing::Test {
public:
    void SetUp() override {
        auto inString = std::shared_ptr<std::istringstream>(new std::istringstream(CAPABILITIES_CONFIG_JSON));
        alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::initialize({inString});

        m_mockAuthDelegate = std::make_shared<testing::StrictMock<MockAuthDelegateInterface>>();
        m_mockNetworkInfoProvider = std::make_shared<testing::StrictMock<MockNetworkInfoProvider>>();
        m_mockNetworkObservableInterface = std::make_shared<testing::StrictMock<MockNetworkObservableInterface>>();
        m_mockAddressBookServiceInterface = std::make_shared<testing::StrictMock<MockAddressBookServiceInterface>>();
        
        // create device info
        auto deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create(alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot());

        
        m_contactAddressBook = std::make_shared<aace::engine::addressBook::AddressBookEntity>( "1000", "TestAddressBook", aace::engine::addressBook::AddressBookType::CONTACT );
        m_navigationAddressBook = std::make_shared<aace::engine::addressBook::AddressBookEntity>( "1001", "TestAddressBook", aace::engine::addressBook::AddressBookType::NAVIGATION );

        EXPECT_CALL( *m_mockAuthDelegate, addAuthObserver(testing::_)).WillOnce(testing::Return());
        EXPECT_CALL( *m_mockNetworkObservableInterface, addObserver(testing::_)).WillOnce(testing::Return());
        EXPECT_CALL( *m_mockAddressBookServiceInterface, addObserver(testing::_)).WillOnce(testing::Return());
        
        EXPECT_CALL( *m_mockNetworkInfoProvider,getNetworkStatus()).WillOnce(testing::Return(aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED));

        m_addressBookCloudUploader = aace::engine::addressBook::AddressBookCloudUploader::create( m_mockAddressBookServiceInterface, m_mockAuthDelegate, std::move(deviceInfo),  m_mockNetworkInfoProvider, m_mockNetworkObservableInterface );
    }

    void TearDown() override {
        m_addressBookCloudUploader->shutdown();
        if( alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::isInitialized() ) {
            alexaClientSDK::avsCommon::avs::initialization::AlexaClientSDKInit::uninitialize();
        }
    }

    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> m_contactAddressBook;
    std::shared_ptr<aace::engine::addressBook::AddressBookEntity> m_navigationAddressBook;

    std::shared_ptr<aace::engine::addressBook::AddressBookCloudUploader> m_addressBookCloudUploader;
    std::shared_ptr<testing::StrictMock<MockAuthDelegateInterface>> m_mockAuthDelegate;
    std::shared_ptr<testing::StrictMock<MockNetworkInfoProvider>> m_mockNetworkInfoProvider;
    std::shared_ptr<testing::StrictMock<MockNetworkObservableInterface>> m_mockNetworkObservableInterface;
    std::shared_ptr<testing::StrictMock<MockAddressBookServiceInterface>> m_mockAddressBookServiceInterface;
};

TEST_F( AddressBookCloudUploaderTest, create ) {
    ASSERT_NE(nullptr, m_addressBookCloudUploader);
}

    
TEST_F( AddressBookCloudUploaderTest, WithNoNetworkAndAuthRefreshedAddAndRemoveAddressBookExpectNoGetEntiresCall ) {
    //First disconnect the netowrk as to make the event loop wait for all the events.
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::DISCONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    // Expect No Call for GetEntries
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).Times(0);

    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
    m_addressBookCloudUploader->addressBookRemoved( m_contactAddressBook );

    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(1).WillRepeatedly(testing::Return(std::string()));

    // Now connect the network
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    sleep(1);
}

TEST_F( AddressBookCloudUploaderTest, WithNoNetworkAndAuthRefreshedAddSingleContactAddressBookExpectNoGetEntriesCall ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::DISCONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    // Expect No Call for GetEntries
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).Times(0);

    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );

}

TEST_F( AddressBookCloudUploaderTest, WithNoNetworkAndAuthRefreshedAddSingleNavigationAddressBookExpectNoGetEntriesCall ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::DISCONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    // Expect No Call for GetEntries
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).Times(0);

    m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook );
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndNoAuthRefreshedAddSingleContactAddressBookExpectNoGetEntriesCall ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNINITIALIZED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    // Expect No Call for GetEntries
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).Times(0);

    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndNoAuthRefreshedAddSingleNavigationAddressBookExpectNoGetEntriesCall ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::UNINITIALIZED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    // Expect No Call for GetEntries
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).Times(0);

    m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook );
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddSingleContactAddressBook ) {
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(1).WillRepeatedly(testing::Return(std::string())); // Called while checking for provision status.

    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).WillOnce(testing::Return(true));

     m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
     sleep(1);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddSingleNavigationAddressBook ) {
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(1).WillRepeatedly(testing::Return(std::string())); // Called while checking for provision status.

    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1001",testing::_)).WillOnce(testing::Return(true));

    m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook );
    sleep(1);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddTwiceSameAddressBook ) {
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(1).WillRepeatedly(testing::Return(std::string())); // Called while checking for provision status.

    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1001",testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(true));


    EXPECT_TRUE( m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook ) );
    EXPECT_TRUE( m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook ) );

    sleep(1); // Sleeping here to allow call(s) to happen before TearDown.
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddContactAndNavigationAddressBookAndExpectGetEntreisCalledInSameSequence ) {
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(1).WillRepeatedly(testing::Return(std::string())); // Called while checking for provision status.

    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    {
        ::testing::InSequence dummy;

        EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1001",testing::_)).WillOnce(testing::Return(true));
    }

    EXPECT_TRUE( m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook ) );
    EXPECT_TRUE( m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook ) );

    sleep(1); // Sleeping here to allow call(s) to happen before TearDown.
}

TEST_F( AddressBookCloudUploaderTest, CheckForSequneceCallWhenAddingAndRemovingOneAddressBook ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(4)) // 1 for provisioning check, rest is the uploading of entries with retry.
        .WillRepeatedly(testing::Return(std::string()));

    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    {
        ::testing::InSequence dummy;

        EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).WillOnce(testing::Return(true));
    }

    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
    sleep(1); // Allow Add Event to schedule.

    m_addressBookCloudUploader->addressBookRemoved( m_contactAddressBook );
    sleep(1); // Allow Remove Event to schedule

    sleep(2); // Sleeping here to allow call(s) to happen before test tear down.
}

TEST_F( AddressBookCloudUploaderTest, CheckForSequneceCallWhenAddingAndRemovingTwoAddressBook ) {
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(1).WillRepeatedly(testing::Return(std::string())); // Called while checking for provision status.

    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    {
        ::testing::InSequence dummy;

        EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1000",testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(3).WillRepeatedly(testing::Return(std::string()));

        EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries("1001",testing::_)).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockAuthDelegate, getAuthToken()).Times(3).WillRepeatedly(testing::Return(std::string()));
    }

    EXPECT_TRUE(m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook ))<<"Contact AddressBook cannot be added";
    sleep(3); // Allow Add Event to schedule.

    EXPECT_TRUE(m_addressBookCloudUploader->addressBookRemoved( m_contactAddressBook ))<<"Contact AddressBook cannot be removed";
    sleep(1); // Allow Remove Event to schedule

    EXPECT_TRUE(m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook ))<<"Navigation AddressBook cannot be added";
    sleep(3); // Allow Add Event to schedule.

    EXPECT_TRUE(m_addressBookCloudUploader->addressBookRemoved( m_navigationAddressBook ))<<"Navigtion AddressBook cannot be removed";
    sleep(1); // Allow Remove Event to schedule

    sleep(2); // Sleeping here to allow call(s) to happen before test tear down.
}
    
TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddContactAddressBookTestAddingNameTwiceFails ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(3))
        .WillRepeatedly(testing::Return(std::string()));
    
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                EXPECT_TRUE( sharedRef->addName("001", "Alice", "Smith", "U"));
                EXPECT_FALSE( sharedRef->addName("001", "Dummy", "Dummy", "U"));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
    sleep(2);
}
    
TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddContactAddressBookTestAddingTwoPhoneTwicePass ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(3))
        .WillRepeatedly(testing::Return(std::string()));
    
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
    sleep(2);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedTestAddingPostalAddressForContactAddressBookFails) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Return(std::string()));
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                EXPECT_FALSE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
    sleep(2);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedTestAddingPhoneForNavigationAddressBookFails) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Return(std::string()));
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                EXPECT_FALSE( sharedRef->addPhone("001", "HOME", "123456789"));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook );
    sleep(2);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedAddContactAddressBookTestAddingTwoPostalAddressTwicePass ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(3))
        .WillRepeatedly(testing::Return(std::string()));
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );

    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Work", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook );
    sleep(2);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedTestAddingMaxAllowedPhonesFails ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(3))
        .WillRepeatedly(testing::Return(std::string()));
    
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                //1
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                //11
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                //21
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "HOME", "123456789"));
                EXPECT_TRUE( sharedRef->addPhone("001", "WORK", "123456789"));
                //31
                EXPECT_FALSE( sharedRef->addPhone("001", "WORK", "123456789"));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_contactAddressBook );
    sleep(2);
}

TEST_F( AddressBookCloudUploaderTest, WithNetworkAndAuthRefreshedTestAddingMaxAllowedPostalAddressFails ) {
    m_addressBookCloudUploader->onNetworkInfoChanged( aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED, 123 );
    m_addressBookCloudUploader->onAuthStateChange( alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::State::REFRESHED, alexaClientSDK::avsCommon::sdkInterfaces::AuthObserverInterface::Error::SUCCESS );
    
    EXPECT_CALL(*m_mockAuthDelegate, getAuthToken())
        .Times(testing::AtLeast(3))
        .WillRepeatedly(testing::Return(std::string()));
    
    EXPECT_CALL(*m_mockAddressBookServiceInterface, getEntries(testing::_,testing::_))
        .Times(testing::AtLeast(1))
        .WillRepeatedly(testing::Invoke([](const std::string& id, std::weak_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory)->bool {
            if ( auto sharedRef = factory.lock() ) {
                //1
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                //11
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                //21
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                EXPECT_TRUE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
                //31
                EXPECT_FALSE( sharedRef->addPostalAddress("001", "Home", "123 Main Street", "", "", "Santa Clara", "California", "", "95001", "US", 90, 180, 5));
            }
            return true;
        }));
    
    m_addressBookCloudUploader->addressBookAdded( m_navigationAddressBook );
    sleep(2);
}

} // aace::test::unit
} // aace::test
} // aace
