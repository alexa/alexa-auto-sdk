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

#ifndef AACE_ENGINE_CBL_CBL_LEGACY_EVENT_NOTIFICATION_INTERFACE_H
#define AACE_ENGINE_CBL_CBL_LEGACY_EVENT_NOTIFICATION_INTERFACE_H

#include <string>
#include <AACE/CBL/CBL.h>

namespace aace {
namespace engine {
namespace cbl {

/**
 * CBLEngineImpl is required to notify the CBL states as in CBL platform interface. This interface
 * is used by CBLAuthorizationProvider to notify those states to CBLEngineImpl.
 */
class CBLLegacyEventNotificationInterface {
public:
    using CBLStateChangedReason = aace::cbl::CBL::CBLStateChangedReason;
    using CBLState = aace::cbl::CBL::CBLState;

    virtual ~CBLLegacyEventNotificationInterface() = default;

    /** 
     * Notifies the legacy CBL states
     * 
     * @note This is used by CBLAuthorizationProvider to notify the CBL states
     * as per the CBL platform interface.
     * 
     * @param state The CBL state
     * @param reason The CBL state change reason
     * @param url The url used to enter the CBL code. This value is set only when the CBLState::CODE_PAIR_RECEIVED
     * @param code The code used to get device authorized. This value is set only when the CBLState::CODE_PAIR_RECEIVED
     */
    virtual void cblStateChanged(
        CBLState state,
        CBLStateChangedReason reason,
        const std::string& url = "",
        const std::string& code = "") = 0;
};

}  // namespace cbl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CBL_CBL_LEGACY_EVENT_NOTIFICATION_INTERFACE_H
