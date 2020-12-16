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
package com.amazon.sampleapp.core;

public abstract interface AuthorizationHandlerInterface {
    /**
     * Add the observer @c AuthorizationHandlerObserverInterface
     */
    public void addObserver(String service, AuthorizationHandlerObserverInterface observer);

    /**
     * Starts the authorization process.
     * Call to this API calls Authorization.startAuthorization.
     *
     * @param service The authorization service name.
     * @param data The authorization data to be passed to the authorization service.
     */
    public void startAuth(String service, String data);

    /**
     * Cancel the authorization process.
     * Call to this API calls Authorization.cancelAuthorization.
     *
     * @param service The authorization service name.
     */
    public void cancelAuth(String service);

    /**
     * Triggers the authorization to logout.
     * Call to this API calls Authorization.logout.
     *
     * @param service The authorization service name.
     */
    public void logoutAuth(String service);

    /**
     * An event during the authorization flow.
     * Call to this API calls Authorization.sendEvent
     *
     * @param service The authorization service name.
     * @param data The event data to be passed to the authorization service.
     */
    public void authEvent(String service, String data);
}