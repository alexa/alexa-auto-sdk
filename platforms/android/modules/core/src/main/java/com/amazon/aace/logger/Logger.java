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

package com.amazon.aace.logger;

import com.amazon.aace.core.PlatformInterface;

import java.nio.charset.StandardCharsets;

/**
 * Logger should be extended handle log events from the AAC SDK.
 */
abstract public class Logger extends PlatformInterface {
    /**
     * Specifies the severity level of a log message
     */
    public enum Level {
        /**
         * Verbose log of an event, enabled only for debug builds
         * @hideinitializer
         */
        VERBOSE("VERBOSE"),
        /**
         * Log of a normal event. Used in release builds
         * @hideinitializer
         */
        INFO("INFO"),
        /**
         * Log of a metric, enabled only for builds with metrics enabled
         * @hideinitializer
         */
        METRIC("METRIC"),
        /**
         * Log of an event that may indicate a problem
         * @hideinitializer
         */
        WARN("WARN"),
        /**
         * Log of an event that indicates an error
         * @hideinitializer
         */
        ERROR("ERROR"),
        /**
         * Log of an event that indicates an unrecoverable error
         * @hideinitializer
         */
        CRITICAL("CRITICAL");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        Level(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }

        /**
         * @internal
         */
        public char toChar() {
            return m_name.charAt(0);
        }
    }

    /**
     * Notifies the platform implementation of a log event from the AAC SDK logger
     *
     * @param  level The log level
     *
     * @param  time The timestamp of the logged message
     *
     * @param  source The source of the log message
     *
     * @param  message The log message
     *
     * @return @c true if the platform implementation successfully handled the log event, else @c false
     */
    public boolean logEvent(Level level, long time, String source, String message) {
        return false;
    }

    public boolean logEvent(Level level, long time, String source, byte[] message) {
        return logEvent(level, time, source, new String(message, StandardCharsets.UTF_8));
    }

    /**
     * Notifies the Engine to use the AAC SDK logger to log a message originating on the platform.
     * The log event will be received by the platform with a call to @c logEvent() from the Engine.
     *
     * @param  level The log level
     *
     * @param  tag The log tag
     *
     * @param  message The log message
     */
    public void log(Level level, String tag, String message) {
        log(getNativeRef(), level, tag, message);
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
    private native void log(long nativeRef, Level level, String tag, String message);
}
// END OF FILE
