package com.amazon.alexa.auto.navigation.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;
import com.amazon.alexa.auto.navigation.handlers.NavigationDirectiveHandler;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;

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
    public NavigationDirectiveHandler provideNavigationDirectiveHandler(NavigationProvider navigationProvider) {
        return new NavigationDirectiveHandler(navigationProvider);
    }

    @Provides
    @Singleton
    public LocalSearchDirectiveHandler providePOIDirectiveHandler(
            WeakReference<Context> context, NavigationProvider navigationProvider) {
        return new LocalSearchDirectiveHandler(context, navigationProvider);
    }
}
