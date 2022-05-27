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
import android.media.AudioManager;

import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.MusicStreamAttributeUpdater;
import com.amazon.alexa.auto.media.aacs.handlers.AudioPlayerHandler;
import com.amazon.alexa.auto.media.aacs.handlers.TemplateRuntimeHandler;
import com.amazon.alexa.auto.media.player.MediaPlayerAudioFocusController;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;
import com.amazon.alexa.auto.media.session.MediaSessionManager;

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
    public PlaybackControlMessages providePlaybackControlMessageSender(AACSMessageSender sender) {
        return new PlaybackControlMessages(sender);
    }

    @Provides
    @Singleton
    public AACSMessageSender provideMessageSender(WeakReference<Context> context) {
        return new AACSMessageSender(context, new AACSSender());
    }

    @Provides
    public AudioPlayerHandler provideAudioPlayerHandler(WeakReference<Context> contextWk,
            AACSMessageSender messageSender, MediaPlayerExo mediaPlayer,
            PlaybackControlMessages playbackControlMessages, MusicStreamAttributeUpdater musicStreamUpdater) {
        Context context = contextWk.get();
        Preconditions.checkNotNull(context);

        AudioManager audioManager =
                (AudioManager) context.getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
        MediaPlayerAudioFocusController focusController = new MediaPlayerAudioFocusController(audioManager);

        return new AudioPlayerHandler(
                messageSender, mediaPlayer, focusController, playbackControlMessages, musicStreamUpdater);
    }

    @Provides
    public TemplateRuntimeHandler provideTemplateRuntimeHandler(MediaSessionManager mediaSessionManager) {
        return new TemplateRuntimeHandler(mediaSessionManager);
    }
}
