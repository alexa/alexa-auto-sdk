package com.amazon.sampleapp.impl.LocalMediaSource;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class SatelliteLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public SatelliteLocalMediaSource(LoggerHandler logger) {
        super(logger, Source.SATELLITE_RADIO);
    }

    @Override
    protected void setPlaybackState(String state) {
        m_state = state;
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[] {
                SupportedPlaybackOperation.PLAY, SupportedPlaybackOperation.PAUSE, SupportedPlaybackOperation.STOP};
    }

    @Override
    protected String getSourcePlaybackState() {
        return m_state;
    }
}
