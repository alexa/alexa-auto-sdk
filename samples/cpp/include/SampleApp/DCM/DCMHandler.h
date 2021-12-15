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

#ifndef SAMPLEAPP_DCM_DCM_HANDLER_H
#define SAMPLEAPP_DCM_DCM_HANDLER_H

#include "SampleApp/Extension.h"

#include <AACE/Core/MessageBroker.h>

namespace sampleApp {
namespace dcm {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  DCMHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////
class DCMHandler : public extension::Extension {
    // Enable extension for dynamic loading
    ENABLE_EXTENSION;

public:
    DCMHandler(const std::string& name) : Extension(name){};
    virtual ~DCMHandler() = default;

    /// @name @c Extension functions
    /// @{
    bool initialize(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker) override;
    bool validate(const std::vector<nlohmann::json>& configs) override;
    /// @}
};

}  // namespace dcm
}  // namespace sampleApp

#endif  // SAMPLEAPP_DCM_DCM_HANDLER_H
