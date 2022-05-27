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

import android.view.ViewGroup;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;

/**
 * Component that handles getting the session view and vending
 * it out to components that want to inflate onto that view.
 */
public interface SessionViewController extends ScopedComponent {
    /**
     * Returns whether there is a current voice interaction session active.
     * @return is there is a voice interaction session active.
     */
    boolean isSessionActive();

    /**
     * Sets the session view container. This should be set when a voice interaction starts
     * and then set to null when the voice interaction is finished.
     * @param viewGroup container of VIS session.
     */
    void setSessionView(ViewGroup viewGroup);

    /**
     * Returns the view container to inflate onto. Note that only one template runtime template
     * will be inflated onto this container. AutoSDK handles that contract of only allowing one
     * template to be shown. It will make sure to call ClearTemplate if a new template needs to
     * be shown.
     * @return the view container.
     */
    Optional<ViewGroup> getTemplateRuntimeViewContainer();

    /**
     * Clears any existing template runtime template.
     */
    void clearTemplate();

    /**
     * Gets an observable that tells us if a template runtime template is inflated.
     * @return an observable that informs us whether a template runtime template is inflated.
     */
    Observable<Boolean> getTemplateDisplayedObservable();

    void setTemplateDisplayed();
}
