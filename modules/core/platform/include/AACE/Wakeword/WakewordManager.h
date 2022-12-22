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

#ifndef AACE_WAKEWORD_WAKEWORD_MANAGER_H
#define AACE_WAKEWORD_WAKEWORD_MANAGER_H

#include <iostream>
#include <string>

#include <AACE/Core/PlatformInterface.h>
#include <AACE/Wakeword/WakewordManagerEngineInterface.h>

namespace aace {
namespace wakeword {

/**
 *  
 */
class WakewordManager : public aace::core::PlatformInterface {
protected:
    WakewordManager() = default;

public:
    using WakeWordIndex = aace::wakeword::WakewordManagerEngineInterface::WakeWordIndex;

    /**
     * Notifies the platform implementation about the wakeword detected event.
     * 
     * @param wakeword Indicates the wakeword detected.
     * @param beginIndex beginIndex of Wakeword detected.
     * @param endIndex endIndex of Wakeword detected.
     */
    virtual void onWakewordDetected(const std::string& wakeword, WakeWordIndex beginIndex, WakeWordIndex endIndex) = 0;

    /**
     * Notifies the Engine of the set wakeword event from the platform implementation.
     * 
     * @param wakeword  Name of the wakeword to be enabled
     * @param wakewordState. true for enable, false for disable
     */
    bool enable3PWakeword(const std::string& wakeword, bool wakewordState);

    /**
     * Destructor
     */
    virtual ~WakewordManager();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<WakewordManagerEngineInterface> wakewordManagerEngineInterface);

private:
    std::weak_ptr<WakewordManagerEngineInterface> m_wakewordManagerEngineInterface;
};

}  // namespace wakeword
}  // namespace aace

#endif  // AACE_WAKEWORD_WAKEWORD_MANAGER_H
