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

import android.os.SystemClock;

import com.amazon.aace.core.PlatformInterface;

/**
 * @c MediaPlaybackRequestor platform interface should be extended and registered into the Alexa Engine.
 * This class is used to send an event to AVS when client is expected to automatically resume the last playing media.
 * Alexa can use this event to push the last playing media on the device.
 */
abstract public class MediaPlaybackRequestor extends PlatformInterface {
    public MediaPlaybackRequestor() {}

    /**
     * This enumeration represents the reason for calling the @c requestMediaPlayback API
     */
    public enum InvocationReason {
        /**
         * Initiating the media resume by the system
         * @hideinitializer
         */
        AUTOMOTIVE_STARTUP("AUTOMOTIVE_STARTUP"),

        /**
         * Initiating the media resume by the driver through physical button. This reason is not accepted yet.
         * @internal
         * @hideinitializer
         */
        EXPLICIT_USER_ACTION("EXPLICIT_USER_ACTION");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private InvocationReason(String name) {
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
     * @c MediaPlaybackRequestStatus indicates the status of the @c requestMediaPlayback event call.
     */
    public enum MediaPlaybackRequestStatus {
        /**
         * Event call is successful
         * @hideinitializer
         */
        SUCCESS("SUCCESS"),

        /**
         * Event call is failed because Alexa is not connected, please retry.
         * @hideinitializer
         */
        FAILED_CAN_RETRY("FAILED_CAN_RETRY"),

        /**
         * Event call is failed because of the threshold timeout.
         * @hideinitializer
         */
        FAILED_TIMEOUT("FAILED_TIMEOUT"),

        /**
         * Event call is failed because of an error.
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
        private MediaPlaybackRequestStatus(String name) {
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
     * This method indicates the success status of the @c requestMediaPlayback call
     * @param MediaPlaybackRequestStatus indicates status code @c MediaPlaybackRequestStatus#SUCCESS} or @c
     *         MediaPlaybackRequestStatus#FAILED_TIMEOUT
     */
    public void mediaPlaybackResponse(MediaPlaybackRequestStatus mediaPlaybackRequestStatus) {}

    /**
     * Platform interface is expected to call this method whenever Alexa is the right source for the media resume.
     * Please refer the overloaded method also. If your platform does not provide right value
     * of elapsed boot time for some platform specific technical reason, use the overloaded method and
     * provide the correct value.
     */
    final public void requestMediaPlayback(InvocationReason invocationReason) {
        requestMediaPlayback(invocationReason, SystemClock.elapsedRealtime());
    }

    /**
     * Platform interface is expected to call this method instead of other overloaded method whenever Alexa is the right
     * source for the media resume, Use the function only when OEM platform's boot elapsed time calculation is
     * special and it does not provide correct results for- Android:
     * https://developer.android.com/reference/android/os/SystemClock#elapsedRealtime() or Linux: sys/sysinfo.h uptime
     * Note: There can be multiple reasons like special power mode handling, unsupported platform etc. OEM needs to
     * ensure right value is provided. Media auto resume is supported only within the certain duration to avoid sudden
     * surprise music playback to driver while driving.
     */
    final public void requestMediaPlayback(InvocationReason invocationReason, long elapsedBootTime) {
        requestMediaPlayback(getNativeRef(), invocationReason, elapsedBootTime);
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
    private native void requestMediaPlayback(long nativeRef, InvocationReason invocationReason, long elapsedBootTime);
}
