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
import android.media.AudioManager;
import android.support.v4.media.session.MediaSessionCompat;

import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.R;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;
import com.amazon.alexa.auto.media.player.MediaSourceFactory;
import com.amazon.alexa.auto.media.session.CustomActionProviders;
import com.amazon.alexa.auto.media.session.MediaMetadataProvider;
import com.amazon.alexa.auto.media.session.MediaSessionManager;
import com.amazon.alexa.auto.media.session.PlaybackControlButtonActionProvider;
import com.amazon.alexa.auto.media.session.PlaybackController;
import com.bumptech.glide.Glide;
import com.google.android.exoplayer2.SimpleExoPlayer;

import java.lang.ref.WeakReference;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide Media Objects.
 */
@Module
public class MediaModule {
    /**
     * Provides the Media Session Manager.
     *
     * @param messageSender AACS Message sender.
     * @return Media Session Manager.
     */
    @Provides
    @Singleton
    public MediaSessionManager provideMediaSessionComponents(WeakReference<Context> context, MediaPlayerExo mediaPlayer,
            MediaSessionCompat mediaSession, PlaybackControlMessages messageSender,
            SharedPreferences sharedPreferences) {
        Context contextStrong = context.get();
        Preconditions.checkNotNull(contextStrong);
        AudioManager audioManager = (AudioManager) context.get().getSystemService(Context.AUDIO_SERVICE);

        return new MediaSessionManager(mediaPlayer, mediaSession,
                new MediaMetadataProvider(Glide.with(contextStrong), context), new PlaybackController(messageSender),
                new CustomActionProviders(contextStrong, messageSender), sharedPreferences, audioManager);
    }

    /**
     * Provides the Media Player.
     *
     * @param contextWk Android Context.
     * @return Media Player.
     */
    @Provides
    @Singleton
    public MediaPlayerExo provideMediaPlayer(WeakReference<Context> contextWk) {
        Context context = contextWk.get();
        if (context == null) {
            throw new RuntimeException("Invalid Context");
        }

        SimpleExoPlayer exoPlayer = new SimpleExoPlayer.Builder(context).build();
        MediaSourceFactory mediaSourceFactory = new MediaSourceFactory(context);
        return new MediaPlayerExo(exoPlayer, mediaSourceFactory);
    }
}
