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
package com.amazon.sampleapp.core;

import com.amazon.aace.alexa.AlexaClient.AuthError;
import com.amazon.aace.alexa.AlexaClient.AuthState;

// Interface for any class that needs to observe the current authentication state
public interface AuthStateObserver {
    /**
     * Notifies the observer of a change in authentication state
     * @param authState The new authentication state
     * @param authError The error reason associated with the authentication state change
     */
    void onAuthStateChanged(AuthState authState, AuthError authError);
}
