package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;
import android.util.Log;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class DefaultMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public DefaultMediaSource(LoggerHandler logger) {
        super(logger, Source.DEFAULT);
    }

    @Override
    protected void setPlaybackState(String state) {
        m_state = state;
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[] {SupportedPlaybackOperation.PLAY, SupportedPlaybackOperation.PAUSE,
                SupportedPlaybackOperation.STOP, SupportedPlaybackOperation.PREVIOUS, SupportedPlaybackOperation.NEXT};
    }
    @Override
    protected ContentSelector[] getSupportedContentSelectors() {
        return new ContentSelector[] {ContentSelector.PRESET};
    }

    @Override
    protected String getSourcePlaybackState() {
        return m_state;
    }
}
