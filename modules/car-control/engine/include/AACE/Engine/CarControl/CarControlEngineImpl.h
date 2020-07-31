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

#ifndef AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_ENGINE_IMPL_H
#define AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_ENGINE_IMPL_H

#include <AVSCommon/Utils/RequiresShutdown.h>

#include <memory>
#include <utility>

#include "AACE/CarControl/CarControl.h"
#include "AACE/Engine/CarControl/CarControlServiceInterface.h"

namespace aace {
namespace engine {
namespace carControl {

class CarControlEngineImpl
        : public CarControlServiceInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown {
public:
    static std::shared_ptr<CarControlEngineImpl> create(
        std::shared_ptr<aace::carControl::CarControl> platformInterface);

    CarControlEngineImpl(std::shared_ptr<aace::carControl::CarControl> platformInterface);

    /// @name @c CarControlServiceInterface methods
    /// @{
    bool turnPowerControllerOn(const std::string& endpointId) override;
    bool turnPowerControllerOff(const std::string& endpointId) override;
    bool isPowerControllerOn(const std::string& endpointId, bool& isOn) override;

    bool turnToggleControllerOn(const std::string& endpointId, const std::string& instance) override;
    bool turnToggleControllerOff(const std::string& endpointId, const std::string& instance) override;
    bool isToggleControllerOn(const std::string& endpointId, const std::string& instance, bool& isOn) override;

    bool setRangeControllerValue(const std::string& endpointId, const std::string& instance, double value) override;
    bool adjustRangeControllerValue(const std::string& endpointId, const std::string& instance, double delta) override;
    bool getRangeControllerValue(const std::string& endpointId, const std::string& instance, double& value) override;

    bool setModeControllerValue(const std::string& endpointId, const std::string& instance, const std::string& value)
        override;
    bool adjustModeControllerValue(const std::string& endpointId, const std::string& instance, int delta) override;
    bool getModeControllerValue(const std::string& endpointId, const std::string& instance, std::string& value)
        override;
    /// @}

protected:
    void doShutdown() override;

private:
    std::shared_ptr<aace::carControl::CarControl> m_platformInterface;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_ENGINE_IMPL_H
