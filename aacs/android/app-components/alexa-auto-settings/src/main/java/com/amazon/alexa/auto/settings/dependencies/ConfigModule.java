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

import com.amazon.alexa.auto.apps.common.util.config.AlexaLocalesProvider;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;

import java.lang.ref.WeakReference;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.inject.Named;
import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module for config helper classes.
 */
@Module
public class ConfigModule {
    private static final String CONFIG_EXECUTOR_SERVICE = "config-exec-svc";

    /**
     * Provides instance of {@link AlexaPropertyManager}.
     *
     * @param context Android Context.
     */
    @Provides
    @Singleton
    public AlexaPropertyManager provideAlexaPropertyProvider(
            WeakReference<Context> context, @Named(CONFIG_EXECUTOR_SERVICE) ExecutorService executorService) {
        return new AlexaPropertyManager(context, executorService);
    }

    /**
     * Provides instance of {@link AlexaLocalesProvider}.
     *
     * @param context Android Context.
     */
    @Provides
    @Singleton
    public AlexaLocalesProvider provideLocalesProvider(
            WeakReference<Context> context, @Named(CONFIG_EXECUTOR_SERVICE) ExecutorService executorService) {
        return new AlexaLocalesProvider(context, executorService);
    }

    @Provides
    @Singleton
    @Named(CONFIG_EXECUTOR_SERVICE)
    public ExecutorService provideExecutorService() {
        return Executors.newSingleThreadExecutor();
    }
}
