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

#ifndef AACE_ALEXA_ALEXA_SPEAKER_H
#define AACE_ALEXA_ALEXA_SPEAKER_H

#include <stdint.h>

#include "AACE/Core/PlatformInterface.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * AlexaSpeaker is the interface for setting the Alexa volume and mute controls.
 */
class AlexaSpeaker : public aace::core::PlatformInterface {
protected:
    AlexaSpeaker() = default;

public:
    virtual ~AlexaSpeaker();

    using SpeakerType = aace::alexa::AlexaSpeakerEngineInterface::SpeakerType;

    /**
     * Notifies the platform implementation that the speaker settings have changed for
     * a specific speaker type.
     * 
     * @note Calling @c AlexaSpeaker::localSetVolume,
     * @c AlexaSpeaker::localAdjustVolume, or @c AlexaSpeaker::localSetMute from inside this
     * function will cause deadlock.
     * 
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] local @c true if the change originated from calling @c AlexaSpeaker::localSetVolume,
     * @c AlexaSpeaker::localAdjustVolume, or @c AlexaSpeaker::localSetMute.
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     * will be scaled to the range [0,100].
     * @param [in] mute The mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false.
     */
    virtual void speakerSettingsChanged(SpeakerType type, bool local, int8_t volume, bool mute);

    /**
     * Notifies the Engine of a volume change event
     * originating on the platform, such as a user pressing a "volume up" or "volume down"
     * button. If the Speaker is @c Type::ALEXA_VOLUME, the Engine will respond with a
     * call to @c setVolume() on each AVS-synced Speaker.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     * must be scaled to the range [0,100].
     */
    void localSetVolume(SpeakerType type, int8_t volume);

    /**
     * Notifies the Engine of a relative adjustment to the volume setting of the Speaker, 
     * originating on the platform.
     * The @c delta value is relative to the current volume setting and is positive to
     * increase volume or negative to reduce volume.
     * The volume @c delta value should be scaled to fit the needs of the platform.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] delta The volume adjustment to apply to the Speaker. @c delta is
     * in the range [-100, 100].
     */
    void localAdjustVolume(SpeakerType type, int8_t delta);

    /**
     * Notifies the Engine of a mute setting change event
     * originating on the platform, such as a user pressing a "mute" button.
     * If the Speaker is @c Type::ALEXA_VOLUME, the Engine will respond with a
     * call to @c setMute() on each AVS-synced Speaker.
     *
     * @param [in] type The type of Alexa speaker being set.
     * @param [in] mute The new mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false
     */
    void localSetMute(SpeakerType type, bool mute);

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::alexa::AlexaSpeakerEngineInterface> alexaSpeakerEngineInterface);

private:
    std::weak_ptr<aace::alexa::AlexaSpeakerEngineInterface> m_alexaSpeakerEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_ALEXA_SPEAKER_H
