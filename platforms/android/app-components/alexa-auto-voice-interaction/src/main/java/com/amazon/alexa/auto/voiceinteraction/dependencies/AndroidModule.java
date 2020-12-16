package com.amazon.alexa.auto.voiceinteraction.dependencies;

import android.content.Context;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;

import dagger.Module;
import dagger.Provides;

/**
 * Module for providing Android Objects.
 */
@Module
public class AndroidModule {
    private WeakReference<Context> mContext;

    /**
     * Constructs the @c AndroidModule.
     *
     * @param context Android Context.
     */
    public AndroidModule(@NonNull Context context) {
        this.mContext = new WeakReference<>(context);
    }

    /**
     * Provides the Android Context.
     *
     * @return Android Context.
     */
    @Provides
    public WeakReference<Context> provideContext() {
        return this.mContext;
    }
}
