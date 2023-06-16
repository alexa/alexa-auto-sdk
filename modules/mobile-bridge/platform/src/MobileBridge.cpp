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

#include "AACE/MobileBridge/MobileBridge.h"

namespace aace {
namespace mobileBridge {

MobileBridge::~MobileBridge() = default;

void MobileBridge::setEngineInterface(std::shared_ptr<MobileBridgeEngineInterface> mobileBridgeEngineInterface) {
    m_mobileBridgeEngineInterface = mobileBridgeEngineInterface;
}

bool MobileBridge::start(int tunFd) {
    if (auto engine = m_mobileBridgeEngineInterface.lock()) {
        return engine->onStart(tunFd);
    }
    return false;
}

bool MobileBridge::stop() {
    if (auto engine = m_mobileBridgeEngineInterface.lock()) {
        return engine->onStop();
    }
    return false;
}

void MobileBridge::authorizeDevice(const std::string& deviceToken, bool authorized) {
    if (auto engine = m_mobileBridgeEngineInterface.lock()) {
        return engine->onDeviceAuthorized(deviceToken, authorized);
    }
}

void MobileBridge::sendInfo(const std::string& deviceToken, uint32_t infoId, const std::string& info) {
    if (auto engine = m_mobileBridgeEngineInterface.lock()) {
        return engine->onInfoSent(deviceToken, infoId, info);
    }
}

}  // namespace mobileBridge
}  // namespace aace
