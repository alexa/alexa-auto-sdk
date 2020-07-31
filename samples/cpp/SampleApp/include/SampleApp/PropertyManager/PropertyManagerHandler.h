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

#ifndef SAMPLEAPP_PROPERTY_MANAGER_PROPERTYMANAGERHANDLER_H
#define SAMPLEAPP_PROPERTY_MANAGER_PROPERTYMANAGERHANDLER_H

#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/PropertyManager/PropertyManager.h>

namespace sampleApp {
namespace propertyManager {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PropertyManagerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class PropertyManagerHandler : public aace::propertyManager::PropertyManager /* isa PlatformInterface */ {
private:
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler{};

protected:
    PropertyManagerHandler(std::weak_ptr<logger::LoggerHandler> loggerHandler);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<PropertyManagerHandler> {
        return std::shared_ptr<PropertyManagerHandler>(new PropertyManagerHandler(args...));
    }
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

    // aace::propertyManager::PropertyManager interface
    auto propertyChanged(const std::string& key, const std::string& newValue) -> void override;
    auto propertyStateChanged(const std::string& name, const std::string& value, const PropertyState state)
        -> void override;

private:
    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    std::weak_ptr<View> m_console{};
};

}  // namespace propertyManager
}  // namespace sampleApp

#endif  // SAMPLEAPP_PROPERTY_MANAGER_PROPERTYMANAGERHANDLER_H
