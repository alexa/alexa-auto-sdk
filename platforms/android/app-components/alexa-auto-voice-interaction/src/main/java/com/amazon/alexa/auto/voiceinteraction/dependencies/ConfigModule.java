package com.amazon.alexa.auto.voiceinteraction.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.voiceinteraction.config.AlexaPropertyManager;
import com.amazon.alexa.auto.voiceinteraction.config.LocalesProvider;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module for config helper classes.
 */
@Module
public class ConfigModule {
    /**
     * Provides instance of {@link AlexaPropertyManager}.
     *
     * @param context Android Context.
     */
    @Provides
    @Singleton
    public AlexaPropertyManager provideAlexaPropertyProvider(WeakReference<Context> context) {
        return new AlexaPropertyManager(context);
    }

    /**
     * Provides instance of {@link LocalesProvider}.
     *
     * @param context Android Context.
     */
    @Provides
    @Singleton
    public LocalesProvider provideLocalesProvider(WeakReference<Context> context) {
        return new LocalesProvider(context);
    }
}
