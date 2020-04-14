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

#ifndef AACE_APL_APL_ENGINE_INTERFACE_H
#define AACE_APL_APL_ENGINE_INTERFACE_H

#include <chrono>
#include <string>

namespace aace {
namespace apl {

class APLEngineInterface {
public:
    enum class ActivityEvent {
        // GUI switched to active state.
        ACTIVATED,

        // GUI become inactive.
        DEACTIVATED,

        // GUI processed one-time event (touch/scroll/etc).
        ONE_TIME,

        /// Interrupt event (touch)
        INTERRUPT,

        // Guard option for unknown received state.
        UNKNOWN
    };
    virtual void onClearCard() = 0;
    virtual void onClearAllExecuteCommands() = 0;
    virtual void onSendUserEvent(const std::string& payload) = 0;
    virtual void onSetAPLMaxVersion(const std::string& aplMaxVersion) = 0;
    virtual void onSetDocumentIdleTimeout(std::chrono::milliseconds documentIdleTimeout) = 0;
    virtual void onRenderDocumentResult(const std::string& token, bool result, const std::string& error) = 0;
    virtual void onExecuteCommandsResult(const std::string& token, bool result, const std::string& error) = 0;
    virtual void onProcessActivityEvent(const std::string& source, ActivityEvent event) = 0;
};

}  // namespace apl
}  // namespace aace

#endif