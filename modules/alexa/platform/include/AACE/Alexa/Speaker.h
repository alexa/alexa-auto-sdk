/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_SPEAKER_H
#define AACE_ALEXA_SPEAKER_H

#include <stdint.h>

#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * Speaker is the interface for volume and mute control for an @c AudioChannel.
 *
 * Volume and mute settings for the Speaker are independent of each other, 
 * and the respective directives from the Engine should not affect the other setting in any way.
 *
 * @sa AudioChannel
 */  
class Speaker {
protected:
    Speaker() = default;

public:

    /**
     * Specifies the type of the Speaker
     */
    enum class Type {

        /**
         * The Speaker type that is controlled by AVS
         */
        AVS_SPEAKER_VOLUME,

        /**
         * The Speaker type that is controlled locally by the platform
         */
        AVS_ALERTS_VOLUME
    };

    virtual ~Speaker() = default;

    /**
     * Notifies the platform implementation to set the absolute volume of the Speaker. The
     * @c volume value should be scaled to fit the needs of the platform.
     *
     * @param [in] volume The absolute volume to set on the Speaker. @c volume
     * is in the range [0,100]. 
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool setVolume( int8_t volume ) = 0;

    /**
     * Notifies the platform implementation to make a relative adjustment to the volume setting of the Speaker. 
     * The @c delta value is relative to the current volume setting and is positive to
     * increase volume or negative to reduce volume.
     * The volume @c delta value should be scaled to fit the needs of the platform.
     *
     * @param [in] delta The volume adjustment to apply to the Speaker. @c delta is
     * in the range [-100, 100].
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool adjustVolume( int8_t delta ) = 0;

    /**
     * Notifies the platform implementation to apply a mute setting to the Speaker
     *
     * @param [in] mute The mute setting to apply to the Speaker. @c true when the Speaker
     * should be muted, @c false when unmuted
     * @return @c true if the platform implementation successfully handled the call, 
     * else @c false
     */
    virtual bool setMute( bool mute ) = 0;

    /**
     * Returns the current volume setting of the Speaker platform implementation
     *
     * @return The current volume setting of the Speaker platform implementation. The volume returned
     * must be scaled to the range [0,100].
     */
    virtual int8_t getVolume() = 0;

    /**
     * Returns the current mute setting of the Speaker platform implementation
     *
     * @return The current mute setting of the Speaker platform implementation.
     * @c true when the Speaker is muted, else @c false
     */
    virtual bool isMuted() = 0;
    
    /**
     * Notifies the Engine of a volume change event
     * originating on the platform, such as a user pressing a "volume up" or "volume down"
     * button. If the Speaker is @c Type::AVS_SPEAKER_VOLUME, the Engine will respond with a
     * call to @c setVolume() on each AVS-synced Speaker.
     *
     * @param [in] volume The new volume setting of the Speaker. The @c volume reported
     * must be scaled to the range [0,100].
     *
     * @sa Type  
     */
    void localVolumeSet( int8_t volume );
    
    /**
     * Notifies the Engine of a mute setting change event
     * originating on the platform, such as a user pressing a "mute" button.
     * If the Speaker is @c Type::AVS_SPEAKER_VOLUME, the Engine will respond with a
     * call to @c setMute() on each AVS-synced Speaker.
     *
     * @param [in] mute The new mute setting of the Speaker. @c true when the Speaker is muted,
     * else @c false
     *
     * @sa Type  
     */
    void localMuteSet( bool mute );

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::SpeakerEngineInterface> speakerEngineInterface );

private:
    std::shared_ptr<aace::alexa::SpeakerEngineInterface> m_speakerEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const Speaker::Type& type) {
    switch (type) {
        case Speaker::Type::AVS_SPEAKER_VOLUME:
            stream << "AVS_SPEAKER_VOLUME";
            break;
        case Speaker::Type::AVS_ALERTS_VOLUME:
            stream << "AVS_ALERTS_VOLUME";
            break;
    }
    return stream;
}

} // aace::alexa
} // aace

#endif // AACE_ALEXA_SPEAKER_H
