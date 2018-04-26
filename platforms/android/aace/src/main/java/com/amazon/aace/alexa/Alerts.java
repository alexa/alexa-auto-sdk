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

// aace/alexa/Alerts.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

/**
 * The @c Alerts class should be extended by the platform implementation to handle alerts from AVS.
 */
public class Alerts extends AudioChannel
{
    /**
     * An enum class which captures the states an alert object can be in.
     */
    public enum AlertState
    {
        /**
         * The alert is ready to start, and is waiting for channel focus.
         * @hideinitializer
         */
        READY("READY"),
        /**
         * The alert has started.
         * @hideinitializer
         */
        STARTED("STARTED"),
        /**
         * The alert has stopped due to user or system intervention.
         * @hideinitializer
         */
        STOPPED("STOPPED"),
        /**
         * The alert has been snoozed.
         * @hideinitializer
         */
        SNOOZED("SNOOZED"),
        /**
         * The alert has completed on its own.
         * @hideinitializer
         */
        COMPLETED("COMPLETED"),
        /**
         * The alert has been determined to have expired, and will not be rendered.
         * @hideinitializer
         */
        PAST_DUE("PAST_DUE"),
        /**
         * The alert has entered the foreground.
         * @hideinitializer
         */
        FOCUS_ENTERED_FOREGROUND("FOCUS_ENTERED_FOREGROUND"),
        /**
         * The alert has entered the background.
         * @hideinitializer
         */
        FOCUS_ENTERED_BACKGROUND("FOCUS_ENTERED_BACKGROUND"),
        /**
         * The alert has encountered an error.
         * @hideinitializer
         */
        ERROR("ERROR");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AlertState( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public Alerts( MediaPlayer mediaPlayer, Speaker speaker ) {
        super( mediaPlayer, speaker , null );
    }

    /**
     * Called when the platform implementation should handle an alert state change.
     *
     * @param [in] alertToken Opaque token that uniquely identifies the alert.
     * @param [in] state New state of the alert.
     * @param [in] reason Reason for the state change.
     * @sa AlertState
     */
    public void alertStateChanged( String alertToken, AlertState state, String reason ) {
    }

}

// END OF FILE
