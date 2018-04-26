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

#ifndef AACE_ALEXA_ALERTS_H
#define AACE_ALEXA_ALERTS_H

#include "AudioChannel.h"
#include "AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

/**
 * The @c Alerts class should be extended by the platform implementation to handle alerts from AVS.
 */
class Alerts : public AudioChannel {
protected:
    Alerts( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

public:
    virtual ~Alerts() = default;

    /**
     * An enum class which captures the states an alert object can be in.
     */
    enum class AlertState {
        /**
         * The alert is ready to start, and is waiting for channel focus.
         */
        READY,
        /**
         * The alert has started.
         */
        STARTED,
        /**
         * The alert has stopped due to user or system intervention.
         */
        STOPPED,
        /**
         * The alert has been snoozed.
         */
        SNOOZED,
        /**
         * The alert has completed on its own.
         */
        COMPLETED,
        /**
         * The alert has been determined to have expired, and will not be rendered.
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
        ERROR
    };

    /**
     * Called when the platform implementation should handle an alert state change.
     *
     * @param [in] alertToken Opaque token that uniquely identifies the alert.
     * @param [in] state New state of the alert.
     * @param [in] reason Reason for the state change.
     * @sa AlertState
     */
    virtual void alertStateChanged( const std::string& alertToken, AlertState state, const std::string& reason ) = 0;

    /**
     * This function provides a way for application code to request this object stop any active alert as the result
     * of a user action, such as pressing a physical 'stop' button on the device.
     */
    void localStop();
    
    /**
     * A function that allows an application to clear all alerts from storage.  This may be useful for a scenario
     * where a user logs out of a device, and another user will log in.  As the first user logs out, their pending
     * alerts should not go off.
     */
    void removeAllAlerts();

    /**
     * @internal
     * Sets engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<aace::alexa::AlertsEngineInterface> alertsEngineInterface );

private:
    std::shared_ptr<aace::alexa::AlertsEngineInterface> m_alertsEngineInterface;
};

} // aace::alexa
} // aace

#endif // AACE_ALEXA_ALERTS_H
