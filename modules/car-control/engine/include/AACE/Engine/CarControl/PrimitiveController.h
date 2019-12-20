/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CAR_CONTROL_PRIMITIVE_CONTROLLER_H
#define AACE_ENGINE_CAR_CONTROL_PRIMITIVE_CONTROLLER_H

#include <vector>

#include <AACE/Engine/CarControl/CapabilityController.h>

namespace aace {
namespace engine {
namespace carControl {

/**
 * Base class for a primitive capability controller, (e.g. ModeController, RangeController, ToggleController)
 */
class PrimitiveController : public CapabilityController {
public:
    /**
     * PrimitiveController constructor.
     */
    PrimitiveController(const std::string& endpointId, const std::string& interface, const std::string& instance);
    /**
     * PrimitiveController destructor.
     */
    ~PrimitiveController();

    /// @c CapabilityController methods
    /// @{
    std::string getId() override;
    /// @}
    /*
     * Get the name of this capability controller instance
     */
    std::string getInstance();

private:
    /// The name of this capability controller instance
    std::string m_instance;
    /// A list of all asset IDs used for the friendly names that describe this controller
    std::vector<std::string> m_assetIds;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_PRIMITIVE_CONTROLLER_H
