package com.amazon.alexa.auto.app.dependencies;

import android.content.Context;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;

import dagger.Module;
import dagger.Provides;

/**
 * Module for providing {@link AlexaAppScope} Android Components.
 */
@Module
public class AndroidAppModule {
    @NonNull
    private final WeakReference<Context> mApplicationContext;

    /**
     * Construct the module with Android Application Context.
     *
     * @param context Android Application Context.
     */
    public AndroidAppModule(@NonNull Context context) {
        mApplicationContext = new WeakReference<>(context);
    }

    /**
     * Provides Android Application Context.
     *
     * @return Android Application Context.
     */
    @Provides
    @AlexaAppScope
    public WeakReference<Context> provideApplicationContext() {
        return mApplicationContext;
    }
}
