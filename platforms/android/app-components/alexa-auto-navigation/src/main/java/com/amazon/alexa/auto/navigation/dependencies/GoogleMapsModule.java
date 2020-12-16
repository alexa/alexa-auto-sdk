package com.amazon.alexa.auto.navigation.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.navigation.providers.NavigationProvider;
import com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module for injecting any dependencies specific to google maps.
 */
@Module
public class GoogleMapsModule {
    @Provides
    @Singleton
    public NavigationProvider provideGoogleMapsHelper(WeakReference<Context> context) {
        return new GoogleMapsNavigationProvider(context);
    }
}
