package com.amazon.sampleapp.impl.LocalMediaSource;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class LineInLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public LineInLocalMediaSource(LoggerHandler logger) {
        super(logger, Source.LINE_IN);
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
                SupportedPlaybackOperation.ADJUST_SEEK, SupportedPlaybackOperation.FAVORITE,
                SupportedPlaybackOperation.UNFAVORITE, SupportedPlaybackOperation.FAST_FORWARD,
                SupportedPlaybackOperation.REWIND, SupportedPlaybackOperation.START_OVER};
    }

    @Override
    protected String getSourcePlaybackState() {
        return m_state;
    }
}
