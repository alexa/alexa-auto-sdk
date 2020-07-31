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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * Alerts should be extended to handle alerts (e.g. timers, alarms, reminders) from AVS.
 * The platform implementation is responsible for rendering visual cues for an active alert.
 */
public class Alerts extends PlatformInterface {
    /**
     * Specifies the state of an alert
     */
    public enum AlertState {
        /**
         * The alert is ready to activate and is waiting for channel focus.
         * @hideinitializer
         */
        READY("READY"),
        /**
         * The alert is activated, and rendering is perceivable by the user.
         * @hideinitializer
         */
        STARTED("STARTED"),
        /**
         * The alert has stopped due to user or system intervention.
         * @hideinitializer
         */
        STOPPED("STOPPED"),
        /**
         * The alert is active but has been snoozed.
         * @hideinitializer
         */
        SNOOZED("SNOOZED"),
        /**
         * The alert has completed on its own, without user interaction.
         * @hideinitializer
         */
        COMPLETED("COMPLETED"),
        /**
         * The alert has expired and will not be rendered.
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
        ERROR("ERROR"),
        /**
         * The alert has been deleted.
         * @hideinitializer
         */
        DELETED("DELETED"),
        /**
         * The alert has been scheduled to trigger at a future time.
         * @hideinitializer
         */
        SCHEDULED_FOR_LATER("SCHEDULED_FOR_LATER");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AlertState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Notifies the platform implementation of an alert state change
     *
     * @param  alertToken The opaque token that uniquely identifies the alert
     *
     * @param  state The new alert state
     *
     * @param  reason The reason for the state change
     */
    public void alertStateChanged(String alertToken, AlertState state, String reason) {}

    /**
     * Notifies the platform implementation of an alert created, with detailed alert info.
     *
     * @param alertToken The AVS token of the alert.
     * @param detailedInfo The alert info payload :
     * {
     *      "time" : <String>
     *      "type" : <String>
     *      "label" : <String>
     * }
     * time The time string ( Scheduled Time ISO_8601 ).
     * type The type of the alert ( ALERT, REMINDER, TIMER ).
     * label The label of the TIMER, description for REMINDER, or empty string for ALARM.
     */
    public void alertCreated(String alertToken, String detailedInfo) {}

    /**
     * Notifies the platform implementation of an alert deleted, with alert token.
     *
     * @param alertToken The AVS token of the alert.
     */
    public void alertDeleted(String alertToken) {}

    /**
     * Notifies the Engine of a platform request to stop any active alert, such as when a user presses a physical 'stop'
     * button.
     */
    final public void localStop() {
        localStop(getNativeRef());
    }

    /**
     * Notifies the Engine of a platform request to clear the user's
     * pending alerts from storage. This may be useful for a scenario in which a user's pending alerts should not go
     * off after he logs out of the application. This does not clear the user's pending alerts from the Alexa cloud
     * since it does not support a local 'Remove all alerts' feature.
     */
    final public void removeAllAlerts() {
        removeAllAlerts(getNativeRef());
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void localStop(long nativeObject);
    private native void removeAllAlerts(long nativeObject);
}
