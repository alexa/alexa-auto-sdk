package com.amazon.alexa.auto.voiceinteraction.dependencies;

import android.content.Context;

import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.voiceinteraction.config.AACSConfigurationPreferences;
import com.amazon.alexa.auto.voiceinteraction.config.AACSConfigurator;

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
