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
