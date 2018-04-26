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

// aace/alexa/AuthProvider.java
// This is an automatically generated file.

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * The @c AuthProvider class should be implemented by clients who handle their own authentication states and manage their own access tokens.  i.e. mobile clients that authenticate with LWA and cannot pass a refresh token to configure the Engine with.
 */
abstract public class AuthProvider extends PlatformInterface
{
    /**
     * The enum AuthState describes the state of authorization.
     */
    public enum AuthState
    {
        /**
         * Authorization not yet acquired.
         * @hideinitializer
         */
        UNINITIALIZED("UNINITIALIZED"),
        /**
         * Authorization has been refreshed.
         * @hideinitializer
         */
        REFRESHED("REFRESHED"),
        /**
         * Authorization has expired.
         * @hideinitializer
         */
        EXPIRED("EXPIRED"),
        /**
         * Authorization failed in a manner that cannot be corrected by retrying.
         * @hideinitializer
         */
        UNRECOVERABLE_ERROR("UNRECOVERABLE_ERROR");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AuthState( String name ) {
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
     * The enum AuthError encodes possible errors which may occur when changing authorization state.
     */
    public enum AuthError
    {
        /**
         * No error.
         * @hideinitializer
         */
        NO_ERROR("NO_ERROR"),
        /**
         * An unknown body containing no error field has been encountered.
         * @hideinitializer
         */
        UNKNOWN_ERROR("UNKNOWN_ERROR"),
        /**
         * The client authorization failed.
         * @hideinitializer
         */
        AUTHORIZATION_FAILED("AUTHORIZATION_FAILED"),
        /**
         * The client is not authorized to use authorization codes.
         * @hideinitializer
         */
        UNAUTHORIZED_CLIENT("UNAUTHORIZED_CLIENT"),
        /**
         * The server encountered a runtime error.
         * @hideinitializer
         */
        SERVER_ERROR("SERVER_ERROR"),
        /**
         * The request is missing a required parameter, has an invalid value, or is otherwise improperly formed.
         * @hideinitializer
         */
        INVALID_REQUEST("INVALID_REQUEST"),
        /**
         * The authorization code is invalid, expired, revoked, or was issued to a different client.
         * @hideinitializer
         */
        AUTHORIZATION_EXPIRED("AUTHORIZATION_EXPIRED"),
        /**
         * The client specified the wrong token type.
         * @hideinitializer
         */
        UNSUPPORTED_GRANT_TYPE("UNSUPPORTED_GRANT_TYPE");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AuthError( String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public AuthProvider() {
    }

    /**
     * Called when the platform implementation should retrieve the an auth token
     *
     * @return the auth token from the platform implementation of AuthProvider.
     */
    public String getAuthToken() {
        return null;
    }

    /**
     * Called when the platform implementation should retrieve an authState
     *
     * @return the auth state from the platform implementation of AuthProvider.
     */
    public AuthState getAuthState() {
        return null;
    }

    /**
     * Notify the Engine that the Auth State/Error has changed
     *
     * @param [in] authState AuthState the authState of the platform implementation
     * @param [in] authError AuthError the authError of the platform implementation
     */
    public void authStateChange( AuthState authState, AuthError authError ) {
        authStateChange( getNativeObject(), authState, authError );
    }

    private native void authStateChange( long nativeObject, AuthState authState, AuthError authError );
}

// END OF FILE
