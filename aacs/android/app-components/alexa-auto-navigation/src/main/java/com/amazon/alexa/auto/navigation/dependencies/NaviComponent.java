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
package com.amazon.alexa.auto.navigation.dependencies;

import com.amazon.alexa.auto.navigation.receiver.LocalSearchTemplateRuntimeReceiver;
import com.amazon.alexa.auto.navigation.receiver.NaviFavoritesReceiver;
import com.amazon.alexa.auto.navigation.receiver.NaviReceiver;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Navigation Dependencies.
 */
@Component(modules = {AACSModule.class, MapsModule.class, AndroidModule.class})
@Singleton
public interface NaviComponent {
    /**
     * Inject dependencies for @c NavigationReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectNavigationReceiver(NaviReceiver receiver);

    /**
     * Inject dependencies for @c POIReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectPOIReceiver(LocalSearchTemplateRuntimeReceiver receiver);

    /**
     * Inject dependencies for @c NaviFavoritesReceiver.
     *
     * @param receiver receiver where dependencies are injected.
     */
    void injectNaviFavoritesReceiver(NaviFavoritesReceiver receiver);
}
