/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/Engine/Core/EngineImpl.h"
#include "AACE/Test/Core/CoreTestHelper.h"
#include "AACE/Core/CoreProperties.h"

using namespace aace::test::core;

/// Test harness for @c EngineImpl class
class EngineImplTest : public ::testing::Test {
public:
    void SetUp() override {
        m_engine = aace::engine::core::EngineImpl::create();
        ASSERT_NE(m_engine, nullptr) << "Create engine failed!";
    }

    void TearDown() override {
        if (m_engine != nullptr) {
            ASSERT_TRUE(m_engine->shutdown()) << "Shutdown engine failed!";

            // reset the engine
            m_engine.reset();
        }
    }

protected:
    std::shared_ptr<aace::engine::core::EngineImpl> m_engine;
};

TEST_F(EngineImplTest, configure) {
    // test negative configuration cases
    ASSERT_FALSE(m_engine->configure(nullptr)) << "Configure engine did not fail!";
    ASSERT_FALSE(m_engine->configure({})) << "Configure engine did not fail!";

    // test valid configuration
    ASSERT_TRUE(m_engine->configure(CoreTestHelper::createDefaultConfiguration())) << "Configure engine failed!";

    // test to make sure we can't configure twice
    ASSERT_FALSE(m_engine->configure(CoreTestHelper::createDefaultConfiguration(false)))
        << "Configure engine twice did not fail!";
}

TEST_F(EngineImplTest, registerPlatformInterface) {
    // test negative case when engine is not configured
    ASSERT_FALSE(m_engine->registerPlatformInterface(CoreTestHelper::createDefaultPlatformInterface()))
        << "Register platform interface did not fail!";

    // configure engine
    ASSERT_TRUE(m_engine->configure(CoreTestHelper::createDefaultConfiguration())) << "Configure engine failed!";

    // TODO: test valid platform interface

    // test valid platform interface without service handler
    ASSERT_FALSE(m_engine->registerPlatformInterface(CoreTestHelper::createDefaultPlatformInterface()))
        << "Register platform interface did not fail!";

    // test invalid platform interface
    ASSERT_FALSE(m_engine->registerPlatformInterface(nullptr)) << "Register platform interface did not fail!";
    ASSERT_FALSE(m_engine->registerPlatformInterface({})) << "Register platform interface did not fail!";
    ASSERT_FALSE(m_engine->registerPlatformInterface({nullptr})) << "Register platform interface did not fail!";
}

TEST_F(EngineImplTest, start) {
    // test start before configure
    ASSERT_FALSE(m_engine->start()) << "Start engine did not fail!";

    // test valid start
    ASSERT_TRUE(m_engine->configure(CoreTestHelper::createDefaultConfiguration())) << "Configure engine failed!";
    ASSERT_TRUE(m_engine->start()) << "Start engine failed!";

    // test to make sure we can't start twice
    ASSERT_FALSE(m_engine->start()) << "Start engine twice did not fail!";
}

TEST_F(EngineImplTest, stop) {
    // test start stop before start - should return true since the engine
    // is already stopped, this is not an error condition!
    ASSERT_TRUE(m_engine->stop()) << "Stop engine failed!";

    // test valid stop after start
    ASSERT_TRUE(m_engine->configure(CoreTestHelper::createDefaultConfiguration())) << "Configure engine failed!";
    ASSERT_TRUE(m_engine->start()) << "Start engine failed!";
    ASSERT_TRUE(m_engine->stop()) << "Stop engine failed!";

    // test to make sure we can't start twice - should return true since the engine
    // is already stopped, this is not an error condition!
    ASSERT_TRUE(m_engine->stop()) << "Stop engine failed!";
}

TEST_F(EngineImplTest, shutdown) {
    // conifgure and start the engine
    ASSERT_TRUE(m_engine->configure(CoreTestHelper::createDefaultConfiguration())) << "Configure engine failed!";
    ASSERT_TRUE(m_engine->start()) << "Start engine failed!";

    // test shutdown valid
    ASSERT_TRUE(m_engine->shutdown()) << "Shutdown engine failed!";
}
