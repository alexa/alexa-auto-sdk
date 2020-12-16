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

import com.amazon.aace.authorization.Authorization;
import com.amazon.sampleapp.core.AuthorizationHandlerInterface;

import org.json.JSONObject;

public abstract interface AuthorizationHandlerObserverInterface {
    /**
     * Initialize the object with the required references.
     *
     * @param handler Reference to the class that handles the authorization process.
     * @param deviceConfig Json holding the device config required for the authorization.
     */
    public void initialize(AuthorizationHandlerInterface handler, JSONObject deviceConfig);

    /**
     * Enables the module to start the authorization.
     * Called when only when it was previously active.
     */
    public void enable();

    /**
     * The authorization request.
     * Called when Authorization.eventReceived is received.
     *
     * @param event  The authorization event in json.
     */
    public void onEventReceived(String event);

    /**
     * The authorization state change event.
     * Called when Authorization.authorizationStateChanged is received.
     *
     * @param state Represents the authorization state.
     */
    public void onAuthorizationStateChanged(Authorization.AuthorizationState state);

    /**
     * The authorization error event.
     * Called when Authorization.authorizationError is received.
     *
     * @param error  The error reason.
     * @param message The error message to log.
     */
    public void onAuthorizationError(String error, String message);

    /**
     * Retrieves the authorization data identified by the key.
     * Called when Authorization.getAuthorizationData is received.
     *
     * @param key Denotes the key for which data is retrieved.
     */
    public String onGetAuthorizationData(String key);

    /**
     * Stores the authorization data identified by the key.
     * Called when Authorization.setAuthorizationData is received.
     *
     * @param key Denotes the key for which data is stored.
     * @param data Holds the authorization data to be stored.
     */
    public void onSetAuthorizationData(String key, String data);
}