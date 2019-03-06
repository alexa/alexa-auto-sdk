/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.cbl;

import com.amazon.aace.core.PlatformInterface;

/**
 * PhoneCallController should be extended to perform CBL authentication
 */
abstract public class CBL extends PlatformInterface
{
    /**
     * Specifies the state of the authorization flow
     *
     */
    public enum CBLState
    {
        /**
         * CBL process is starting
         * @hideinitializer
         */
        STARTING("STARTING"),

        /**
         * Initiating the process to request a code pair
         * @hideinitializer
         */
        REQUESTING_CODE_PAIR("REQUESTING_CODE_PAIR"),

        /**
         * Code pair is received and is waiting on user to authenticate
         * @hideinitializer
         */
        CODE_PAIR_RECEIVED("CODE_PAIR_RECEIVED"),

        /**
         * Refreshing token state has begun
         * @hideinitializer
         */
        REFRESHING_TOKEN("REFRESHING_TOKEN"),

        /**
         * Requesting for authorization token
         * @hideinitializer
         */
        REQUESTING_TOKEN("REQUESTING_TOKEN"),

        /**
         * CBL process is ending
         * @hideinitializer
         */
        STOPPING("STOPPING");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private CBLState( String name ) {
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
     * Specifies the reason for a state change
     * @hideinitializer
     */
    public enum CBLStateChangedReason
    {
        /**
         * THe CBL state changed successfully
         * @hideinitializer
         */
        SUCCESS("SUCCESS"),

        /**
         * Error occured in the CBL process
         * @hideinitializer
         */
        ERROR("ERROR"),

        /**
         * Request timed out
         * @hideinitializer
         */
        TIMEOUT("TIMEOUT"),

        /**
         * Code pair has expired and use will need to initiate the authentication process again
         * @hideinitializer
         */
        CODE_PAIR_EXPIRED("CODE_PAIR_EXPIRED"),

        /**
         * No reason specified
         * @hideinitializer
         */
        NONE("NONE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private CBLStateChangedReason( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public CBL() {
    }

    /**
     * Notifies the platform implementation of an authorization flow state change
     */
    public void cblStateChanged( CBLState state, CBLStateChangedReason reason, String url, String code ) {
    }

    /**
    * Notifies the platform implementation to clear the refresh token
    */
    public void clearRefreshToken() {
    }

    /**
     * Notifies the platform implementation to set the refresh token
     */
    public void setRefreshToken( String refreshToken ) {
    }

    /**
     * Returns the refresh token stored by the platform implementation, otherwise return an empty string
     */
    public String getRefreshToken() {
        return null;
    }
    

    /**
     * Notifies the Engine to begin the authorization process
     */
    final protected void start() {
        start( getNativeObject() );
    }

    /**
     * Notifies the Engine to cancel the authorization process
     */
    final protected void cancel() {
        cancel( getNativeObject() );
    }

   private native void start( long nativeObject );
   private native void cancel( long nativeObject );
}

// END OF FILE
