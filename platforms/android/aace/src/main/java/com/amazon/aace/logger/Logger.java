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

// aace/logger/Logger.java
// This is an automatically generated file.

package com.amazon.aace.logger;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c Logger should be extended by the platform implementation to handle logs coming from the SDK.
 */
abstract public class Logger extends PlatformInterface
{
    /**
     * Enum used to specify the severity assigned to a log message.
     */
    public enum Level
    {
        /**
         * Most verbose log level. Only enabled for debug builds.
         * @hideinitializer
         */
        VERBOSE("VERBOSE"),
        /**
         * Logs of normal operations, to be used in release builds.
         * @hideinitializer
         */
        INFO("INFO"),
        /**
         * Log of an event that may indicate a problem.
         * @hideinitializer
         */
        WARN("WARN"),
        /**
         * Log of an event that indicates an error.
         * @hideinitializer
         */
        ERROR("ERROR"),
        /**
         * Log of an event that indicates an unrecoverable error.
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
        Level( String name ) {
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
            return m_name.charAt( 0 );
        }
    }

    /**
     * Called when the platform implementation should handle a log event from the SDK logger.
     *
     * @param [in] level The @c aace::logger::Logger::Level log level
     * @param [in] time Time of the logged message
     * @param [in] source The source of the log message
     * @param [in] message The log message
     *
     * @return @c true if log event was handled by the platform.
     */
    public boolean logEvent( Level level, long time, String source, String message ) {
        return false;
    }

    /**
     * Use the Engine's logger to log an event.
     *
     * @param [in] level The @c aace::logger::Logger::Level log level
     * @param [in] tag The log tag
     * @param [in] message The log message
     */
    public void log( Level level, String tag, String message ) {
        log( getNativeObject(), level, tag, message );
    }

    private native void log( long nativeObject, Level level, String tag, String message );
}

// END OF FILE
