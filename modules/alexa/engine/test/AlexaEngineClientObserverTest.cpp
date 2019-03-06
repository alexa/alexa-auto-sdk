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

#include "AACE/Engine/Alexa/AlexaEngineClientObserver.h"
#include "AACE/Engine/Core/EngineMacros.h"

#ifndef DEBUG
#define DEBUG 1
#endif

namespace aace {    
namespace test {
namespace unit {

using namespace alexaClientSDK;
using namespace alexaClientSDK::avsCommon;

class AlexaEngineClientObserverTest : public ::testing::Test {
public:

    void SetUp() override {
        if(DEBUG) {
            std::cout<<"Enter "<<__PRETTY_FUNCTION__<<std::endl;
        }

        m_alexaEngineClientObserver = aace::engine::alexa::AlexaEngineClientObserver::create();

        if(DEBUG) {
            std::cout<<"Exit "<<__PRETTY_FUNCTION__<<std::endl;
        }
    }

    std::shared_ptr<aace::engine::alexa::AlexaEngineClientObserver> m_alexaEngineClientObserver;
};

/**
 * Test create() with valid parameters
 */
TEST_F(AlexaEngineClientObserverTest, create) {
    EXPECT_NE(nullptr, m_alexaEngineClientObserver) << "PlaybackControllerEngineImpl pointer is null";
}

}  // namespace unit
}  // namespace test
}  // namespace aace

