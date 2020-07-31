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

#ifndef AACE_ENGINE_CAR_CONTROL_TOGGLECONTROLLER_H
#define AACE_ENGINE_CAR_CONTROL_TOGGLECONTROLLER_H

#include <AVSCommon/SDKInterfaces/ToggleController/ToggleControllerInterface.h>
#include <Endpoints/EndpointBuilder.h>

#include <nlohmann/json.hpp>

#include "AACE/Engine/CarControl/PrimitiveController.h"

namespace aace {
namespace engine {
namespace carControl {

/**
 * ToggleController capability controller
 */
class ToggleController
        : public PrimitiveController
        , public alexaClientSDK::avsCommon::sdkInterfaces::toggleController::ToggleControllerInterface
        , public std::enable_shared_from_this<ToggleController> {
public:
    /// Aliases to improve readability
    /// @{
    using AlexaResponseType = alexaClientSDK::avsCommon::avs::AlexaResponseType;
    using AlexaStateChangeCauseType = alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType;
    using ToggleControllerAttributes =
        alexaClientSDK::avsCommon::sdkInterfaces::toggleController::ToggleControllerAttributes;
    using ToggleControllerObserverInterface =
        alexaClientSDK::avsCommon::sdkInterfaces::toggleController::ToggleControllerObserverInterface;
    using ToggleState =
        alexaClientSDK::avsCommon::sdkInterfaces::toggleController::ToggleControllerInterface::ToggleState;
    using json = nlohmann::json;
    /// @}

    /**
     * Create a @c ToggleController object
     *
     * @return A pointer to a new @c ToggleController if arguments are valid, otherwise @c nullptr
     */
    static std::shared_ptr<ToggleController> create(
        const json& controllerConfig,
        const std::string& endpointId,
        const std::string& interface,
        const AssetStore& assetStore);

    /// @c CapabilityController methods
    /// @{
    void build(
        std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
        std::unique_ptr<EndpointBuilder>& builder) override;
    /// @}

    /// @name ToggleControllerInterface methods
    /// @{
    std::pair<AlexaResponseType, std::string> setToggleState(const bool state, const AlexaStateChangeCauseType cause)
        override;
    std::pair<AlexaResponseType, alexaClientSDK::avsCommon::utils::Optional<ToggleState>> getToggleState() override;
    bool addObserver(std::shared_ptr<ToggleControllerObserverInterface> observer) override;
    void removeObserver(const std::shared_ptr<ToggleControllerObserverInterface>& observer) override;
    /// @}

private:
    /**
     * ToggleController constructor
     */
    ToggleController(
        const std::string& endpointId,
        const std::string& interface,
        const std::string& instance,
        ToggleControllerAttributes attributes);

    /// The attributes of this ToggleController
    ToggleControllerAttributes m_attributes;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_TOGGLECONTROLLER_H
