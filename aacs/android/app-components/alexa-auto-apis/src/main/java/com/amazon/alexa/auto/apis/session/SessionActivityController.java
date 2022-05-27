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
package com.amazon.alexa.auto.apis.session;

import androidx.fragment.app.Fragment;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import io.reactivex.rxjava3.core.Observable;

/**
 * Component that handles getting the session activity and vending
 * it out to components that want to inflate its voice fragment onto that activity.
 */
public interface SessionActivityController extends ScopedComponent {
    /**
     * Add voice fragment into voice session activity.
     * @param fragment voice fragment
     */
    void addFragment(Fragment fragment);

    /**
     * Remove any existing voice fragment from voice session activity.
     */
    void removeFragment();

    /**
     * Get voice fragment which is added into voice session activity.
     * @return voice fragment
     */
    Fragment getFragment();

    /**
     * Check if voice fragment has been added.
     * @return true if added
     */
    boolean isFragmentAdded();

    /**
     * Gets an observable that tells us if a voice fragment is inflated.
     * @return an observable that informs us whether a voice fragment is inflated.
     */
    Observable<Boolean> getFragmentAddedObservable();
}
