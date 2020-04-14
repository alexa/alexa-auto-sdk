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

#ifndef AACE_ENGINE_CAR_CONTROL_POWERCONTROLLER_H
#define AACE_ENGINE_CAR_CONTROL_POWERCONTROLLER_H

#include <AVSCommon/SDKInterfaces/PowerController/PowerControllerInterface.h>
#include <Endpoints/EndpointBuilder.h>

#include "AACE/Engine/CarControl/CapabilityController.h"

namespace aace {
namespace engine {
namespace carControl {

/**
 * PowerController capability controller.
 */
class PowerController
        : public CapabilityController
        , public alexaClientSDK::avsCommon::sdkInterfaces::powerController::PowerControllerInterface
        , public std::enable_shared_from_this<PowerController> {
public:
    /// Aliases to improve readability
    /// @{
    using AlexaResponseType = alexaClientSDK::avsCommon::avs::AlexaResponseType;
    using AlexaStateChangeCauseType = alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType;
    using PowerControllerObserverInterface =
        alexaClientSDK::avsCommon::sdkInterfaces::powerController::PowerControllerObserverInterface;
    using PowerState = alexaClientSDK::avsCommon::sdkInterfaces::powerController::PowerControllerInterface::PowerState;
    /// @}

    /**
     * Create a @c PowerController object
     *
     * @return A pointer to a new @c PowerController if arguments are valid, otherwise @c nullptr
     */
    static std::shared_ptr<PowerController> create(const std::string& endpointId, const std::string& interface);

    /// @c CapabilityController methods
    /// @{
    void build(
        std::shared_ptr<CarControlServiceInterface> carControlServiceInterface,
        std::unique_ptr<EndpointBuilder>& builder) override;
    /// @}

    /// @name PowerControllerInterface methods
    /// @{
    std::pair<alexaClientSDK::avsCommon::avs::AlexaResponseType, std::string> setPowerState(
        const bool state,
        const AlexaStateChangeCauseType cause) override;
    std::pair<AlexaResponseType, alexaClientSDK::avsCommon::utils::Optional<PowerState>> getPowerState() override;
    bool addObserver(std::shared_ptr<PowerControllerObserverInterface> observer) override;
    void removeObserver(const std::shared_ptr<PowerControllerObserverInterface>& observer) override;
    /// @}

private:
    /**
     * PowerController constructor.
     */
    PowerController(const std::string& endpointId, const std::string& interface);
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_POWERCONTROLLER_H
