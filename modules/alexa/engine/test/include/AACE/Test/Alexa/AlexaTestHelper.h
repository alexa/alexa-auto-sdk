/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_TEST_ALEXA_TEST_HELPER_H
#define AACE_ENGINE_TEST_ALEXA_TEST_HELPER_H

#include <AACE/Core/EngineConfiguration.h>
#include "AlexaMockComponentFactory.h"
#include <gtest/gtest.h>
#include <istream>

namespace aace {
namespace test {
namespace alexa {

class AlexaTestHelper {
public:
    static std::shared_ptr<aace::core::config::EngineConfiguration> createAlexaConfiguration(
        bool withExpectCall = true);
    static std::shared_ptr<AlexaMockComponentFactory> createAlexaMockComponentFactory();
    static std::shared_ptr<std::istream> getAVSConfig();
};

}  // namespace alexa
}  // namespace test
}  // namespace aace

#endif  // AACE_ENGINE_TEST_ALEXA_TEST_HELPER_H
