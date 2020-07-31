/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_EQUALIZER_CONTROLLER_H
#define AACE_ALEXA_EQUALIZER_CONTROLLER_H

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * EqualizerController should be extended to use Alexa to control device equalizer settings. It supports adjusting 
 * levels of amplitude gain in decibels (dB) for a 3-band equalizer with a 1dB level of precision. 
 * 
 * In addition to performing the audio equalization, the platform implementation is also responsible for the following:
 * @li Mapping the provided equalizer bands to frequency ranges.
 * @li Scaling values as absolute levels of amplitude gain in integer dB to the platform's internal range.
 * @li Mapping the provided bands to a different number of platform-specific target bands, if necessary.
 * @li Applying equalization to only selected portions of the audio output. Alexa speech, alarms, etc. may not be 
 *  affected.
 * @li Persisting level settings across restart/power cycle.
 * 
 * Device equalizer settings are configurable. See 
 * @c aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig for details on configuring supported 
 * bands, default state, and supported decibel ranges.
 */
class EqualizerController : public aace::core::PlatformInterface {
protected:
    EqualizerController() = default;

public:
    virtual ~EqualizerController();

    /**
     * Describes the equalizer bands supported by Alexa. The platform implementation may support a subset of these.
     * 
     * @sa aace::alexa::EqualizerControllerEngineInterface::EqualizerBand
     */
    using EqualizerBand = aace::alexa::EqualizerControllerEngineInterface::EqualizerBand;

    /**
     * Describes the level of gain of a particular equalizer band as an integer dB value. This is an
     * @c aace::alexa::EqualizerController::EqualizerBand and @c int pair.
     * 
     * @sa aace::alexa::EqualizerControllerEngineInterface::EqualizerBandLevel
     */
    using EqualizerBandLevel = aace::alexa::EqualizerControllerEngineInterface::EqualizerBandLevel;

    /**
     * Notifies the platform implementation to apply the provided gain settings to the corresponding equalizer bands.
     * 
     * @note Calling @c EqualizerController::localSetBandLevels, 
     * @c EqualizerController::localAdjustBandLevels, or @c EqualizerController::localResetBands from inside this 
     * function will cause deadlock.
     * 
     * @param [in] bandLevels The equalizer bands and their gain settings to apply as integer dB values.
     */
    virtual void setBandLevels(const std::vector<EqualizerBandLevel>& bandLevels) = 0;

    /**
     * Retrieves the current equalizer gain settings on the device for each supported band. If unsupported band levels 
     * are provided, the Engine will truncate levels to the configured range.
     * 
     * @return The supported equalizer bands and their current gain settings as integer dB values.
     */
    virtual std::vector<EqualizerBandLevel> getBandLevels() = 0;

    /**
     * Notifies the Engine that gain levels for one or more equalizer bands are being set directly on the device. If 
     * unsupported levels are provided, the Engine will truncate the settings to the configured range.
     * 
     * @param [in] bandLevels The equalizer bands to change and their gain settings as integer dB values.
     */
    void localSetBandLevels(const std::vector<EqualizerBandLevel>& bandLevels);

    /**
     * Notifies the Engine that relative adjustments to equalizer band gain levels are being made directly on the 
     * device. If adjustments put the band level settings beyond the configured dB range, the Engine will truncate the 
     * settings to the configured range.
     * 
     * @param [in] bandAdjustments The equalizer bands to adjust and their relative gain adjustments as integer dB 
     *             values.
     */
    void localAdjustBandLevels(const std::vector<EqualizerBandLevel>& bandAdjustments);

    /**
     * Notifies the Engine that the gain levels for the equalizer bands are being reset to their defaults.
     * 
     * @param [in] bands The equalizer bands to reset. Empty @a bands resets all supported equalizer bands.
     */
    void localResetBands(const std::vector<EqualizerBand>& bands = {});

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(
        std::shared_ptr<aace::alexa::EqualizerControllerEngineInterface> equalizerControllerEngineInterface);

private:
    /**
     * @internal
     * The associated Engine implementation
     */
    std::weak_ptr<aace::alexa::EqualizerControllerEngineInterface> m_equalizerControllerEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_EQUALIZER_CONTROLLER_H
