package com.amazon.alexa.auto.media.dependencies;

import android.content.Context;
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
     * Proivdes the Media Session Manager.
     *
     * @param messageSender AACS Message sender.
     * @return Media Session Manager.
     */
    @Provides
    @Singleton
    public MediaSessionManager provideMediaSessionComponents(WeakReference<Context> context, MediaPlayerExo mediaPlayer,
            MediaSessionCompat mediaSession, PlaybackControlMessages messageSender) {
        Context contextStrong = context.get();
        Preconditions.checkNotNull(contextStrong);

        return new MediaSessionManager(mediaPlayer, mediaSession,
                new MediaMetadataProvider(Glide.with(contextStrong), context), new PlaybackController(messageSender),
                new CustomActionProviders(contextStrong, messageSender));
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
