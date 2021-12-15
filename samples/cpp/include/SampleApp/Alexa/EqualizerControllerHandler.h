/*
 * Copyright 2018-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ALEXA_EQUALIZERCONTROLLERHANDLER_H
#define SAMPLEAPP_ALEXA_EQUALIZERCONTROLLERHANDLER_H

#include "SampleApp/Activity.h"
#include "SampleApp/Logger/LoggerHandler.h"

#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Alexa/EqualizerController/EqualizerBand.h>
#include <AASB/Message/Alexa/EqualizerController/EqualizerBandLevel.h>

// JSON for Modern C++
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sampleApp {
namespace alexa {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  EqualizerControllerHandler
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class EqualizerControllerHandler {
private:
    std::weak_ptr<Activity> m_activity;
    std::weak_ptr<logger::LoggerHandler> m_loggerHandler;
    std::shared_ptr<aace::core::MessageBroker> m_messageBroker;

protected:
    EqualizerControllerHandler(
        std::weak_ptr<Activity> activity,
        std::weak_ptr<logger::LoggerHandler> loggerHandler,
        std::shared_ptr<aace::core::MessageBroker> messageBroker);

public:
    template <typename... Args>
    static auto create(Args&&... args) -> std::shared_ptr<EqualizerControllerHandler> {
        return std::shared_ptr<EqualizerControllerHandler>(new EqualizerControllerHandler(args...));
    }
    auto getActivity() -> std::weak_ptr<Activity>;
    auto getLoggerHandler() -> std::weak_ptr<logger::LoggerHandler>;

private:
    /**
     * Handles the GetBandLevels messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleGetBandLevelsMessage(const std::string& message);

    /**
     * Handles the SetBandLevels messages received from the Engine.
     *
     * @param [in] message The message received from the Engine
     */
    void handleSetBandLevelsMessage(const std::string& message);

    /**
     * Notifies the Engine that gain levels for one or more equalizer bands are being set directly on the device. If
     * unsupported levels are provided, the Engine will truncate the settings to the configured range.
     *
     * @param [in] bandLevels The equalizer bands to change and their gain settings as integer dB values.
     */
    void localSetBandLevels(
        const std::vector<aasb::message::alexa::equalizerController::EqualizerBandLevel>& bandLevels);

    /**
     * Notifies the Engine that relative adjustments to equalizer band gain levels are being made directly on the
     * device. If adjustments put the band level settings beyond the configured dB range, the Engine will truncate the
     * settings to the configured range.
     *
     * @param [in] bandAdjustments The equalizer bands to adjust and their relative gain adjustments as integer dB
     *             values.
     */
    void localAdjustBandLevels(
        const std::vector<aasb::message::alexa::equalizerController::EqualizerBandLevel>& bandAdjustments);

    /**
     * Notifies the Engine that the gain levels for the equalizer bands are being reset to their defaults.
     *
     * @param [in] bands The equalizer bands to reset. Empty @a bands resets all supported equalizer bands.
     */
    void localResetBands(const std::vector<aasb::message::alexa::equalizerController::EqualizerBand>& bands);

    /**
     * Retrieves the current equalizer gain settings on the device for each supported band. If unsupported band levels
     * are provided, the Engine will truncate levels to the configured range.
     *
     * @return The supported equalizer bands and their current gain settings as integer dB values.
     */
    std::vector<aasb::message::alexa::equalizerController::EqualizerBandLevel> getBandLevels();

    /**
     * Applies the provided gain settings to the corresponding equalizer bands.
     *
     * @note Calling @c EqualizerController::localSetBandLevels,
     * @c EqualizerController::localAdjustBandLevels, or @c EqualizerController::localResetBands from inside this
     * function will cause deadlock.
     *
     * @param [in] bandLevels The equalizer bands and their gain settings to apply as integer dB values.
     */
    void setBandLevels(const std::vector<aasb::message::alexa::equalizerController::EqualizerBandLevel>& bandLevels);

private:
    std::weak_ptr<View> m_console{};

    auto log(logger::LoggerHandler::Level level, const std::string& message) -> void;
    auto setupUI() -> void;
    auto subscribeToAASBMessages() -> void;
    auto toString(aasb::message::alexa::equalizerController::EqualizerBand enumValue) -> std::string;
};

}  // namespace alexa
}  // namespace sampleApp

#endif  // SAMPLEAPP_ALEXA_EQUALIZERCONTROLLERHANDLER_H
