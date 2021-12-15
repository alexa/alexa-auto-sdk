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

package com.amazon.aacsipc;

import android.content.ComponentName;

/**
 * Simple class for defining a TargetComponent App for routing directives
 */
public class TargetComponent {
    // Reflects the type of target registered for the Directive
    public enum Type { ACTIVITY, SERVICE, RECEIVER, UNKNOWN }

    /**
     * Specifies the PackageName + Component for the target component.
     * Can be null if the component is not specified.
     */
    public final ComponentName component;

    /**
     * Specifies the type of registered Component.
     * Will be null if component is not specified.
     */
    public final Type type;

    /**
     * The package name of the target Component.
     * Can be null if target is either not found or the app needs disambiguation.
     */
    public final String packageName;

    private TargetComponent(final ComponentName component, final Type type) {
        this.component = component;
        this.type = type;
        packageName = component.getPackageName();
    }

    private TargetComponent() {
        component = null;
        type = Type.UNKNOWN;
        this.packageName = null;
    }

    public static TargetComponent withComponent(final ComponentName component, final Type type) {
        return new TargetComponent(component, type);
    }

    public static TargetComponent notFound() {
        return new TargetComponent();
    }
}
