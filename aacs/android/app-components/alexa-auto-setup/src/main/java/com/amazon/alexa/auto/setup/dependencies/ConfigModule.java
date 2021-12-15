package com.amazon.alexa.auto.setup.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;

import java.lang.ref.WeakReference;
import java.util.concurrent.ExecutorService;

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
     * Provides instance of {@link LocalesProvider}.
     *
     * @param context Android Context.
     */
    @Provides
    @Singleton
    public LocalesProvider provideLocalesProvider(
            WeakReference<Context> context, @Named(CONFIG_EXECUTOR_SERVICE) ExecutorService executorService) {
        return new LocalesProvider(context, executorService);
    }
}
