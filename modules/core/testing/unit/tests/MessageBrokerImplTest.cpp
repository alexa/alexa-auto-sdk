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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include <chrono>

// testing includes
#include <AACE/Test/Unit/Core/CoreTestHelper.h>
// engine includes
#include <AACE/Engine/MessageBroker/Message.h>
#include <AACE/Engine/MessageBroker/MessageBrokerImpl.h>
// platform includes
#include <AACE/Core/CoreProperties.h>

using namespace aace::test::unit::core;
using aace::engine::messageBroker::Message;

/// Test harness for @c MessageBrokerImpl class
class MessageBrokerImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_broker = aace::engine::messageBroker::MessageBrokerImpl::create();
        ASSERT_NE(m_broker, nullptr) << "Create message broker failed!";
        m_broker->setMessageTimeout(std::chrono::milliseconds{500});
    }

    void TearDown() override {
        if (m_broker != nullptr) {
            m_broker->shutdown();
            m_broker.reset();
        }
    }

protected:
    std::shared_ptr<aace::engine::messageBroker::MessageBrokerImpl> m_broker;
};

static auto SAMPLE_REQUEST = R"({
  "header": {
    "id": "23b578ed-6dc3-460a-998e-1647ba6cde42",
    "messageType": "Publish",
    "version": "4.0",
    "messageDescription": {
        "topic": "LocationProvider",
        "action": "GetLocation"
    }
  }
})";

static auto SAMPLE_REPLY = R"({
  "header": {
    "id": "4c4d13b6-6a8d-445b-931a-a3feb0878311",
    "messageType": "Reply",
    "version": "4.0",
    "messageDescription": {
      "topic": "LocationProvider",
      "action": "GetLocation",
      "replyToId": "23b578ed-6dc3-460a-998e-1647ba6cde42"
    }
  },
  "payload": {
    "location": {
      "latitude": 37.410,
      "longitude": -122.025
    }
  }
})";

TEST_F(MessageBrokerImplTest, sendEmptyMessage) {
    auto reply = m_broker->publish("{}").get();
    ASSERT_FALSE(reply.valid());
}

TEST_F(MessageBrokerImplTest, happyPath) {
    m_broker->subscribe(
        "LocationProvider",
        [=](Message message) { m_broker->publish(SAMPLE_REPLY).send(); },
        Message::Direction::OUTGOING);
    auto reply = m_broker->publish(SAMPLE_REQUEST).get();
    ASSERT_TRUE(reply.valid());
}

TEST_F(MessageBrokerImplTest, messageTimeout) {
    m_broker->subscribe(
        "LocationProvider",
        [=](Message message) {
            // do nothing
        },
        Message::Direction::OUTGOING);
    auto reply = m_broker->publish(SAMPLE_REQUEST).get();
    // The reply should be invalid
    ASSERT_FALSE(reply.valid());
}

TEST_F(MessageBrokerImplTest, noSubscriberNoWait) {
    auto pm = m_broker->publish(SAMPLE_REQUEST);
    auto startTime = std::chrono::system_clock::now();
    auto reply = pm.get();
    auto duration = std::chrono::system_clock::now() - startTime;

    // Should not wait for the reply since there is no subscriber
    ASSERT_TRUE(duration < pm.timeout() / 2);
    ASSERT_FALSE(reply.valid());
}
