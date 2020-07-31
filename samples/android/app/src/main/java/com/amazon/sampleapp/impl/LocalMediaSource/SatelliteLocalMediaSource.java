package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class SatelliteLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public SatelliteLocalMediaSource(
            Context context, LoggerHandler logger, PlaybackControllerHandler playbackControllerHandler) {
        super(context, logger, Source.SATELLITE_RADIO, playbackControllerHandler);
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
