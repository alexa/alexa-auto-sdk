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
package com.amazon.alexa.auto.settings.dependencies;

import android.content.Context;

import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.settings.config.AACSConfigurationPreferences;
import com.amazon.alexa.auto.settings.config.AACSConfigurator;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects to communicate with AACS.
 */
@Module
public class AACSModule {
    /**
     * Provides instance of {@link AACSConfigurator}.
     *
     * @param context Android Context.
     * @param sender Sender to send messages to AACS.
     * @return Instance of {@link AACSConfigurator}.
     */
    @Provides
    @Singleton
    public AACSConfigurator provideAACSConfigurator(WeakReference<Context> context, AACSSender sender) {
        return new AACSConfigurator(context, sender, new AACSConfigurationPreferences(context));
    }

    /**
     * Provides an instance of {@link AACSSender}.
     *
     * @return An instance of {@link AACSSender}.
     */
    @Provides
    @Singleton
    public AACSSender provideAACSSender() {
        return new AACSSender();
    }
}
