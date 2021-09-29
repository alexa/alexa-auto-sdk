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

#include "AACE/DeviceUsage/DeviceUsage.h"

namespace aace {
namespace deviceUsage {

DeviceUsage::~DeviceUsage() = default;  // key function

void DeviceUsage::reportNetworkDataUsage(const std::string& usage) {
    if (auto m_deviceUsageEngineInterface_lock = m_deviceUsageEngineInterface.lock()) {
        m_deviceUsageEngineInterface_lock->onReportNetworkDataUsage(usage);
    }
}

void DeviceUsage::setEngineInterface(std::shared_ptr<DeviceUsageEngineInterface> deviceUsageEngineInterface) {
    m_deviceUsageEngineInterface = deviceUsageEngineInterface;
}

}  // namespace deviceUsage
}  // namespace aace
