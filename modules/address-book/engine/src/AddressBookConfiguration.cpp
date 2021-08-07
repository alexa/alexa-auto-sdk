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

#include "AACE/AddressBook/AddressBookConfiguration.h"

#include <nlohmann/json.hpp>

namespace aace {
namespace addressBook {
namespace config {

std::shared_ptr<aace::core::config::EngineConfiguration> AddressBookConfiguration::createAddressBookConfig(
    bool cleanAllAddressBooksAtStart) {
    // clang-format off
    nlohmann::json config = {
        {"aace.addressBook", {
            {"cleanAllAddressBooksAtStart", cleanAllAddressBooksAtStart}
        }}
    };
    // clang-format on
    auto stream = std::make_shared<std::stringstream>(config.dump());
    return aace::core::config::StreamConfiguration::create(stream);
}

}  // namespace config
}  // namespace addressBook
}  // namespace aace