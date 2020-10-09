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

#include <AASB/Engine/Alexa/AASBEqualizerController.h>
#include <AACE/Engine/Core/EngineMacros.h>
#include <AACE/Alexa/EqualizerController.h>

#include <AASB/Message/Alexa/EqualizerController/LocalSetBandLevelsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/LocalAdjustBandLevelsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/LocalResetBandsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/SetBandLevelsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/GetBandLevelsMessage.h>
#include <AASB/Message/Alexa/EqualizerController/GetBandLevelsMessageReply.h>

namespace aasb {
namespace engine {
namespace alexa {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.alexa.AASBEqualizerController");

// aliases
using Message = aace::engine::aasb::Message;

std::shared_ptr<AASBEqualizerController> AASBEqualizerController::create(
    std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        ThrowIfNull(messageBroker, "invalidMessageBrokerInterface");

        // create the equalizer controller platform handler
        auto equalizerController = std::shared_ptr<AASBEqualizerController>(new AASBEqualizerController());

        // initialize the equalizer controller
        ThrowIfNot(equalizerController->initialize(messageBroker), "initializeFailed");

        return equalizerController;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

bool AASBEqualizerController::initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker) {
    try {
        m_messageBroker = messageBroker;

        std::weak_ptr<AASBEqualizerController> wp = shared_from_this();

        messageBroker->subscribe(
            aasb::message::alexa::equalizerController::LocalSetBandLevelsMessage::topic(),
            aasb::message::alexa::equalizerController::LocalSetBandLevelsMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::alexa::equalizerController::LocalSetBandLevelsMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    // convert the band levels from aasb to aace types
                    std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> bandLevels;
                    for (auto next : payload.bandLevels) {
                        bandLevels.push_back(
                            {static_cast<aace::alexa::EqualizerController::EqualizerBand>(next.band), next.level});
                    }

                    sp->localSetBandLevels(bandLevels);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalSetBandLevelsMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::equalizerController::LocalAdjustBandLevelsMessage::topic(),
            aasb::message::alexa::equalizerController::LocalAdjustBandLevelsMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::alexa::equalizerController::LocalAdjustBandLevelsMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    // convert the band levels from aasb to aace types
                    std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> bandLevels;
                    for (auto next : payload.bandAdjustments) {
                        bandLevels.push_back(
                            {static_cast<aace::alexa::EqualizerController::EqualizerBand>(next.band), next.level});
                    }

                    sp->localAdjustBandLevels(bandLevels);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalAdjustBandLevelsMessage").d("reason", ex.what()));
                }
            });

        messageBroker->subscribe(
            aasb::message::alexa::equalizerController::LocalResetBandsMessage::topic(),
            aasb::message::alexa::equalizerController::LocalResetBandsMessage::action(),
            [wp](const Message& message) {
                try {
                    auto sp = wp.lock();
                    ThrowIfNull(sp, "invalidWeakPtrReference");

                    aasb::message::alexa::equalizerController::LocalResetBandsMessage::Payload payload =
                        nlohmann::json::parse(message.payload());

                    // convert the bands from aasb to aace types
                    std::vector<aace::alexa::EqualizerController::EqualizerBand> bands;
                    for (auto band : payload.bands) {
                        bands.push_back(static_cast<aace::alexa::EqualizerController::EqualizerBand>(band));
                    }

                    sp->localResetBands(bands);
                } catch (std::exception& ex) {
                    AACE_ERROR(LX(TAG, "LocalResetBandsMessage").d("reason", ex.what()));
                }
            });

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

//
// aace::alexa::EqualizerController
//
void AASBEqualizerController::setBandLevels(
    const std::vector<aace::alexa::EqualizerController::EqualizerBandLevel>& bandLevels) {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        // create the SetBandLevelsMessage
        aasb::message::alexa::equalizerController::SetBandLevelsMessage message;

        // convert the band levels from aace to aasb
        for (auto next : bandLevels) {
            message.payload.bandLevels.push_back(
                {static_cast<aasb::message::alexa::EqualizerBand>(next.first), next.second});
        }

        m_messageBroker_lock->publish(message.toString()).send();
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
    }
}

std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> AASBEqualizerController::getBandLevels() {
    try {
        AACE_VERBOSE(LX(TAG));

        auto m_messageBroker_lock = m_messageBroker.lock();
        ThrowIfNull(m_messageBroker_lock, "invalidMessageBrokerReference");

        aasb::message::alexa::equalizerController::GetBandLevelsMessage message;

        auto result = m_messageBroker_lock->publish(message.toString()).get();

        ThrowIfNot(result.valid(), "waitForBandLevelTimeout");

        aasb::message::alexa::equalizerController::GetBandLevelsMessageReply::Payload payload =
            nlohmann::json::parse(result.payload());
        std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> bandLevels;

        // Need to check name of variable in GetBandLevelsMessageReply.h
        for (auto next : payload.bandLevels) {
            bandLevels.push_back({static_cast<aace::alexa::EqualizerController::EqualizerBand>(next.band), next.level});
        }

        return bandLevels;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return {};
    }
}

}  // namespace alexa
}  // namespace engine
}  // namespace aasb
