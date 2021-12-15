/*
 * Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_EXTENSION_H
#define SAMPLEAPP_EXTENSION_H

#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include <AACE/Core/Engine.h>
#include <AACE/Core/MessageBroker.h>

#include "SampleApp/Activity.h"
#include "SampleApp/Views.h"
#include "SampleApp/Logger/LoggerHandler.h"

namespace sampleApp {
namespace extension {

/**
 * Base class for an extension.
 * A new extension inherits from this class, and uses the
 * `ENABLE_EXTENSION` macro defined in this file as the first
 * line of the class definition. The implementation file then
 * uses the `REGISTER_EXTENSION` macro to register the extension
 * by passing a name for the extension and the defined extension
 * class.
 */
class Extension {
public:
    // Destructor
    virtual ~Extension() = default;
    // Constructor
    Extension(const std::string& name) : m_name{name} {
    }

public:
    // Initialize extension with application resources
    virtual bool initialize(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker) = 0;
    // Validate extension
    virtual bool validate(const std::vector<nlohmann::json>& configs) = 0;
    // Get extension name
    std::string getName() {
        return m_name;
    }

private:
    // Extension name
    std::string m_name;
};

/**
 * Maintains a registry of registered extensions. Provides
 * functionality to manage the lifecycle of extensions such
 * as validation and platform interface registration.
 */
class Manager {
    // declare service map type
    using ExtensionMap = std::unordered_map<std::string, std::shared_ptr<Extension>>;

public:
    // Initialize extensions
    static bool initializeExtensions(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);
    // Register an extension
    static bool registerExtension(std::shared_ptr<Extension> extension);
    // Validate registered extensions
    static bool validateExtensions(const std::vector<nlohmann::json>& configs, std::shared_ptr<View> console);
    // Get extension
    static std::shared_ptr<Extension> getExtension(const std::string& name);

private:
    // Registry iterator
    static Manager::ExtensionMap::iterator registryBegin();
    static Manager::ExtensionMap::iterator registryEnd();

    // Singleton instance getter
    static std::shared_ptr<Manager> getInstance();

    Manager() = default;

private:
    // Singleton instance for the extension Manager
    static std::shared_ptr<Manager> m_instance;
    // Extension registry
    ExtensionMap m_extensionMap;
};

}  // namespace extension
}  // namespace sampleApp

#define EXTENSION_REGISTERED_VAR_NAME s_engineService_registered
#define ENABLE_EXTENSION \
private:                 \
    static const bool EXTENSION_REGISTERED_VAR_NAME;

#define REGISTER_EXTENSION(name, T)                                                               \
    const bool T::EXTENSION_REGISTERED_VAR_NAME = []() -> bool {                                  \
        return sampleApp::extension::Manager::registerExtension(std::shared_ptr<T>(new T(name))); \
    }();

#endif  // SAMPLEAPP_EXTENSION_H
