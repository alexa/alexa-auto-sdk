package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;
import android.util.Log;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.LocalMediaSource;
import com.amazon.aace.alexa.Speaker;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public abstract class LocalMediaSourceHandler extends LocalMediaSource
{
    private static final String sTag = LocalMediaSourceHandler.class.getSimpleName();

    private final LoggerHandler mLogger;

    private float mVolume = 0.5f;

    protected LocalMediaSourceHandler(Context context, LoggerHandler logger, Source type) {
        super( type, new LocalMediaSourceSpeaker());
        mLogger = logger;
    }

    @Override
    public boolean play( ContentSelector selector, String payload ) {
        mLogger.postInfo( sTag, String.format( "play [source=%s,selector=%s,payload=%s]", getSource(), selector.toString(), payload));
        setFocus();
        setPlaybackState("PLAYING");
        return true;
    }

    @Override
    public boolean playControl( PlayControlType controlType ) {
        mLogger.postInfo( sTag, String.format( "playControl [source=%s,controlType=%s]", getSource(), controlType.toString()));
        setFocus();
        switch( controlType ) {
            case PAUSE:
                setPlaybackState("STOPPED");
                break;
            case RESUME:
                setPlaybackState("PLAYING");
                break;
        }
        return true;
    }

    @Override
    public boolean seek( long offset ) {
        mLogger.postInfo( sTag, String.format( "seek [source=%s,offset=%d]", getSource(), offset));
        return true;

    }

    @Override
    public boolean adjustSeek( long deltaOffset ) {
        mLogger.postInfo( sTag, String.format( "adjustSeek [source=%s,deltaOffset=%d]", getSource(), deltaOffset));
        return true;
    }

    @Override
    public LocalMediaSourceState getState() {

        LocalMediaSourceState stateToReturn = new LocalMediaSourceState();
        stateToReturn.playbackState = new PlaybackState();
        stateToReturn.playbackState.state = getSourcePlaybackState();
        stateToReturn.playbackState.supportedOperations = getSupportedPlaybackOperations();
        stateToReturn.sessionState = new SessionState();
        stateToReturn.sessionState.supportedContentSelectors = getSupportedContentSelectors();
        return stateToReturn;
    }

    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[0];
    }

    protected ContentSelector[] getSupportedContentSelectors() {
        return new ContentSelector[0];
    }

    protected String getSourcePlaybackState() {
        return "IDLE";
    }

    protected void setPlaybackState( String state) {
    }

    private static class LocalMediaSourceSpeaker extends Speaker {
        private SeekBar mVolumeControl;
        private TextView mMuteButton;
        private byte mVolume = 50;
        private boolean mIsMuted = false;

        LocalMediaSourceSpeaker() {}

        @Override
        public boolean setVolume( byte volume ) {
            if(mVolume == volume)
                return true;
            Log.d( sTag, String.format( "Handling setVolume(%s)", volume ) );
            mVolume = volume;
            if ( mIsMuted ) {

//                mPlayer.setVolume( 0 );
//                updateUIVolume( ( byte ) 0 );
            } else {
                float channelVolume = volume / 100f;
//                mPlayer.setVolume( channelVolume );
//                updateUIVolume( volume );
            }
            return true;
        }

        @Override
        public boolean adjustVolume( byte value ) {
            return setVolume( ( byte ) ( mVolume + value ) );
        }

        @Override
        public byte getVolume() {
            if ( mIsMuted ) return 0;
            else return mVolume;
        }

        @Override
        public boolean setMute( boolean mute ) {
            if ( mute && !mIsMuted ) {
                Log.d( sTag, String.format( "Handling mute" ) );
                // handle setting mute for local sources
            } else if ( !mute && mIsMuted ) {
                Log.d( sTag, String.format( "Handling unmute" ) );
                // handle setting mute for local sources
            }

            mIsMuted = mute;
            if ( mute ) {
                // handle setting volume for local sources
            } else {
                // handle setting volume for local sources
            }
            return true;
        }

        @Override
        public boolean isMuted() {
            return mIsMuted;
        }


    }
}
