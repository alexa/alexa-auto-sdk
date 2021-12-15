/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * DeviceSetup platform interface should be extended and registered into the Alexa Engine.
 * This class is used to send an event to AVS SDK when client application setup is completed.
 * Alexa walks through the supported Alexa out-of-the-box experience for the first-time user when
 * setupCompleted event is triggered.
 *
 * @deprecated This platform interface is deprecated.
 *             Use the Alexa Auto Services Bridge (AASB) message broker
 *             to publish and subscribe to AASB messages instead.
 *             @see aace::core::MessageBroker
 */
abstract public class DeviceSetup extends PlatformInterface {
    public DeviceSetup() {}

    /**
     * StatusCode indicates the status of the setupCompletedResponse event call.
     */
    public enum StatusCode {
        /**
         * Event {@link #setupCompleted() setupCompleted()} is sent to AVS successfully
         * @hideinitializer
         */
        SUCCESS("SUCCESS"),

        /**
         * SDK failed to send the event setupCompleted.
         * @hideinitializer
         */
        FAIL("FAIL");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private StatusCode(String name) {
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
     * Override and implement this callback method.
     * This method indicates the success status of the {@link #setupCompleted() setupCompleted()} call
     * @param statusCode indicates status code {@link StatusCode#SUCCESS} or {@link StatusCode#FAIL}
     */
    public void setupCompletedResponse(StatusCode statusCode) {}

    /**
     * When client application setup is completed, setupCompleted() function should be called.
     * Alexa walks through the supported Alexa out of box experience to the first time user.
     * <p>Note:</p>
     * <p>1. This method should be called only once after completion of OOBE. Subsequent calls may be ignored by the
     * cloud.</p> <p>2. Do not call this method if user is in Connectivity Mode, Preview Mode, or if Alexa wake word is
     * disabled. Calling the API in such conditions results in undesired user experience.
     * </p>
     */
    final public void setupCompleted() {
        setupCompleted(getNativeRef());
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
    private native void setupCompleted(long nativeRef);
}
