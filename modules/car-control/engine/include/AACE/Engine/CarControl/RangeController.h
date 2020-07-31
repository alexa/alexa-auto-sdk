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

#ifndef AACE_ENGINE_CAR_CONTROL_RANGECONTROLLER_H
#define AACE_ENGINE_CAR_CONTROL_RANGECONTROLLER_H

#include <AVSCommon/SDKInterfaces/RangeController/RangeControllerInterface.h>
#include <Endpoints/EndpointBuilder.h>

#include <nlohmann/json.hpp>

#include "AACE/Engine/CarControl/PrimitiveController.h"

namespace aace {
namespace engine {
namespace carControl {

/**
 * RangeController capability controller.
 */
class RangeController
        : public PrimitiveController
        , public alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerInterface
        , public std::enable_shared_from_this<RangeController> {
public:
    /// Aliases to improve readability
    /// @{
    using AlexaResponseType = alexaClientSDK::avsCommon::avs::AlexaResponseType;
    using AlexaStateChangeCauseType = alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType;
    using RangeControllerAttributes =
        alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerAttributes;
    using RangeControllerObserverInterface =
        alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerObserverInterface;
    using RangeState = alexaClientSDK::avsCommon::sdkInterfaces::rangeController::RangeControllerInterface::RangeState;
    using json = nlohmann::json;
    /// @}

    /**
     * Create a @c RangeController object
     *
     * @return A pointer to a new @c RangeController if arguments are valid, otherwise @c nullptr
     */
    static std::shared_ptr<RangeController> create(
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

    /// @name RangeControllerInterface methods
    /// @{
    RangeControllerConfiguration getConfiguration() override;
    std::pair<AlexaResponseType, std::string> setRangeValue(double rangeValue, const AlexaStateChangeCauseType cause)
        override;
    std::pair<AlexaResponseType, std::string> adjustRangeValue(double rangeDelta, const AlexaStateChangeCauseType cause)
        override;
    std::pair<AlexaResponseType, alexaClientSDK::avsCommon::utils::Optional<RangeState>> getRangeState() override;
    bool addObserver(std::shared_ptr<RangeControllerObserverInterface> observer) override;
    void removeObserver(const std::shared_ptr<RangeControllerObserverInterface>& observer) override;
    /// @}

private:
    /**
     * RangeController constructor.
     */
    RangeController(
        const std::string& endpointId,
        const std::string& interface,
        const std::string& instance,
        RangeControllerAttributes attributes,
        double minimum,
        double maximum,
        double precision);

    /// The attributes of this RangeController
    RangeControllerAttributes m_attributes;

    /// The minimum of the allowed range for this controller
    double m_minimum;
    /// The maximum of the allowed range for this controller
    double m_maximum;
    /// The precision of range increments allowed for this controller
    double m_precision;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_RANGECONTROLLER_H
