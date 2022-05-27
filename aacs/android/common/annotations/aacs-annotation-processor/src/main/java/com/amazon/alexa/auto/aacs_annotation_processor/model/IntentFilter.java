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

import javax.lang.model.type.TypeMirror;

@SuppressWarnings("unused")
public class IntentFilter {
    private final TypeMirror type;
    private final String name;
    private final String[] categories;
    private final String[] actions;
    private final String permission;

    public IntentFilter(TypeMirror type, String name, String[] categories, String[] actions, String permission) {
        this.type = type;
        this.name = name;
        this.categories = categories;
        this.actions = actions;
        this.permission = permission;
    }

    public TypeMirror getType() {
        return type;
    }

    public String getName() {
        return name;
    }

    public String[] getCategories() {
        return categories;
    }

    public String[] getActions() {
        return actions;
    }

    public String getPermission() {
        return permission;
    }
}
