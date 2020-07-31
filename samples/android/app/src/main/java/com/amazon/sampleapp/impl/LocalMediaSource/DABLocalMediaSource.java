package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class DABLocalMediaSource extends LocalMediaSourceHandler {
    public DABLocalMediaSource(
            Context context, LoggerHandler logger, PlaybackControllerHandler playbackControllerHandler) {
        super(context, logger, Source.DAB, playbackControllerHandler);
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[] {};
    }
}
