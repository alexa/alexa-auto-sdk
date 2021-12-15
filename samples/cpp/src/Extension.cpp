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

#include "SampleApp/Extension.h"

namespace sampleApp {
namespace extension {

std::shared_ptr<Manager> Manager::m_instance;

Manager::ExtensionMap::iterator Manager::registryBegin() {
    return getInstance()->m_extensionMap.begin();
}

Manager::ExtensionMap::iterator Manager::registryEnd() {
    return getInstance()->m_extensionMap.end();
}

bool Manager::registerExtension(std::shared_ptr<Extension> ext) {
    if (getInstance()->m_extensionMap.find(ext->getName()) == getInstance()->m_extensionMap.end()) {
        getInstance()->m_extensionMap[ext->getName()] = ext;
        return true;
    }
    return false;
}

std::shared_ptr<Manager> Manager::getInstance() {
    if (m_instance == nullptr) {
        m_instance = std::shared_ptr<Manager>(new Manager());
    }
    return m_instance;
}

bool Manager::initializeExtensions(
    std::weak_ptr<Activity> activity,
    std::weak_ptr<logger::LoggerHandler> loggerHandler,
    std::shared_ptr<aace::core::MessageBroker> messageBroker) {
    // Initialize extensions
    bool initialized = true;
    for (auto& it : getInstance()->m_extensionMap) {
        auto extension = it.second;
        if (!extension->initialize(activity, loggerHandler, messageBroker)) {
            initialized = false;
        }
    }
    return initialized;
}

bool Manager::validateExtensions(const std::vector<nlohmann::json>& configs, std::shared_ptr<View> console) {
    // Validate extensions
    bool validated = true;
    for (auto& it : getInstance()->m_extensionMap) {
        auto extension = it.second;
        console->printRuler();
        console->print("Registered Extension: " + extension->getName());
        if (extension->validate(configs)) {
            console->printLine(" passed validation");
        } else {
            console->printLine(" failed validation");
            validated = false;
        }
        console->printRuler();
    }
    return validated;
}

std::shared_ptr<Extension> Manager::getExtension(const std::string& name) {
    auto extensionIt = getInstance()->m_extensionMap.find(name);
    if (extensionIt != registryEnd()) {
        return extensionIt->second;
    }
    return nullptr;
}

}  // namespace extension
}  // namespace sampleApp
