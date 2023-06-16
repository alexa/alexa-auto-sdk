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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_IMPL_H
#define AACE_ENGINE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_IMPL_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "AACE/Engine/MobileBridge/Config.h"
#include "AACE/Engine/MobileBridge/TransportLoop.h"
#include "AACE/Engine/Utils/Threading/Executor.h"
#include "AACE/MobileBridge/MobileBridge.h"
#include "AACE/MobileBridge/MobileBridgeEngineInterface.h"
#include "AVSCommon/Utils/DeviceInfo.h"

namespace aace {
namespace engine {
namespace mobileBridge {

class MobileBridgeEngineImpl
        : public aace::mobileBridge::MobileBridgeEngineInterface
        , public std::enable_shared_from_this<MobileBridgeEngineImpl> {
public:
    static std::shared_ptr<MobileBridgeEngineImpl> create(
        std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo = nullptr,
        std::shared_ptr<Config> config = nullptr);

    explicit MobileBridgeEngineImpl(
        std::shared_ptr<aace::mobileBridge::MobileBridge> mobileBridge,
        std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo,
        std::shared_ptr<Config> config);
    ~MobileBridgeEngineImpl() override;

    void setRetryTable(const std::vector<int>& retryTable);

    // MobileBridgeEngineInterface
    bool onStart(int tunFd = -1) override;
    bool onStop() override;
    void onDeviceAuthorized(const std::string& deviceToken, bool authorized) override;
    void onInfoSent(const std::string& deviceToken, uint32_t infoId, const std::string& info) override;

    bool shutdown();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_MOBILE_BRIDGE_ENGINE_IMPL_H
