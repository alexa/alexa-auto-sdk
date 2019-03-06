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
#ifndef AACE_MOCK_MOCKENGINE_H_
#define AACE_MOCK_MOCKENGINE_H_

#include <gmock/gmock.h>

#include <AACE/Core/Engine.h>
#include <AACE/Core/EngineConfiguration.h>

namespace aace {
namespace core {

/**
 * Mock implementation for @c aace::core::Engine
 */
class MockEngine : public Engine {
public:
    MOCK_METHOD1(
        configure,
        bool(std::initializer_list<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList));
    MOCK_METHOD1(
        configure,
        bool(std::vector<std::shared_ptr<aace::core::config::EngineConfiguration>> configurationList));
    MOCK_METHOD1(configure, bool(std::shared_ptr<aace::core::config::EngineConfiguration> configuration));
    MOCK_METHOD0(start, bool());
    MOCK_METHOD0(stop, bool());
    MOCK_METHOD0(shutdown, bool());
    MOCK_METHOD2(setProperty, bool(const std::string& key, const std::string& value));
    MOCK_METHOD1(getProperty, std::string(const std::string& key));
    MOCK_METHOD1(registerPlatformInterface, bool(std::shared_ptr<aace::core::PlatformInterface> platformInterface));
    MOCK_METHOD1(
        registerPlatformInterface,
        bool(std::initializer_list<std::shared_ptr<aace::core::PlatformInterface>> platformInterfaceList));
};
}  // namespace core
}  // namespace aace
#endif  // AACE_MOCK_MOCKENGINE_H_