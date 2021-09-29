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

#ifndef AACE_JNI_DEVICEUSAGE_DEVICEUSAGEBINDER_H_
#define AACE_JNI_DEVICEUSAGE_DEVICEUSAGEBINDER_H_

#include <AACE/DeviceUsage/DeviceUsage.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace deviceUsage {

class DeviceUsageHandler : public aace::deviceUsage::DeviceUsage {
public:
    DeviceUsageHandler(jobject obj);

private:
    JObject m_obj;
};

class DeviceUsageBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    DeviceUsageBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_deviceUsageHandler;
    }

    std::shared_ptr<DeviceUsageHandler> getDeviceUsage() {
        return m_deviceUsageHandler;
    }

private:
    std::shared_ptr<DeviceUsageHandler> m_deviceUsageHandler;
};

}  // namespace deviceUsage
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_DEVICEUSAGE_DEVICEUSAGEBINDER_H_
