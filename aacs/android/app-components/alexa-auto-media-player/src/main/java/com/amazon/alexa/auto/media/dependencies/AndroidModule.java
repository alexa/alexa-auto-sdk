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
package com.amazon.alexa.auto.media.dependencies;

import android.content.Context;
import android.content.SharedPreferences;
import android.support.v4.media.session.MediaSessionCompat;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.media.MusicStreamAttributeUpdater;
import com.amazon.alexa.auto.media.ShutdownActionReceiver;
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
    private static final String MEDIA_STATE_KEY = "MEDIA_STATE_KEY";

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

    @Provides
    @Singleton
    public SharedPreferences provideSharedPreferences() {
        Context context = this.mContext.get();
        return context.getSharedPreferences(MEDIA_STATE_KEY, Context.MODE_PRIVATE);
    }

    @Provides
    @Singleton
    public ShutdownActionReceiver provideShutdownObserver() {
        return new ShutdownActionReceiver(mContext);
    }
}
