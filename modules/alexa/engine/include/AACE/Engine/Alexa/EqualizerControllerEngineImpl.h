/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_ALEXA_EQUALIZER_CONTROLLER_ENGINE_IMPL_H
#define AACE_ENGINE_ALEXA_EQUALIZER_CONTROLLER_ENGINE_IMPL_H

#include <string>

#include <acsdkEqualizer/EqualizerCapabilityAgent.h>
#include <acsdkEqualizerImplementations/EqualizerController.h>
#include <acsdkEqualizerInterfaces/EqualizerConfigurationInterface.h>
#include <acsdkEqualizerInterfaces/EqualizerInterface.h>
#include <acsdkEqualizerInterfaces/EqualizerStorageInterface.h>
#include <AVSCommon/SDKInterfaces/CapabilitiesDelegateInterface.h>
#include <AVSCommon/SDKInterfaces/ContextManagerInterface.h>
#include <AVSCommon/SDKInterfaces/Endpoints/EndpointCapabilitiesRegistrarInterface.h>
#include <AVSCommon/SDKInterfaces/ExceptionEncounteredSenderInterface.h>
#include <AVSCommon/SDKInterfaces/MessageSenderInterface.h>
#include <AVSCommon/Utils/Error/SuccessResult.h>
#include <RegistrationManager/CustomerDataManagerInterface.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"
#include "AACE/Alexa/EqualizerController.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

