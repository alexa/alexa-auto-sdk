/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_TEST_UNIT_PROPERTYMANAGER_MOCK_PROPERTYMANAGER_SERVICE_INTERFACE_H
#define AACE_TEST_UNIT_PROPERTYMANAGER_MOCK_PROPERTYMANAGER_SERVICE_INTERFACE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <AACE/Engine/PropertyManager/PropertyManagerServiceInterface.h>

namespace aace {
namespace test {
namespace unit {
namespace core {

class MockPropertyManagerServiceInterface : public aace::engine::propertyManager::PropertyManagerServiceInterface {
public:
    MOCK_METHOD1(registerProperty, bool(const aace::engine::propertyManager::PropertyDescription& propertyDescription));
    MOCK_METHOD2(updatePropertyValue, void(const std::string& name, const std::string& newValue));
    MOCK_METHOD2(
        addListener,
        bool(
            const std::string& name,
            std::shared_ptr<aace::engine::propertyManager::PropertyListenerInterface> listener));
    MOCK_METHOD2(
        removeListener,
        void(
            const std::string& name,
            std::shared_ptr<aace::engine::propertyManager::PropertyListenerInterface> listener));
    MOCK_METHOD3(setProperty, bool(const std::string&, const std::string&, const bool&));
    MOCK_METHOD1(getProperty, std::string(const std::string& name));
};

}  // namespace core
}  // namespace unit
}  // namespace test
}  // namespace aace

#endif