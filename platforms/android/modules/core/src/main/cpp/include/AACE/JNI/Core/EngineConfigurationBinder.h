/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_CORE_CONFIG_ENGINE_CONFIGURATION_BINDER_H
#define AACE_JNI_CORE_CONFIG_ENGINE_CONFIGURATION_BINDER_H

#include <AACE/Core/EngineConfiguration.h>

namespace aace {
namespace jni {
namespace core {
namespace config {

class EngineConfigurationBinder {
public:
    EngineConfigurationBinder(std::shared_ptr<aace::core::config::EngineConfiguration> config) : m_config(config) {
    }

    std::shared_ptr<aace::core::config::EngineConfiguration> getConfig() {
        return m_config;
    }

private:
    std::shared_ptr<aace::core::config::EngineConfiguration> m_config;
};

}  // namespace config
}  // namespace core
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_CORE_CONFIG_ENGINE_CONFIGURATION_BINDER_H
