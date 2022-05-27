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

import java.util.HashSet;
import java.util.Set;

import javax.lang.model.element.TypeElement;

public class NaviProviderClassBinding {
    private final TypeElement classElement;
    private final Set<NaviProvider> naviProviderModuleSet;

    public NaviProviderClassBinding(TypeElement typeElement) {
        this.classElement = typeElement;
        this.naviProviderModuleSet = new HashSet<>();
    }

    public TypeElement getClassElement() {
        return classElement;
    }

    public Set<NaviProvider> getNaviProviderModules() {
        return naviProviderModuleSet;
    }

    public void addNaviProviderObserverBinding(NaviProvider naviProvider) {
        naviProviderModuleSet.add(naviProvider);
    }
}
