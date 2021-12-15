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

#ifndef AACE_CAR_CONTROL_CAR_CONTROL_BINDER_H
#define AACE_CAR_CONTROL_CAR_CONTROL_BINDER_H

#include <AACE/CarControl/CarControl.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace carControl {

//
// CarControlBinder
//

class CarControlHandler : public aace::carControl::CarControl {
public:
    CarControlHandler(jobject obj);

    /**
         * PowerController
         */
    bool turnPowerControllerOn(const std::string& endpointId) override;
    bool turnPowerControllerOff(const std::string& endpointId) override;
    bool isPowerControllerOn(const std::string& endpointId, bool& isOn) override;

    /**
         * ToggleController
         */
    bool turnToggleControllerOn(const std::string& endpointId, const std::string& controllerId) override;
    bool turnToggleControllerOff(const std::string& endpointId, const std::string& controllerId) override;
    bool isToggleControllerOn(const std::string& endpointId, const std::string& controllerId, bool& isOn) override;

    /**
         * RangeController
         */
    bool setRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double value) override;
    bool adjustRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double delta)
        override;
    bool getRangeControllerValue(const std::string& endpointId, const std::string& controllerId, double& value)
        override;

    /**
         * ModeController
         */
    bool setModeControllerValue(
        const std::string& endpointId,
        const std::string& controllerId,
        const std::string& value) override;
    bool adjustModeControllerValue(const std::string& endpointId, const std::string& controllerId, int delta) override;
    bool getModeControllerValue(const std::string& endpointId, const std::string& controllerId, std::string& value)
        override;

private:
    JObject m_obj;
};

//
// CarControlBinder
//

class CarControlBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    CarControlBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_carControlHandler;
    }

private:
    std::shared_ptr<CarControlHandler> m_carControlHandler;
};

}  // namespace carControl
}  // namespace jni
}  // namespace aace

#endif  // AACE_CAR_CONTROL_CAR_CONTROL_BINDER_H