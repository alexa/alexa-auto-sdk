package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class SiriusXMLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";

    public SiriusXMLocalMediaSource(
            Context context, LoggerHandler logger, PlaybackControllerHandler playbackControllerHandler) {
        super(context, logger, Source.SIRIUS_XM, playbackControllerHandler);
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
        return new ContentSelector[] {ContentSelector.PRESET, ContentSelector.CHANNEL};
    }

    @Override
    protected String getSourcePlaybackState() {
        return m_state;
    }
}
