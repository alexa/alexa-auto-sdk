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
package com.amazon.alexa.auto.aacs_annotation_processor.model;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import javax.lang.model.element.PackageElement;
import javax.lang.model.element.TypeElement;

public class PackageBinding {
    private final PackageElement packageElement;
    private final Map<TypeElement, IntentFilterClassBinding> intentFilterClassBindings;
    private final Map<TypeElement, LifecycleObserverClassBinding> lifecycleObserverClassBindings;
    private final Map<TypeElement, NaviProviderClassBinding> naviProviderClassBindings;

    public PackageBinding(PackageElement packageElement) {
        this.packageElement = packageElement;
        this.intentFilterClassBindings = new HashMap<>();
        this.lifecycleObserverClassBindings = new HashMap<>();
        this.naviProviderClassBindings = new HashMap<>();
    }

    public String getPackageName() {
        return packageElement.getQualifiedName().toString();
    }

    public Collection<IntentFilterClassBinding> getIntentFilterClassBindings() {
        return intentFilterClassBindings.values();
    }

    public Collection<LifecycleObserverClassBinding> getLifecycleObserverClassBindings() {
        return lifecycleObserverClassBindings.values();
    }

    public Collection<NaviProviderClassBinding> getNaviProviderClassBinding() {
        return naviProviderClassBindings.values();
    }

    public IntentFilterClassBinding getIntentFilterClassBinding(TypeElement typeElement) {
        return intentFilterClassBindings.computeIfAbsent(typeElement, IntentFilterClassBinding::new);
    }

    public LifecycleObserverClassBinding getLifecycleObserverClassBinding(TypeElement typeElement) {
        return lifecycleObserverClassBindings.computeIfAbsent(typeElement, LifecycleObserverClassBinding::new);
    }

    public NaviProviderClassBinding getNaviProviderClassBinding(TypeElement typeElement) {
        return naviProviderClassBindings.computeIfAbsent(typeElement, NaviProviderClassBinding::new);
    }
}
