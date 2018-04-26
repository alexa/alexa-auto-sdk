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

// aace/alexa/Notifications.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

/**
 * The @c Notifications class should be extended by the platform implementation to handle Notifications from AVS and provide an indication whether notifications are available.
 */
public class Notifications extends AudioChannel
{
    /**
     *  An enum class which captures the states a notifications object can be in.
     */
    public enum IndicatorState
    {
        /**
         *  The notifications indicator should be turned off
         * @hideinitializer
         */
        OFF("OFF"),
        /**
         *  The notifications indicator should be turned on
         * @hideinitializer
         */
        ON("ON"),
        /**
         *  The notifications indicator state is unknown
         * @hideinitializer
         */
        UNKNOWN("UNKNOWN");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private IndicatorState( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public Notifications( MediaPlayer mediaPlayer, Speaker speaker ) {
        super( mediaPlayer, speaker, null );
    }

    /**
     *  Called when the notification indicator has changed state
     */
    public void setIndicator( IndicatorState state ) {
    }

}

// END OF FILE