/// EqualizerController Engine implementation
class EqualizerControllerEngineImpl
        : public aace::alexa::EqualizerControllerEngineInterface
        , public alexaClientSDK::acsdkEqualizerInterfaces::EqualizerInterface
        , public alexaClientSDK::acsdkEqualizerInterfaces::EqualizerStorageInterface
        , public alexaClientSDK::avsCommon::utils::RequiresShutdown
        , public std::enable_shared_from_this<EqualizerControllerEngineImpl> {
public:
    /**
     * Factory method to create an EqualizerControllerEngineImpl instance
     *
     * @param equalizerPlatformInterface The associated EqualizerController platform interface instance
     * @param capabilitiesRegistrar The EndpointCapabilitiesRegistrarInterface for the default endpoint
     * @param capabilitiesDelegate Interface to control DCF configuration
     * @param customerDataManager Component to register the capability agent as a customer data container
     * @param exceptionEncounteredSender Interface to report exceptions to AVS
     * @param contextManager Interface to provide equalizer state to AVS
     * @param messageSender Interface to send events to AVS
     *
     * @return A new instance of @c EqualizerControllerEngineImpl on success, @c nullptr otherwise
     */
    static std::shared_ptr<EqualizerControllerEngineImpl> create(
        std::shared_ptr<aace::alexa::EqualizerController> equalizerPlatformInterface,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> customerDataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>
            exceptionEncounteredSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    // EqualizerInterface functions
    virtual void setEqualizerBandLevels(
        alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap bandLevels) override;
    virtual int getMinimumBandLevel() override;
    virtual int getMaximumBandLevel() override;

    // EqualizerControllerEngineInterface functions
    void onLocalSetBandLevels(const std::vector<EqualizerBandLevel>& bandLevels) override;
    void onLocalAdjustBandLevels(const std::vector<EqualizerBandLevel>& bandAdjustments) override;
    void onLocalResetBands(const std::vector<EqualizerBand>& bands) override;

    // EqualizerStorageInterface functions
    virtual void saveState(const alexaClientSDK::acsdkEqualizerInterfaces::EqualizerState& state) override;
    virtual alexaClientSDK::avsCommon::utils::error::SuccessResult<
        alexaClientSDK::acsdkEqualizerInterfaces::EqualizerState>
    loadState() override;
    virtual void clear() override;

protected:
    // RequiresShutdown functions
    virtual void doShutdown() override;

private:
    /**
     * EqualizerControllerEngineImpl constructor
     *
     * @param equalizerPlatformInterface The associated @c EqualizerController platform interface instance
     */
    EqualizerControllerEngineImpl(std::shared_ptr<aace::alexa::EqualizerController> equalizerPlatformInterface);

    /**
     * Initialize the @c EqualizerControllerEngineImpl instance
     *
     * @param capabilitiesRegistrar The EndpointCapabilitiesRegistrarInterface for the default endpoint
     * @param capabilitiesDelegate Interface to control DCF configuration
     * @param customerDataManager Component to register the capability agent as a customer data container
     * @param exceptionEncounteredSender Interface to report exceptions to AVS
     * @param contextManager Interface to provide equalizer state to AVS
     * @param messageSender Interface to send events to AVS
     *
     * @return @c true on successful initialization, @c false otherwise
     */
    bool initialize(
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::endpoints::EndpointCapabilitiesRegistrarInterface>
            capabilitiesRegistrar,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::CapabilitiesDelegateInterface> capabilitiesDelegate,
        std::shared_ptr<alexaClientSDK::registrationManager::CustomerDataManagerInterface> customerDataManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ExceptionEncounteredSenderInterface>
            exceptionEncounteredSender,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::ContextManagerInterface> contextManager,
        std::shared_ptr<alexaClientSDK::avsCommon::sdkInterfaces::MessageSenderInterface> messageSender);

    /**
     * Truncate the band level setting for @a bandLevel to the configured min/max range
     *
     * @return The truncated level
     */
    int truncateBandLevel(const EqualizerBandLevel& bandLevel);

    /**
     * Convert @c std::vector<EqualizerBandLevel> to
     * @c alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap and truncate values that fall out of
     * the configured min/max range
     *
     * @param [in] bandLevels The band level vector to convert
     * @return The converted band level map
     */
    inline alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap convertAndTruncateBandLevels(
        const std::vector<EqualizerBandLevel>& bandLevels) {
        alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap newMap;
        for (const auto& bandLevel : bandLevels) {
            auto band = convertBand(bandLevel.first);
            int level = truncateBandLevel(bandLevel);
            newMap[band] = level;
        }
        return newMap;
    }

    /**
     * Convert @c std::vector<EqualizerBandLevel> to
     * @c alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap
     *
     * @param [in] bandLevels The band level vector to convert
     * @return The converted band level map
     */
    inline alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap convertBandLevels(
        const std::vector<EqualizerBandLevel>& bandLevels) {
        alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap newMap;
        for (const auto& bandLevel : bandLevels) {
            newMap[convertBand(bandLevel.first)] = bandLevel.second;
        }
        return newMap;
    }

    /**
     * Convert @c alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap to
     * @c std::vector<EqualizerBandLevel>
     *
     * @return The converted band level vector
     */
    inline std::vector<EqualizerBandLevel> convertBandLevels(
        const alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBandLevelMap& bandLevels) {
        std::vector<EqualizerBandLevel> newBandLevels;
        for (const auto& bandLevel : bandLevels) {
            EqualizerControllerEngineInterface::EqualizerBand band = convertBand(bandLevel.first);
            newBandLevels.push_back({band, bandLevel.second});
        }
        return newBandLevels;
    }

    /**
     * Convert @c alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand to
     * @c EqualizerControllerEngineInterface::EqualizerBand
     *
     * @param [in] band The band to convert
     * @return The converted band
     */
    inline EqualizerControllerEngineInterface::EqualizerBand convertBand(
        const alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand& band) {
        switch (band) {
            case alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand::BASS:
                return EqualizerControllerEngineInterface::EqualizerBand::BASS;
            case alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand::MIDRANGE:
                return EqualizerControllerEngineInterface::EqualizerBand::MIDRANGE;
            case alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand::TREBLE:
            default:
                return EqualizerControllerEngineInterface::EqualizerBand::TREBLE;
        }
    }

    /**
     * Convert @c EqualizerControllerEngineInterface::EqualizerBand to
     * @c alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand
     *
     * @param [in] band The band to convert
     * @return The converted band
     */
    inline alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand convertBand(
        const EqualizerControllerEngineInterface::EqualizerBand& band) {
        switch (band) {
            case EqualizerControllerEngineInterface::EqualizerBand::BASS:
                return alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand::BASS;
            case EqualizerControllerEngineInterface::EqualizerBand::MIDRANGE:
                return alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand::MIDRANGE;
            case EqualizerControllerEngineInterface::EqualizerBand::TREBLE:
            default:
                return alexaClientSDK::acsdkEqualizerInterfaces::EqualizerBand::TREBLE;
        }
    }

    /**
     * Provides a string representation for an @c std::vector<EqualizerBandLevel>
     *
     * @param bandLevels The band level vector
     * @return A string representation of the band levels
     */
    inline std::string bandLevelsToString(const std::vector<EqualizerBandLevel>& bandLevels) {
        std::string stateString;
        for (const auto& bandLevel : bandLevels) {
            EqualizerControllerEngineInterface::EqualizerBand band = bandLevel.first;
            int level = bandLevel.second;
            stateString += equalizerBandToString(band) + ":" + std::to_string(level) + " ";
        }
        return stateString;
    }

    /// The associated @c EqualizerController platform interface instance
    std::shared_ptr<aace::alexa::EqualizerController> m_equalizerPlatformInterface;

    /// The equalizer controller capability agent
    std::shared_ptr<alexaClientSDK::acsdkEqualizer::EqualizerCapabilityAgent> m_equalizerCapabilityAgent;

    /// The @c EqualizerController component used for all equalizer state manipulations and notifications
    std::shared_ptr<alexaClientSDK::acsdkEqualizer::EqualizerController> m_equalizerController;

    /// The component for providing equalizer capabilities and configuration settings
    std::shared_ptr<alexaClientSDK::acsdkEqualizerInterfaces::EqualizerConfigurationInterface> m_configuration;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ALEXA_EQUALIZER_CONTROLLER_ENGINE_IMPL_H
