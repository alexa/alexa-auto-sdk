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

#ifndef AACE_JNI_ALEXA_DEVICESETUPBINDER_H_
#define AACE_JNI_ALEXA_DEVICESETUPBINDER_H_

#include <AACE/Alexa/DeviceSetup.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class DeviceSetupHandler : public aace::alexa::DeviceSetup {
public:
    DeviceSetupHandler(jobject obj);

    // aace::alexa::DeviceSetup interface
    void setupCompletedResponse(StatusCode statusCode) override;

private:
    JObject m_obj;
};

class DeviceSetupBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    DeviceSetupBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_deviceSetupHandler;
    }

    std::shared_ptr<DeviceSetupHandler> getDeviceSetup() {
        return m_deviceSetupHandler;
    }

private:
    std::shared_ptr<DeviceSetupHandler> m_deviceSetupHandler;
};

//
// JStatusCode
//
class JDeviceSetupStatusCodeConfig : public EnumConfiguration<DeviceSetupHandler::StatusCode> {
public:
    using T = DeviceSetupHandler::StatusCode;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/DeviceSetup$StatusCode";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::SUCCESS, "SUCCESS"}, {T::FAIL, "FAIL"}};
    }
};

using JStatusCode = JEnum<DeviceSetupHandler::StatusCode, JDeviceSetupStatusCodeConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_DEVICESETUPBINDER_H_
