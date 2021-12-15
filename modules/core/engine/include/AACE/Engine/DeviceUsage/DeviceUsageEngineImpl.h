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

#ifndef AACE_ENGINE_DEVICEUSAGE_DEVICEUSAGEENGINEIMPL_H_
#define AACE_ENGINE_DEVICEUSAGE_DEVICEUSAGEENGINEIMPL_H_

#include <memory>

#include "AACE/DeviceUsage/DeviceUsage.h"
#include "AACE/DeviceUsage/DeviceUsageEngineInterfaces.h"

namespace aace {
namespace engine {
namespace deviceUsage {

class DeviceUsageEngineImpl
        : public aace::deviceUsage::DeviceUsageEngineInterface
        , public std::enable_shared_from_this<DeviceUsageEngineImpl> {
private:
    /**
     * Constructor.
     */
    DeviceUsageEngineImpl(std::shared_ptr<aace::deviceUsage::DeviceUsage> deviceUsagePlatformInterface);

public:
    /**
     * Factory method for creating instance of @c DeviceUsageEngineImpl
     */
    static std::shared_ptr<DeviceUsageEngineImpl> create(
        std::shared_ptr<aace::deviceUsage::DeviceUsage> deviceUsagePlatformInterface);

    virtual ~DeviceUsageEngineImpl() = default;

    /// @name DeviceUsageEngineInterface Functions
    /// @{
    void onReportNetworkDataUsage(const std::string& usage) override;
    /// @}

    void doShutdown();

private:
    /// Auto SDK DeviceUsage platform interface handler instance.
    std::shared_ptr<aace::deviceUsage::DeviceUsage> m_deviceUsagePlatformInterface;
};

}  // namespace deviceUsage
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_DEVICEUSAGE_DEVICEUSAGEENGINEIMPL_H_
