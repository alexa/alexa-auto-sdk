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
 * Notifications should be extended to handle rendering indicators for notifications from AVS.
 * The platform implementation is responsible for rendering visual cues for an active notification.
 *
 * @note This interface provides indicators that a notification was received, not the content of the notification.
 */
public class Notifications extends PlatformInterface {
    /**
     * Specifies whether a notification indicator should be rendered on the platform
     */
    public enum IndicatorState {
        /**
         * The notification indicator should be turned off
         * @hideinitializer
         */
        OFF("OFF"),
        /**
         * The notification indicator should be turned on
         * @hideinitializer
         */
        ON("ON"),
        /**
         * The notification indicator state is unknown.
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
        private IndicatorState(String name) {
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
     * Notifies the platform implementation of whether a notification indicator should be rendered
     *
     * @param  state The new notification indicator state
     */
    public void setIndicator(IndicatorState state) {}

    /**
     * Notifies the platform implementation of notification received
     */
    public void onNotificationReceived() {}

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
}
