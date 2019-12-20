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

#ifndef AACE_ENGINE_CAR_CONTROL_ENDPOINT_H
#define AACE_ENGINE_CAR_CONTROL_ENDPOINT_H

#include <nlohmann/json.hpp>

#include "AACE/CarControl/CarControl.h"
#include "AACE/Engine/Alexa/EndpointBuilderFactory.h"
#include "AACE/Engine/CarControl/AssetStore.h"
#include "AACE/Engine/CarControl/CapabilityController.h"

using json = nlohmann::json;

namespace aace {
namespace engine {
namespace carControl {

/**
 * Represents a unique endpoint of the vehicle with its associated capabilities.
 */
class Endpoint {
public:
    /**
     * Create an @c Endpoint object.
     *
     * @return A pointer to a new @c Endpoint if arguments are valid, otherwise @c nullptr.
     */
    static std::shared_ptr<Endpoint> create(const json& endpointConfig, AssetStore& assetStore);
    /**
     * Endpoint destructor
     */
    ~Endpoint();

    std::string getId();

    /**
     * Constructs the AVS SDK representation of the endpoint for registration with the endpoint registration manager,
     * which includes the following:
     *  @li Creating the internal representation of this endpoint with an @c EndpointBuilder
     *  @li Performing the translation of 'EndpointResources' to use multiple endpoints for multiple friendly names
     *  @li Creating the internal representation of the capabilities of this endpoint by calling
     *  CapabilityController::build()
     *
     * @return @c true if successful
     */
    bool build(
        std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
        std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> endpointBuilderFactory,
        const std::string& manufacturer = "");

    /**
     * Adds a controller with the specified ID to this endpoint
     */
    bool addController(const std::string& id, std::shared_ptr<CapabilityController> controller);

private:
    Endpoint(const std::string endpointId, const std::vector<std::string>& names);
    std::string m_endpointId;
    /// A list of all asset IDs used to identify this endpoint's friendly names
    std::vector<std::string> m_assetIds;
    /// A list of all friendly names used to identify this endpoint
    std::vector<std::string> m_names;
    /// A map of controllers belonging to this endpoint keyed by controller id
    std::unordered_map<std::string, std::shared_ptr<CapabilityController>> m_controllers;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_ENDPOINT_H
