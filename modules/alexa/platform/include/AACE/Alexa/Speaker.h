/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c Speaker class is the base class for platform interfaces that have volume control.
 */
class Speaker {
protected:
    Speaker() = default;

public:
    /**
     * This enum provides the type of the @c Speaker class.
     */
    enum class Type {
        /**
         * Speaker source that should be synced with AVS.
         */
        AVS_SYNCED,
        /**
         * Speaker source that will not be synced with AVS.
         */
        LOCAL
    };

    virtual ~Speaker() = default;

    /**
     * Called when the platform implementation should change the volume.
     *
     * @param [in] volume The absolute volume level scaled from 0 (min) to 100 (max).
     * @return @c true if the call was handled successfully.
     */
    virtual bool setVolume( int8_t volume ) = 0;

    /**
     * Called when the platform implementation should adjust the volume to
     * a relative level.
     *
     * @param [in] delta The relative volume adjustment. A positive or
     * negative value used to increase or decrease volume in relation
     * to the current volume setting.
     * @return @c true if the call was handled successfully.
     */
    virtual bool adjustVolume( int8_t delta ) = 0;

    /**
     * Called when the platform implementation should mute/unmute.
     *
     * @param [in] mute @c true when the media player is muted, and @c false when unmuted.
     * @return @c true if the call was handled successfully.
     */
    virtual bool setMute( bool mute ) = 0;

    /**
     * Called when the Engine needs the platform's current volume.
     *
     */
    virtual int8_t getVolume() =  0;

    /**
     * Called when the Engine needs the platform's muting state.
     *
     */
    virtual bool isMuted() = 0;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_SPEAKER_H
