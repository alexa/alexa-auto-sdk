package com.amazon.sampleapp.impl.LocalMediaSource;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class FMLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public FMLocalMediaSource(LoggerHandler logger) {
        super(logger, Source.FM_RADIO);
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
        return new ContentSelector[] {ContentSelector.PRESET, ContentSelector.FREQUENCY};
    }

    @Override
    protected String getSourcePlaybackState() {
        return m_state;
    }
}
