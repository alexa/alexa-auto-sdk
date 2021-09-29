package com.amazon.alexa.auto.templateruntime.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.templateruntime.weather.WeatherDirectiveHandler;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects to communicate with AACS.
 */
@Module
public class AACSModule {
    @Provides
    @Singleton
    public WeatherDirectiveHandler provideWeatherDirectiveHandler(
            WeakReference<Context> context) {
        return new WeatherDirectiveHandler(context);
    }
}
