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

#include "SampleApp/DCM/DCMHandler.h"

// C++ Standard Library
#include <regex>

// Guidelines Support Library
#include <gsl/gsl-lite.hpp>

namespace sampleApp {
namespace dcm {

REGISTER_EXTENSION("DeviceClientMetrics", DCMHandler);

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DCMHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool DCMHandler::initialize(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) {
    return true;
}

bool DCMHandler::validate(const std::vector<nlohmann::json>& configs) {
    // Look for dcm config
    for (auto const& j : configs) {
        try {
            auto obj = j.at("aace.dcm");
            if (obj.is_object()) {
                return true;
            }
        } catch (nlohmann::json::exception& e) {
        }
    }

    return false;
}

}  // namespace dcm
}  // namespace sampleApp
