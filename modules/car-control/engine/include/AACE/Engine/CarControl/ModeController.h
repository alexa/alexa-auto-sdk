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

#ifndef AACE_ENGINE_CAR_CONTROL_MODECONTROLLER_H
#define AACE_ENGINE_CAR_CONTROL_MODECONTROLLER_H

#include <AVSCommon/SDKInterfaces/ModeController/ModeControllerInterface.h>
#include <Endpoints/EndpointBuilder.h>

#include <nlohmann/json.hpp>

#include "AACE/Engine/CarControl/PrimitiveController.h"

namespace aace {
namespace engine {
namespace carControl {

/**
 * ModeController capability controller
 */
class ModeController
        : public PrimitiveController
        , public alexaClientSDK::avsCommon::sdkInterfaces::modeController::ModeControllerInterface
        , public std::enable_shared_from_this<ModeController> {
public:
    /// Aliases to improve readability
    /// @{
    using AlexaResponseType = alexaClientSDK::avsCommon::avs::AlexaResponseType;
    using AlexaStateChangeCauseType = alexaClientSDK::avsCommon::sdkInterfaces::AlexaStateChangeCauseType;
    using ModeControllerAttributes = alexaClientSDK::avsCommon::sdkInterfaces::modeController::ModeControllerAttributes;
    using ModeControllerObserverInterface =
        alexaClientSDK::avsCommon::sdkInterfaces::modeController::ModeControllerObserverInterface;
    using ModeState = alexaClientSDK::avsCommon::sdkInterfaces::modeController::ModeControllerInterface::ModeState;
    using json = nlohmann::json;
    /// @}

    /**
     * Create a @c ModeController object
     *
     * @return A pointer to a new @c ModeController if arguments are valid, otherwise @c nullptr
     */
    static std::shared_ptr<ModeController> create(
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

    /// @name ModeControllerInterface methods
    /// @{
    ModeControllerConfiguration getConfiguration() override;
    std::pair<AlexaResponseType, std::string> setMode(const std::string& mode, const AlexaStateChangeCauseType cause)
        override;
    std::pair<AlexaResponseType, std::string> adjustMode(int delta, const AlexaStateChangeCauseType cause) override;
    std::pair<AlexaResponseType, alexaClientSDK::avsCommon::utils::Optional<ModeState>> getMode() override;
    bool addObserver(std::shared_ptr<ModeControllerObserverInterface> observer) override;
    void removeObserver(const std::shared_ptr<ModeControllerObserverInterface>& observer) override;
    /// @}

private:
    /**
     * ModeController constructor
     */
    ModeController(
        const std::string& endpointId,
        const std::string& interface,
        const std::string& instance,
        ModeControllerAttributes attributes);

    /// The attributes of this ModeController
    ModeControllerAttributes m_attributes;

    /// The list of modes supported by this controller
    std::vector<std::string> m_supportedModes;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_MODECONTROLLER_H
