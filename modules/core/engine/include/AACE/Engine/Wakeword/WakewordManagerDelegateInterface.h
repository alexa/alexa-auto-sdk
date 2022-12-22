/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_DELEGATE_INTERFACE_H
#define AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_DELEGATE_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace wakeword {

class WakewordManagerDelegateInterface {
public:
    /**
     * Enables/Disable 3P Wakeword detection.
     * 
     * @param wakeword  The wakeword to be enabled.
     * @param state  The enable/disable state.
     * @return Returns @c true when success in enable/disable.
     */
    virtual bool enable3PWakewordDetection(const std::string& wakeword, bool state) = 0;

    /**
     * Helper funtion to get the enablement status of the 3P wakeword.
     * 
     * @param wakeword  The wakeword whoes status is being set.
     * @return Returns @c true if wakeword detection is enabled, else returns @c false.     
     */
    virtual bool is3PWakewordEnabled(const std::string& wakeword) = 0;
};

}  // namespace wakeword
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_WAKEWORD_WAKEWORD_MANAGER_DELEGATE_INTERFACE_H
