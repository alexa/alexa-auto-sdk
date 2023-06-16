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

#ifndef AACE_JNI_MOBILE_BRIDGE_MOBILE_BRIDGE_BINDER_H
#define AACE_JNI_MOBILE_BRIDGE_MOBILE_BRIDGE_BINDER_H

#include "AACE/JNI/Core/PlatformInterfaceBinder.h"
#include "AACE/MobileBridge/MobileBridge.h"

namespace aace {
namespace jni {
namespace mobileBridge {

class MobileBridgeHandler : public aace::mobileBridge::MobileBridge {
public:
    MobileBridgeHandler(jobject obj);
    ~MobileBridgeHandler();

    /// @name @c aace::mobileBridge::MobileBridge methods
    /// @{
    std::vector<std::shared_ptr<aace::mobileBridge::Transport>> getTransports() override;
    std::shared_ptr<Connection> connect(const std::string& transportId) override;
    void disconnect(const std::string& transportId) override;
    void onActiveTransportChange(const std::string& transportId, const std::string& transportState) override;
    void onDeviceHandshaked(
        const std::string& transportId,
        const std::string& deviceToken,
        const std::string& friendlyName) override;
    void onInfo(const std::string& deviceToken, uint32_t messageId, const std::string& message) override;
    bool protectSocket(int socket) override;
    /// @}

private:
    JObject m_obj;
};

class MobileBridgeBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    MobileBridgeBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_mobileBridgeHandler;
    }

    std::shared_ptr<aace::mobileBridge::MobileBridge> getMobileBridge() {
        return m_mobileBridgeHandler;
    }

private:
    std::shared_ptr<MobileBridgeHandler> m_mobileBridgeHandler;
};

}  // namespace mobileBridge
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_MOBILE_BRIDGE_MOBILE_BRIDGE_BINDER_H
