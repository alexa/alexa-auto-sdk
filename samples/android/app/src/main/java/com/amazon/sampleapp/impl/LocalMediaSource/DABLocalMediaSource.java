package com.amazon.sampleapp.impl.LocalMediaSource;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class DABLocalMediaSource extends LocalMediaSourceHandler {
    public DABLocalMediaSource(LoggerHandler logger) {
        super(logger, Source.DAB);
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[] {};
    }
}
