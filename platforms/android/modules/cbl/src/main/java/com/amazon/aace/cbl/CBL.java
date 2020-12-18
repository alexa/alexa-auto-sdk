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

package com.amazon.aace.cbl;

import com.amazon.aace.core.PlatformInterface;

/**
 * CBL should be extended to perform CBL authentication.
 *
 * @deprecated This platform interface is being deprecated. Use `com.amazon.aace.authorization.Authorization` instead.
 */
abstract public class CBL extends PlatformInterface {
    /**
     * Specifies the state of the authorization flow
     *
     */
    public enum CBLState {
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
        private CBLState(String name) {
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
    public enum CBLStateChangedReason {
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
         * Authorization token is invalid, expired, revoked, or was issued to a different client
         * @hideinitializer
         */
        AUTHORIZATION_EXPIRED("AUTHORIZATION_EXPIRED"),

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
        private CBLStateChangedReason(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public CBL() {}

    /**
     * Notifies the platform implementation of an authorization flow state change
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    public void cblStateChanged(CBLState state, CBLStateChangedReason reason, String url, String code) {}

    /**
     * Notifies the platform implementation to clear the refresh token
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    public void clearRefreshToken() {}

    /**
     * Notifies the platform implementation to set the refresh token
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    public void setRefreshToken(String refreshToken) {}

    /**
     * Returns the refresh token stored by the platform implementation, otherwise return an empty string
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    public String getRefreshToken() {
        return "";
    }

    /**
     * Notifies the platform implementation to set the user profile, this is requested one time after authorization
     * requestUserProfile must be enabled in configuration
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    public void setUserProfile(String name, String email) {}

    /**
     * Notifies the Engine to begin the authorization process
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    final protected void start() {
        start(getNativeRef());
    }

    /**
     * Notifies the Engine to cancel the authorization process
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    final protected void cancel() {
        cancel(getNativeRef());
    }

    /**
     * Notifies the Engine to reset the authorization state
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     */
    final protected void reset() {
        reset(getNativeRef());
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
    private native void start(long nativeRef);
    private native void cancel(long nativeRef);
    private native void reset(long nativeRef);
}

// END OF FILE
