package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;
import android.util.Log;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.Speaker;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;


public class SiriusXMLocalMediaSource extends LocalMediaSourceHandler {
    String m_state = "IDLE";
    public SiriusXMLocalMediaSource(Context context, LoggerHandler logger ) {
        super( context, logger, Source.SIRIUS_XM );
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
                SupportedPlaybackOperation.STOP
        };
    }

    @Override
    protected ContentSelector[] getSupportedContentSelectors()
    {
        return new ContentSelector[]{
                ContentSelector.PRESET,
                ContentSelector.CHANNEL
        };
    }

    @Override
    protected String getSourcePlaybackState()
    {
        return m_state;
    }
}
