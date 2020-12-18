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
 * AuthProvider should be extended to manage access tokens for AVS authorization and report
 * client authorization state to the Engine.
 *
 * @deprecated This platform interface is being deprecated. Use `com.amazon.aace.authorization.Authorization` instead.
 */
abstract public class AuthProvider extends PlatformInterface {
    /**
     * Describes the state of client authorization with AVS
     */
    public enum AuthState {
        /**
         * Authorization has not yet been acquired.
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
         * Authorization has failed in a manner that cannot be corrected by retrying.
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
        private AuthState(String name) {
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
     * Describes an error during an attempt to authorize with AVS
     */
    public enum AuthError {
        /**
         * No error encountered
         * @hideinitializer
         */
        NO_ERROR("NO_ERROR"),
        /**
         * An error was encountered, but no error description can be determined.
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
         * The request is missing a required parameter, has an invalid value, or is otherwise malformed.
         * @hideinitializer
         */
        INVALID_REQUEST("INVALID_REQUEST"),
        /**
         * One of the values in the request was invalid.
         * @hideinitializer
         */
        INVALID_VALUE("INVALID_VALUE"),
        /**
         * The authorization code is invalid, expired, revoked, or was issued to a different client.
         * @hideinitializer
         */
        AUTHORIZATION_EXPIRED("AUTHORIZATION_EXPIRED"),
        /**
         * The client specified the wrong token type.
         * @hideinitializer
         */
        UNSUPPORTED_GRANT_TYPE("UNSUPPORTED_GRANT_TYPE"),
        /**
         * Invalid code pair provided in Code-based linking token request.
         * @hideinitializer
         */
        INVALID_CODE_PAIR("INVALID_CODE_PAIR"),
        /**
         * Waiting for user to authorize the specified code pair.
         * @hideinitializer
         */
        AUTHORIZATION_PENDING("AUTHORIZATION_PENDING"),
        /**
         * Client should slow down in the rate of requests polling for an access token.
         * @hideinitializer
         */
        SLOW_DOWN("SLOW_DOWN"),
        /**
         * Internal error in client code.
         * @hideinitializer
         */
        INTERNAL_ERROR("INTERNAL_ERROR"),
        /**
         * Client ID not valid for use with code based linking.
         * @hideinitializer
         */
        INVALID_CBL_CLIENT_ID("INVALID_CBL_CLIENT_ID");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private AuthError(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public AuthProvider() {}

    /**
     * Returns the token used by the platform implementation for authorization with AVS.
     * The platform implementation should retrieve an auth token if it does not have one.
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     *
     * @return The token used to authorize with AVS
     */
    public String getAuthToken() {
        return null;
    }

    /**
     * Returns the AVS authorization state of the platform implementation
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     *
     * @return The AVS authorization state
     */
    public AuthState getAuthState() {
        return null;
    }

    /**
     * Notifies the platform implementation that the specified access token was used in an
     * unauthorized request to AVS. AVS responded to this request with a 403 code
     * indicating the token was not valid.
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     *
     * @param token The access token used in an unauthorized request to AVS
     */
    public void authFailure(String token) {}

    /**
     * Notifies the Engine of a change in AVS authorization state in the platform implementation
     *
     * @deprecated This method is being deprecated as this platform interface is being deprecated.
     *
     * @param  authState The new authorization state
     * @param  authError The error state of the authorization attempt
     */
    final public void authStateChange(AuthState authState, AuthError authError) {
        authStateChange(getNativeRef(), authState, authError);
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
    private native void authStateChange(long nativeObject, AuthState authState, AuthError authError);
}
