/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.apis.uxRestrictions;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import io.reactivex.rxjava3.core.Observable;

/**
 * API interface to let Alexa App know about the current car UX restrictions state and
 * to observe car UX restriction updates.
 */
public interface CarUxRestrictionsController extends ScopedComponent {
    /**
     * Get active car UX restriction.
     *
     * @return CarUxRestriction Car UX restriction.
     */
    CarUxRestriction getActiveCarUxRestriction();

    /**
     * Fetch observable that can be used to monitor the car UX restrictions changes.
     *
     * @return Observable of car UX restriction status.
     */
    Observable<CarUxRestrictionStatus> observeCarUxRestrictionChanged();

    /**
     * Register listener for car UX restrictions changes.
     */
    void registerListener();

    /**
     * Unregister listener for car UX restrictions changes.
     */
    void unregisterListener();
}
