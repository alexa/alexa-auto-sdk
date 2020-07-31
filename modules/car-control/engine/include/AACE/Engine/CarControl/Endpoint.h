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
     * Create an @c Endpoint object from the specified JSON configuration.
     * The provided JSON is expected to follow the format of a single endpoint
     * entry of the "endpoints" array of "aace.carControl" configuration, e.g.
     * @code
     * {
     * "endpointId": "driver.fan",
     * "endpointResources": {
     *      "friendlyNames": [
     *          {
     *              "@type": "asset",
     *              "value": {
     *                  "assetId": "Alexa.Automotive.DeviceName.Fan"
     *              }
     *          }
     *      ]
     * },
     * "capabilities": [
     *      {
     *          "type": "AlexaInterface",
     *          "interface": "Alexa.PowerController",
     *          "version": "3",
     *          "properties": {
     *              "supported": [
     *                  {
     *                      "name": "powerState"
     *                  }
     *              ],
     *              "proactivelyReported": false,
     *              "retrievable": false
     *          }
     *      }
     * ],
     * "relationships": {
     *      "isMemberOf": {
     *          "zoneID": "zone.all"
     *      }
     * }
     * }
     * @endcode
     *
     * @param endpointConfig The entry of the "endpoints" array of "aace.carControl" config used to construct
     *        this endpoint.
     * @param assetStore The @c AssetStore storing the asset friendly name literals used in the configuration
     * @return A pointer to a new @c Endpoint if arguments are valid, otherwise @c nullptr.
     */
    static std::shared_ptr<Endpoint> create(const json& endpointConfig, const AssetStore& assetStore);

    /**
     * Endpoint destructor
     */
    ~Endpoint();

    /**
     * Get the configured endpoint ID for this endpoint.
     */
    std::string getId();

    /**
     * Get the endpoint ID sent in discovery for this endpoint.
     * @note This ID is not necessarily different than the configured ID, but it may also be an ID derived from client 
     *       ID, product ID, DSN, and configured endpoint ID.
     * @note This ID is not initialized until @c build().
     */
    std::string getDiscoveryId();

    /**
     * Constructs the AVS SDK representation of the endpoint for registration with the endpoint registration manager,
     * which includes the following:
     *  @li Creating the internal representation of this endpoint with an @c EndpointBuilder
     *  @li Performing the translation of assetIds from 'EndpointResources' to the literal text representations
     *  @li Creating the internal representation of the capabilities of this endpoint by calling
     *  CapabilityController::build()
     *
     * @return @c true if successful
     */
    bool build(
        std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
        std::shared_ptr<aace::engine::alexa::EndpointBuilderFactory> endpointBuilderFactory,
        const AssetStore& assetStore,
        const std::string& manufacturer = "",
        const std::string& description = "");

    /**
     * Adds a controller with the specified ID to this endpoint
     */
    bool addController(const std::string& id, std::shared_ptr<CapabilityController> controller);

private:
    /**
     * Endpoint constructor
     *
     * @param endpointId The configured ID for this endpoint
     * @param assetIds A complete list of asset IDs used to identify this endpoint
     */
    Endpoint(const std::string endpointId, const std::vector<std::string>& assetIds);

    /// The configured endpoint ID for this endpoint
    std::string m_endpointId;

    /// The endpoint ID sent in discovery for this endpoint.
    std::string m_discoveryEndpointId;

    /// A list of all asset IDs used to identify this endpoint's friendly names
    std::vector<std::string> m_assetIds;

    /// A map of controllers belonging to this endpoint keyed by controller id
    std::unordered_map<std::string, std::shared_ptr<CapabilityController>> m_controllers;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_ENDPOINT_H
