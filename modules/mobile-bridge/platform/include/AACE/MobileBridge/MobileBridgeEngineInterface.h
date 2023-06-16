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

#ifndef AACE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_INTERFACE_H
#define AACE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_INTERFACE_H

#include <iostream>
#include <string>

#include "AACE/Core/PlatformInterface.h"

/** @file */

namespace aace {
namespace mobileBridge {

/**
 * The class to interface with the Mobile Bridge Engine.
 */
class MobileBridgeEngineInterface {
public:
    virtual ~MobileBridgeEngineInterface() = default;

    virtual bool onStart(int tunFd) = 0;
    virtual bool onStop() = 0;
    virtual void onDeviceAuthorized(const std::string& deviceToken, bool authorized) = 0;
    virtual void onInfoSent(const std::string& deviceToken, uint32_t infoId, const std::string& info) = 0;
};

}  // namespace mobileBridge
}  // namespace aace

#endif  // AACE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_INTERFACE_H
