package com.amazon.sampleapp.impl.LocalMediaSource;

import android.content.Context;
import android.util.Log;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.LocalMediaSource;
import com.amazon.aace.alexa.MediaPlayer;
import com.amazon.aace.alexa.Speaker;
import com.amazon.maccandroid.model.PlayBackStateFields;
import com.amazon.maccandroid.model.state.MediaAppMetaData;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;


public class CDLocalMediaSource extends LocalMediaSource {

    private static final String sTag = "LocalMediaSource";

    private final LoggerHandler mLogger;

    private boolean m_authorized = false;

    public CDLocalMediaSource (Context context, LoggerHandler logger, Source cd ) {
        super(cd, new CDSpeaker());
        mLogger = logger;
    }

    @Override
    public boolean authorize( boolean authorized ) {
        String authStr = authorized ? "authorized" : "not authorized";
        mLogger.postInfo( sTag, String.format( "Mock Local Source player %s", authStr) );
        return m_authorized = authorized;
    }

    @Override
    public boolean play( String payload ) {
        if ( m_authorized ) {
            mLogger.postInfo( sTag, String.format( "playing local source with payload: %s", payload));
            setFocus();
            return true;
        } else {
            mLogger.postInfo(sTag, "Cannot play local source, not authorized");
            return false;
        }
    }

    @Override
    public boolean playControl( PlayControlType controlType ) {
        if ( m_authorized ) {
            mLogger.postInfo( sTag, String.format( "play control local source type: %s", controlType.toString()));
            setFocus();
            return true;
        } else {
            mLogger.postInfo(sTag, "Cannot play control on local source, not authorized");
            return false;
        }
    }

    @Override
    public boolean seek( long offset ) {
        if ( m_authorized ) {
            mLogger.postInfo( sTag, String.format( "seeking local source with offset: %s", offset));
            return true;
        } else {
            mLogger.postInfo(sTag, "Cannot seek on local source, not authorized");
            return false;
        }
    }

    @Override
    public boolean adjustSeek( long deltaOffset ) {
        if ( m_authorized ) {
            mLogger.postInfo( sTag, String.format( "adjusting seek local source with delta offset: %s", deltaOffset));
            return true;
        } else {
            mLogger.postInfo(sTag, "Cannot adjust seek on local source, not authorized");
            return false;
        }
    }

    @Override
    public LocalMediaSourceState getState() {
        LocalMediaSourceState stateToReturn = new LocalMediaSourceState();
        stateToReturn.playbackState = new PlaybackState();
        stateToReturn.playbackState.state = "IDLE";
        stateToReturn.playbackState.supportedOperations = new SupportedPlaybackOperation[]{ SupportedPlaybackOperation.PLAY, SupportedPlaybackOperation.PAUSE, SupportedPlaybackOperation.STOP };
        stateToReturn.playbackState.trackOffset = 0;
        stateToReturn.playbackState.shuffleEnabled = false;
        stateToReturn.playbackState.repeatEnabled = false;
        stateToReturn.playbackState.favorites = Favorites.NOT_RATED;
        stateToReturn.playbackState.type = "ExternalMediaPlayerMusicItem";
        stateToReturn.playbackState.playbackSource = "mock playbackSource";
        stateToReturn.playbackState.playbackSourceId = "mock playbackSourceId";
        stateToReturn.playbackState.trackName = "mock trackName";
        stateToReturn.playbackState.trackId = "";
        stateToReturn.playbackState.trackNumber = "mock trackNumber";
        stateToReturn.playbackState.artistName = "mock artistName";
        stateToReturn.playbackState.artistId = "";
        stateToReturn.playbackState.albumName = "mock albumName";
        stateToReturn.playbackState.albumId = "";
        stateToReturn.playbackState.mediaProvider = "local CD player";
        stateToReturn.playbackState.mediaType = MediaType.TRACK;
        stateToReturn.playbackState.duration = 60000;
        stateToReturn.sessionState = new SessionState();
        stateToReturn.sessionState.endpointId = "localId";
        stateToReturn.sessionState.loggedIn = false;
        stateToReturn.sessionState.userName = "";
        stateToReturn.sessionState.isGuest = false;
        stateToReturn.sessionState.launched = false;
        stateToReturn.sessionState.active = false;
        stateToReturn.sessionState.accessToken = "";
        stateToReturn.sessionState.tokenRefreshInterval = 0;
        stateToReturn.sessionState.playerCookie = "mock playerCookie";
        stateToReturn.sessionState.spiVersion = "1.0";
        //mLogger.postInfo(sTag, "LocalMediaSource GETSTATE called");
        return stateToReturn;
    }


    // Speaker

    private static class CDSpeaker extends Speaker {
        private SeekBar mVolumeControl;
        private TextView mMuteButton;
        private byte mVolume = 50;
        private boolean mIsMuted = false;
        private String mName = "CD Speaker";

        CDSpeaker() {
            super();
        }

        @Override
        public boolean setVolume( byte volume ) {
            if(mVolume == volume)
                return true;
            Log.d( sTag, String.format( "(%s) Handling setVolume(%s)", mName, volume ) );
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
                Log.d( sTag, String.format( "Handling mute (%s)", mName ) );
                //updateMuteButton( mute );
            } else if ( !mute && mIsMuted ) {
                Log.d( sTag, String.format( "Handling unmute (%s)", mName ) );
                //updateMuteButton( mute );
            }

            mIsMuted = mute;
            if ( mute ) {
//                mPlayer.setVolume( 0 );
//                updateUIVolume( ( byte ) 0 );
            } else {
//                mPlayer.setVolume( mVolume / 100f );
//                updateUIVolume( mVolume );
            }
            return true;
        }

        @Override
        public boolean isMuted() {
            return mIsMuted;
        }


    }
}
