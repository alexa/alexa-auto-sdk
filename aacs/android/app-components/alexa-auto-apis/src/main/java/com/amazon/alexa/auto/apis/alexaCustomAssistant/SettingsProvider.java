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

import com.amazon.alexa.auto.apis.app.ScopedComponent;

/**
 * An interface to handle custom setting related navigation graph and resources.
 * A component can provide custom setting navigation graph that will override
 * the default setting navigation graph.
 */
public interface SettingsProvider extends ScopedComponent {
    /**
     * Provide custom navigation graph from alexa settings provider.
     * @return resource for the custom navigation graph.
     */
    int getCustomSettingNavigationGraph();

    /**
     * Provide start destination for the navigation graph from alexa settings provider.
     * @return resource id for the start destination.
     */
    int getSettingStartDestination();

    /**
     * Provide setting resource id by key.
     * @param key The key to map to a specific setting resource id.
     * @return The setting resource id. Return 0 if there is no resource id.
     */
    int getSettingResId(String key);
}
