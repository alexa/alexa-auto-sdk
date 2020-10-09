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

#ifndef AASB_ENGINE_PROPERTY_MANAGER_AASB_PROPERTY_MANAGER_H
#define AASB_ENGINE_PROPERTY_MANAGER_AASB_PROPERTY_MANAGER_H

#include <AACE/PropertyManager/PropertyManager.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

#include <string>
#include <memory>

namespace aasb {
namespace engine {
namespace propertyManager {

class AASBPropertyManager
        : public aace::propertyManager::PropertyManager
        , public std::enable_shared_from_this<AASBPropertyManager> {
private:
    AASBPropertyManager() = default;

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBPropertyManager> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::propertyManager::PropertyManager
    void propertyChanged(const std::string& name, const std::string& newValue) override;
    void propertyStateChanged(const std::string& name, const std::string& value, const PropertyState state) override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace propertyManager
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_PROPERTY_MANAGER_AASB_PROPERTY_MANAGER_H
