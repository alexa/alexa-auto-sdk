package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

public class CDLocalMediaSource extends LocalMediaSourceHandler
{
    String m_state = "IDLE";

    public CDLocalMediaSource(Context context, LoggerHandler logger, PlaybackControllerHandler playbackControllerHandler) {
        super( context, logger, Source.COMPACT_DISC, playbackControllerHandler );
    }

    @Override
    protected void setPlaybackState( String state ) {
        m_state = state;
    }

    @Override
    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations()
    {
        return new SupportedPlaybackOperation[]{
            SupportedPlaybackOperation.PLAY,
            SupportedPlaybackOperation.PAUSE,
            SupportedPlaybackOperation.STOP,
            SupportedPlaybackOperation.PREVIOUS,
            SupportedPlaybackOperation.NEXT,
            SupportedPlaybackOperation.ENABLE_SHUFFLE,
            SupportedPlaybackOperation.DISABLE_SHUFFLE,
            SupportedPlaybackOperation.ENABLE_REPEAT_ONE,
            SupportedPlaybackOperation.ENABLE_REPEAT,
            SupportedPlaybackOperation.DISABLE_REPEAT,
            SupportedPlaybackOperation.SEEK,
            SupportedPlaybackOperation.ADJUST_SEEK
        };
    }

    @Override
    protected String getSourcePlaybackState()
    {
        return m_state;
    }
}
