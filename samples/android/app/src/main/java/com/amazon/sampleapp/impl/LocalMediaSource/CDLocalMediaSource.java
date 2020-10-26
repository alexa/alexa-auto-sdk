package com.amazon.sampleapp.impl.LocalMediaSource;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class CDLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public CDLocalMediaSource(LoggerHandler logger) {
        super(logger, Source.COMPACT_DISC);
    }

    @Override
    protected void setPlaybackState(String state) {
        m_state = state;
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[] {SupportedPlaybackOperation.PLAY, SupportedPlaybackOperation.PAUSE,
                SupportedPlaybackOperation.STOP, SupportedPlaybackOperation.PREVIOUS, SupportedPlaybackOperation.NEXT,
                SupportedPlaybackOperation.ENABLE_SHUFFLE, SupportedPlaybackOperation.DISABLE_SHUFFLE,
                SupportedPlaybackOperation.ENABLE_REPEAT_ONE, SupportedPlaybackOperation.ENABLE_REPEAT,
                SupportedPlaybackOperation.DISABLE_REPEAT, SupportedPlaybackOperation.SEEK,
                SupportedPlaybackOperation.ADJUST_SEEK};
    }

    @Override
    protected String getSourcePlaybackState() {
        return m_state;
    }
}
