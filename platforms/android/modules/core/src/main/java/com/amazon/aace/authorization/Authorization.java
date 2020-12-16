/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.authorization;

import com.amazon.aace.core.PlatformInterface;

/**
 * Authorization should be extended to carry out the authorization's required for the Engine.
 *
 * This is a single platform interface for any supported authorizations by the Engine. The authorization process
 * shall be carried out by the individual providers, referred here as authorization service. The platform implementation
 * is responsible to follow the protocol established by the authorization service. Please refer to the authorization
 * service documentation for the protocol information.
 */
abstract public class Authorization extends PlatformInterface {
    /**
     * Describes the authorization states.
     */

    public enum AuthorizationState {
        /**
         * Device is unauthorized
         * @hideinitializer
         */
        UNAUTHORIZED("UNAUTHORIZED"),

        /**
         * Device authorization in-progress
         * @hideinitializer
         */
        AUTHORIZING("AUTHORIZING"),

        /**
         * Device is authorized
         * @hideinitializer
         */
        AUTHORIZED("AUTHORIZED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        AuthorizationState(String name) {
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

    public Authorization() {}

    /**
     * Notifies the platform implementation about the authorization event.
     *
     * @param service Indicates the authorization service.
     * @param event JSON representation of the authorization event as defined by the authorization service.
     */
    public void eventReceived(String service, String event){};

    /**
     * Notifies the platform implementation about the authorization state change.
     *
     * @param service Indicates the authorization service.
     * @param state The authorization state represented using @c AuthorizationState
     */
    public void authorizationStateChanged(String service, AuthorizationState state){};

    /**
     * Notifies the platform implementation about any error in the authorization process.
     *
     * @param service Indicates the authorization service.
     * @param error Indicates the authorization error. The value is defined by the each authorization service.
     * @param message Error message for logging purpose.
     */
    public void authorizationError(String service, String error, String message){};

    /**
     * Function to get the authorization data from the platform implementation.
     * @note Call to this API need not necessarily require a call to the @c setAuthorizationData.
     *
     * @param service Indicates the authorization service.
     * @param key The key of the data requested.
     * @return Returns the value of the key if available otherwise returns an empty string.
     */
    public String getAuthorizationData(String service, String key) {
        return "";
    };

    /**
     * Function to set the authorization data in platform implementation.
     * The platform implementation are expected to store these values securely.
     *
     * @param service Indicates the authorization service.
     * @param key Represents the key of the data
     * @param data The data to be set.
     */
    public void setAuthorizationData(String service, String key, String data){};

    /**
     * Notifies the Engine to start the authorization process.
     * Engine to call @c authStateChanged with @c AUTHORIZING on the start of authorization process.
     * Engine to call @c authStateChanged with @c AUTHORIZED on the successful authorization.
     *
     * @param service Indicates the authorization service.
     * @param data Represents the data in json format. The value of this is defined by each authorization service.
     */
    final protected void startAuthorization(String service, String data) {
        startAuthorization(getNativeRef(), service, data);
    }

    /**
     * Notifies the Engine to cancel the authorization.
     * @note This does not logout the active authorization.
     *
     * @param service Indicates the authorization service.
     */
    final protected void cancelAuthorization(String service) {
        cancelAuthorization(getNativeRef(), service);
    }

    /**
     * The authorization event from platform implementation.
     *
     * @param service Indicates the authorization service.
     * @param event Represents the event in json format. The value of this is defined by each authorization service.
     */
    final protected void sendEvent(String service, String event) {
        sendEvent(getNativeRef(), service, event);
    }

    /**
     * Notifies the Engine to logout.
     * Engine would call @c authStateChanged with @c UNAUTHORIZED on the successful logout.
     *
     * @param service Indicates the authorization service that needs to be logged out.
     */
    final protected void logout(String service) {
        logout(getNativeRef(), service);
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
    private native void startAuthorization(long nativeRef, String service, String data);
    private native void cancelAuthorization(long nativeRef, String service);
    private native void sendEvent(long nativeRef, String service, String event);
    private native void logout(long nativeRef, String service);
}
