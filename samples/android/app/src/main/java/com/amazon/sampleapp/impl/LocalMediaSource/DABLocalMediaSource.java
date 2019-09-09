package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;


public class DABLocalMediaSource extends LocalMediaSourceHandler
{
    public DABLocalMediaSource( Context context, LoggerHandler logger ) {
        super( context, logger, Source.DAB );
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations()
    {
        return new SupportedPlaybackOperation[]{};
    }
}
