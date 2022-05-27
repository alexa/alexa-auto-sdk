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

public class IntentFilterBindingSet {
    private final Map<PackageElement, PackageBinding> packageBindingMap;

    public IntentFilterBindingSet() {
        packageBindingMap = new HashMap<>();
    }

    public Collection<PackageBinding> getPackageBindings() {
        return packageBindingMap.values();
    }

    public PackageBinding getPackageBinding(PackageElement packageElement) {
        return packageBindingMap.computeIfAbsent(packageElement, PackageBinding::new);
    }

    public void addBinding(PackageElement packageElement, TypeElement classElement, IntentFilter intentFilterBinding) {
        PackageBinding packageBinding = getPackageBinding(packageElement);
        IntentFilterClassBinding intentFilterClassBinding = packageBinding.getIntentFilterClassBinding(classElement);
        intentFilterClassBinding.addIntentFilterBinding(intentFilterBinding);
    }
}
