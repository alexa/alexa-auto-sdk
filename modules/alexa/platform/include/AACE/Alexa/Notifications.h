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

#ifndef AACE_ALEXA_NOTIFICATIONS_H
#define AACE_ALEXA_NOTIFICATIONS_H

#include <AACE/Core/PlatformInterface.h>
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * Notifications should be extended to handle rendering indicators for notifications from AVS.
 *
 * The platform implementation is responsible for rendering visual cues for an active notification.
 * The Notifications @c MediaPlayer will receive directives from the Engine
 * for rendering audio indicators required by a change in @c Notifications::IndicatorState.
 *
 * @note This interface provides indicators that a notification was received, not the content of the notification.
 *
 * @sa AudioChannel
 */
class Notifications : public aace::core::PlatformInterface {
protected:
    Notifications() = default;

public:
    virtual ~Notifications();

    /**
     * Specifies whether a notification indicator should be rendered on the platform
     */
    enum class IndicatorState {

        /**
       * The notification indicator should be turned off
       */
        OFF = 0,

        /**
       * The notification indicator should be turned on
       */
        ON = 1,

        /**
       * The notification indicator state is unknown.
       */
        UNKNOWN
    };

    /**
     * Notifies the platform implementation of whether a notification indicator should be rendered
     *
     * @param [in] state The new notification indicator state
     */
    virtual void setIndicator(IndicatorState state) = 0;

    /**
     * Notifies the platform implementation of notification received
     */
    virtual void onNotificationReceived(){};
};

inline std::ostream& operator<<(std::ostream& stream, const Notifications::IndicatorState& state) {
    switch (state) {
        case Notifications::IndicatorState::OFF:
            stream << "OFF";
            break;
        case Notifications::IndicatorState::ON:
            stream << "ON";
            break;
        case Notifications::IndicatorState::UNKNOWN:
            stream << "UNKNOWN";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_NOTIFICATIONS_H
