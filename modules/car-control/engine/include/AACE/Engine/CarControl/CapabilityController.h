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

#ifndef AACE_ENGINE_CAR_CONTROL_CAPABILITY_CONTROLLER_H
#define AACE_ENGINE_CAR_CONTROL_CAPABILITY_CONTROLLER_H

#include <Endpoints/EndpointBuilder.h>

#include "AACE/Engine/CarControl/AssetStore.h"
#include "AACE/Engine/CarControl/CarControlEngineImpl.h"

namespace aace {
namespace engine {
namespace carControl {

/**
 * Base class for a capability controller
 */
class CapabilityController {
public:
    /// Alias to improve readability
    /// @{
    using EndpointBuilder = alexaClientSDK::endpoints::EndpointBuilder;
    /// @}

    /**
     * CapabilityController constructor.
     */
    CapabilityController(const std::string& interface, const std::string& endpointId);
    /**
     * CapabilityController destructor.
     */
    ~CapabilityController();

    /**
     * Get the ID of the endpoint controlled by this controller
     */
    std::string getEndpointId();
    /**
     * Get the ID of this controller
     */
    virtual std::string getId();
    /**
     * Get the interface type name this controller
     */
    std::string getInterface();

    /**
     * Create the internal representation of the endpoint using the @c EndpointBuilder
     */
    virtual void build(
        std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
        std::unique_ptr<EndpointBuilder>& builder) = 0;

private:
    /// The ID of the endpoint controlled by this controller
    std::string m_endpointId;
    /// The interface type of this controller
    std::string m_interface;

protected:
    /// The car control service interface reference, used to access the platform interface
    std::shared_ptr<CarControlServiceInterface> m_carControlServiceInterface;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_CAPABILITY_CONTROLLER_INTERFACE_H
