package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;
import android.util.Log;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.LocalMediaSource;
import com.amazon.aace.alexa.Speaker;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class FMLocalMediaSource extends LocalMediaSourceHandler
{
    String m_state = "IDLE";

    public FMLocalMediaSource( Context context, LoggerHandler logger ) {
        super( context, logger, Source.FM_RADIO );
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
                ContentSelector.FREQUENCY
        };
    }

    @Override
    protected String getSourcePlaybackState()
    {
        return m_state;
    }
}
