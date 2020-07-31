package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class AMLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";
    public AMLocalMediaSource(
            Context context, LoggerHandler logger, PlaybackControllerHandler playbackControllerHandler) {
        super(context, logger, Source.AM_RADIO, playbackControllerHandler);
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
