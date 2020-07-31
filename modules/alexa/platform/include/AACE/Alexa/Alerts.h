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

#ifndef AACE_ALEXA_ALERTS_H
#define AACE_ALEXA_ALERTS_H

#include <AACE/Core/PlatformInterface.h>
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * Alerts should be extended to handle alerts (e.g. timers, alarms, reminders) from AVS.
 *
 * The platform implementation is responsible for rendering visual cues for an active alert.
 * The Alerts @c MediaPlayer will receive directives from the Engine to handle alerts audio playback.
 *
 * @sa AudioChannel
 */
class Alerts : public aace::core::PlatformInterface {
protected:
    Alerts() = default;

public:
    virtual ~Alerts();

    /**
     * Specifies the state of an alert
     */
    enum class AlertState {

        /**
         * The alert is ready to activate and is waiting for channel focus.
         */
        READY,

        /**
         * The alert is activated, and rendering is perceivable by the user.
         */
        STARTED,

        /**
         * The alert has stopped due to user or system intervention.
         */
        STOPPED,

        /**
         * The alert is active but has been snoozed.
         */
        SNOOZED,

        /**
         * The alert has completed on its own, without user interaction.
         */
        COMPLETED,

        /**
         * The alert has expired and will not be rendered.
         */
        PAST_DUE,

        /**
         * The alert has entered the foreground.
         */
        FOCUS_ENTERED_FOREGROUND,

        /**
         * The alert has entered the background.
         */
        FOCUS_ENTERED_BACKGROUND,

        /**
         * The alert has encountered an error.
         */
        ERROR,

        /**
        * The alert has been deleted.
        */
        DELETED,

        /**
        * The alert has been scheduled to trigger at a future time.
        */
        SCHEDULED_FOR_LATER
    };

    /**
     * Notifies the platform implementation of an alert state change
     *
     * @param [in] alertToken The opaque token that uniquely identifies the alert
     * @param [in] state The new alert state
     * @param [in] reason The reason for the state change
     */
    virtual void alertStateChanged(const std::string& alertToken, AlertState state, const std::string& reason) = 0;

    /**
     * Notifies the platform implementation of an alert created, with detailed alert info. 
     *
     * @param [in] alertToken The AVS token of the alert.
     * @param [in] detailedInfo The alert info payload :
     * {
     *      "time" : <String>
     *      "type" : <String>
     *      "label" : <String>
     * }
     * time The time string ( Scheduled Time ISO_8601 ).
     * type The type of the alert ( ALERT, REMINDER, TIMER ).
     * label The label of the TIMER, description for REMINDER, or empty string for ALARM.
     */
    virtual void alertCreated(const std::string& alertToken, const std::string& detailedInfo) = 0;

    /**
     * Notifies the platform implementation of an alert deleted, with the alertToken.
     *
     * @param [in] alertToken The AVS token of the alert.
     */
    virtual void alertDeleted(const std::string& alertToken) = 0;

    /**
     * Notifies the Engine of a platform request to stop any active alert, such as when a user presses a physical 'stop' button.
     */
    void localStop();

    /**
     * Notifies the Engine of a platform request to clear the user's
     * pending alerts from local storage. This may be useful for a scenario in which a user's pending alerts should not go 
     * off after he logs out of the application. This does not clear the user's pending alerts from the Alexa cloud since it
     * does not support a local 'Remove all alerts' feature.
     */
    void removeAllAlerts();

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<aace::alexa::AlertsEngineInterface> alertsEngineInterface);

private:
    std::weak_ptr<aace::alexa::AlertsEngineInterface> m_alertsEngineInterface;
};

inline std::ostream& operator<<(std::ostream& stream, const Alerts::AlertState& state) {
    switch (state) {
        case Alerts::AlertState::READY:
            stream << "READY";
            break;
        case Alerts::AlertState::STARTED:
            stream << "STARTED";
            break;
        case Alerts::AlertState::STOPPED:
            stream << "STOPPED";
            break;
        case Alerts::AlertState::SNOOZED:
            stream << "SNOOZED";
            break;
        case Alerts::AlertState::COMPLETED:
            stream << "COMPLETED";
            break;
        case Alerts::AlertState::PAST_DUE:
            stream << "PAST_DUE";
            break;
        case Alerts::AlertState::FOCUS_ENTERED_FOREGROUND:
            stream << "FOCUS_ENTERED_FOREGROUND";
            break;
        case Alerts::AlertState::FOCUS_ENTERED_BACKGROUND:
            stream << "FOCUS_ENTERED_BACKGROUND";
            break;
        case Alerts::AlertState::ERROR:
            stream << "ERROR";
            break;
        case Alerts::AlertState::DELETED:
            stream << "DELETED";
            break;
        case Alerts::AlertState::SCHEDULED_FOR_LATER:
            stream << "SCHEDULED_FOR_LATER";
            break;
    }
    return stream;
}

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_ALERTS_H
