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
package com.amazon.alexa.auto.apis.alexaCustomAssistant;

import android.content.Context;
import android.view.ViewGroup;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to handle custom animation. A component can provide custom animation
 * that will be rendered in the layouts of the components that initialize it.
 */
public interface AnimationProvider extends ScopedComponent {
    /**
     * Initialize animation with the given context and view group.
     * @param context Android context.
     * @param viewGroup The layout that the animation will be rendered onto.
     */
    void initialize(Context context, ViewGroup viewGroup);

    /**
     * Un-initialize animation.
     */
    void uninitialize();

    /**
     * Provide custom layout from animation provider.
     * @return resource id for the custom layout.
     */
    int getCustomLayout();

    /**
     * Perform different animations depending on different signals.
     * @param signal Information for different animations to be rendered. For example, dialog state.
     */
    void doTakeOver(String signal);

    /**
     * Check if animation provider should take over the animation rendering.
     * @return true if animation provider should take over. Otherwise, return false.
     */
    boolean shouldTakeOver();

    /**
     * Prepare animation for PTT action.
     */
    void prepareAnimationForPTT();
}
