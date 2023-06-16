/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_WAKEWORD_WAKEWORD_MANAGER_ENGINE_INTERFACE_H_
#define AACE_WAKEWORD_WAKEWORD_MANAGER_ENGINE_INTERFACE_H_

#include <iostream>
#include <string>

/** @file */

namespace aace {
namespace wakeword {

/**
 * WakewordManagerEngineInterface
 */
class WakewordManagerEngineInterface {
public:
    /**
     * @brief Wakeword index 
     * 
     */
    using WakeWordIndex = uint64_t;

    virtual ~WakewordManagerEngineInterface() = default;

    virtual bool onEnable3PWakeword(const std::string& wakeword, bool wakewordState) = 0;

};

}  // namespace wakeword
}  // namespace aace

#endif  // AACE_WAKEWORD_WAKEWORD_MANAGER_ENGINE_INTERFACE_H_
