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

#ifndef AACE_ENGINE_APL_APL_RUNTIME_PROPERTY_GENERATOR_H
#define AACE_ENGINE_APL_APL_RUNTIME_PROPERTY_GENERATOR_H

#include <unordered_map>

namespace aace {
namespace engine {
namespace apl {

/**
 * This class handles translating vehicle properties such as driving state
 * and UI contrast (day/night) into APL runtime properties. The APL runtime
 * properties are used to affect how the APL experience is rendered. 
 */
class APLRuntimePropertyGenerator {
public:
    APLRuntimePropertyGenerator();

    void handleProperty(const std::string& name, const std::string& value);
    std::string getAPLRuntimeProperties();

private:
    std::unordered_map<std::string, std::string> m_platformProperties;
};

}  // namespace apl
}  // namespace engine
}  // namespace aace

#endif
