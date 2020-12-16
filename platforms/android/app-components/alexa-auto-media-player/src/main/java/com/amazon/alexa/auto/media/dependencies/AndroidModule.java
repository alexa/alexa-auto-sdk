package com.amazon.alexa.auto.media.dependencies;

import android.content.Context;
import android.support.v4.media.session.MediaSessionCompat;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.media.MusicStreamAttributeUpdater;
import com.amazon.alexa.auto.media.player.NotificationController;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

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

    /**
     * Provide the Android Media Session.
     *
     * @return Media Session Compat.
     */
    @Provides
    @Singleton
    public MediaSessionCompat provideMediaSession() {
        Context context = this.mContext.get();
        if (context == null) {
            throw new RuntimeException("Invalid Context");
        }

        return new MediaSessionCompat(context, "AlexaMediaService");
    }

    /**
     * Provides Notification Controller.
     *
     * @return Notification Controller.
     */
    @Provides
    @Singleton
    public NotificationController provideNotificationController() {
        Context context = this.mContext.get();
        if (context == null) {
            throw new RuntimeException("Invalid Context");
        }

        return new NotificationController(context);
    }

    /**
     * Provides the music stream attribute updater.
     *
     * @return Music stream attribute updater.
     */
    @Provides
    public MusicStreamAttributeUpdater provideMusicStreamUpdater() {
        return new MusicStreamAttributeUpdater(mContext);
    }
}
