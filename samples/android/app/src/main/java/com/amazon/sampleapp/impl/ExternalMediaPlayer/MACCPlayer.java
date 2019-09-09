package com.amazon.sampleapp.impl.ExternalMediaPlayer;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;

import android.util.Log;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.ExternalMediaAdapter;
import com.amazon.aace.alexa.PlaybackController;
import com.amazon.aace.audio.AudioOutput.MutedState;
import com.amazon.maccandroid.MACCAndroidClient;
import com.amazon.maccandroid.MACCAndroidClientCallback;
import com.amazon.maccandroid.model.Directive.AdjustSeekDirective;
import com.amazon.maccandroid.model.Directive.LoginDirective;
import com.amazon.maccandroid.model.Directive.LogoutDirective;
import com.amazon.maccandroid.model.Directive.PlayControlDirective;
import com.amazon.maccandroid.model.Directive.PlayDirective;
import com.amazon.maccandroid.model.Directive.SeekDirective;
import com.amazon.maccandroid.model.PlayBackStateFields;
import com.amazon.maccandroid.model.PlayerEvents;
import com.amazon.maccandroid.model.players.AuthorizedPlayer;
import com.amazon.maccandroid.model.players.DiscoveredPlayer;
import com.amazon.maccandroid.model.state.ExternalMediaPlayerState;
import com.amazon.maccandroid.model.state.MediaAppMetaData;
import com.amazon.maccandroid.model.state.MediaAppPlaybackState;

import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import com.amazon.sampleapp.impl.PlaybackController.PlaybackControllerHandler;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_DISABLE_REPEAT;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_DISABLE_SHUFFLE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_REPEAT;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_REPEAT_ONE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_ENABLE_SHUFFLE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_FASTFORWARD;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_FAVORITE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_NEXT;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_PAUSE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_PLAY;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_PREVIOUS;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_REWIND;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_STARTOVER;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_STOP;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.PLAY_CONTROL_UNFAVORITE;
import static com.amazon.maccandroid.model.APIConstants.Directives.PlayControl.SEEK_CONTROL_SET_SEEK_POSITION;

public class MACCPlayer extends ExternalMediaAdapter {
    private static final String TAG = MACCPlayer.class.getSimpleName();
    private final Activity mActivity;
    public static final int LAST_SCAN_COOLOFF = 15;
    private final Context mContext;
    private final LoggerHandler mLogger;
    private final PlaybackControllerHandler mPlaybackController;
    private final MACCAndroidClient mClient;
    public static final String SPOTIFY_PROVIDER_NAME = "Spotify";

    private long mLastScanTimeInMillis;

    private Runnable mRunDiscoveryRunnable = new Runnable() {
        @Override
        public void run() {
            if (mClient != null) {
                mClient.initAndRunDiscovery();
            }
            mLastScanTimeInMillis = Calendar.getInstance().getTimeInMillis();
        }
    };

    private float mVolume = 0.5f;
    private MutedState mMutedState = MutedState.UNMUTED;

    private final MACCAndroidClientCallback mMACCAndroidClientCallback = new MACCAndroidClientCallback() {
        @Override
        public void onPlayerDiscovered(List<DiscoveredPlayer> list) {
            DiscoveredPlayerInfo[] discoveredPlayers = new DiscoveredPlayerInfo[list.size()];
            for (int i = 0; i < list.size(); i++) {
                discoveredPlayers[i] = new DiscoveredPlayerInfo();
                discoveredPlayers[i].localPlayerId = list.get(i).getLocalPlayerId();
                discoveredPlayers[i].spiVersion = "1.0";
                discoveredPlayers[i].validationData = new String[list.get(i).getValidationData().size()];
                discoveredPlayers[i].validationData = list.get(i).getValidationData().toArray(discoveredPlayers[i].validationData);
                discoveredPlayers[i].validationMethod = list.get(i).getValidationMethod();
                Log.i(TAG, "discoveredPlayer: localPlayerId: " + discoveredPlayers[i].localPlayerId
                        + " | spiVersion: " + discoveredPlayers[i].spiVersion
                        + " | validationData: " + discoveredPlayers[i].validationData
                        + " | validationMethod: " + discoveredPlayers[i].validationMethod);
                for (String validationData : discoveredPlayers[i].validationData) {
                    Log.i(TAG, "validationData: " + validationData);
                }
            }
            reportDiscoveredPlayers(discoveredPlayers);
            Log.i(TAG, "reported players");
        }

        @Override
        public void onError(String errorName, int errorCode, boolean fatal, String playerId, UUID playbackSessionId) {
            Log.i(TAG, "onError: " + errorName + " | errorCode: " + errorCode + " | playerId: " + playerId);
            playerError(playerId, errorName, errorCode, "none", fatal);
        }

        @Override
        public void onPlayerEvent(String playerId, Set<PlayerEvents> playerEvents, String skillToken, UUID playbackSessionId) {

            Log.i(TAG, "onPlayerEvent | events: " + playerEvents);

            MediaAppPlaybackState state = mClient.getState(playerId).getMediaAppPlaybackState();
            MediaAppMetaData metaData = state.getMediaAppMetaData();

            for (PlayerEvents event : playerEvents) {

                playerEvent(playerId, event.getName());

                switch( event ) {
                    case PlaybackStopped:
                    case PlaybackSessionEnded:
                        break;
                    case TrackChanged:
                    case PlaybackStarted:
                        setFocus(playerId);
                        mPlaybackController.setPlayerInfo( metaData.getTrackName(), metaData.getArtist(), SPOTIFY_PROVIDER_NAME );
                        break;
                    case PlaybackSessionStarted:
                        if( state.getPlaybackState() == PlayBackStateFields.State.PLAYING ) { // already playing at startup
                            setFocus(playerId);
                            mPlaybackController.setPlayerInfo( metaData.getTrackName(), metaData.getArtist(), SPOTIFY_PROVIDER_NAME );
                        }
                        break;
                    case PlayModeChanged:
                        if ( mPlaybackController.getProvider().equals( SPOTIFY_PROVIDER_NAME) ) {
                            mPlaybackController.hidePlayerInfoControls(); // reset control view
                            updateControls(state);
                        }
                        break;
                }
            }
        }

        @Override
        public void requestTokenForPlayerId(String localPlayerId) {
            Log.i(TAG,"requestTokenForPlayerId");
            requestToken(localPlayerId);
        }

        @Override
        public void onRemovedPlayer(String localPlayerId) {
            Log.i(TAG, "onRemovedPlayer: " + localPlayerId);
            removeDiscoveredPlayer(localPlayerId);
            scheduleScan();
        }

    };
    private void updateControls( MediaAppPlaybackState state ){
        PlayBackStateFields.repeat repeat = state.getRepeatMode();
        // Set playback controller states on player event

        for( String operation : state.getSupportedOperations() ) {
            Log.i(TAG, operation );
            switch( operation ) {
                case "RESUME" :
                    mPlaybackController.stop(); // If resume is currently supported, UI must be in stopped state
                    break;
                case "PAUSE" :
                case "STOP" :
                    mPlaybackController.start(); // If pause/stop is currently supported, UI must be in started state
                    break;
                case "ENABLE_REPEAT" :
                    //if not set to repeat once
                    if( repeat != PlayBackStateFields.repeat.ONE_REPEATED ) {
                        boolean enabled = repeat == PlayBackStateFields.repeat.REPEATED;
                        mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.LOOP.toString(), true, enabled );
                    }
                    break;
                case "ENABLE_SHUFFLE" :
                    boolean enabled = ( state.isShuffleEnabled() == PlayBackStateFields.shuffle.SHUFFLED );
                    mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.SHUFFLE.toString(), true, enabled );
                    break;
                case "ENABLE_REPEAT_ONE" :
                    if( repeat == PlayBackStateFields.repeat.ONE_REPEATED ) {
                        mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.REPEAT.toString(), true, true);
                    } else mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.REPEAT.toString(), true, false);
                    break;
            }
        }
    }

    private final Handler mHandler;

    public MACCPlayer(Activity activity, LoggerHandler logger, PlaybackControllerHandler playbackControllerHandler) {

        mActivity = activity;
        mContext = activity.getApplicationContext();
        mLogger = logger;
        mPlaybackController = playbackControllerHandler;
        mClient = new MACCAndroidClient(mContext);
        mClient.registerCallback(mMACCAndroidClientCallback);
        mHandler = new Handler();
    }

    public void runDiscovery() {
        Log.i(TAG, "runDiscovery");
        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                mClient.initAndRunDiscovery();
            }
        }, 100);
        mLastScanTimeInMillis = Calendar.getInstance().getTimeInMillis();

    }

    public void scheduleScan() {
        long minutesSinceLastScan = TimeUnit.MILLISECONDS.toMinutes(Calendar
                .getInstance().getTimeInMillis() - mLastScanTimeInMillis);
        Log.i(TAG, "scheduleScan | rescheduling scan as last scan was " +
                minutesSinceLastScan + " minutes ago");
        mHandler.removeCallbacks(mRunDiscoveryRunnable);
        if (minutesSinceLastScan > LAST_SCAN_COOLOFF) {
            Log.i(TAG, "scheduleScan | running scan now as it has surpassed cooloff time");
            mHandler.post(mRunDiscoveryRunnable);
        } else {
            long millisecondDelay = TimeUnit.MINUTES.toMillis(LAST_SCAN_COOLOFF -
                    minutesSinceLastScan);
            Log.i(TAG, "scheduleScan | running scan after " + millisecondDelay +
                    " millisecond delay");
            mHandler.postDelayed(mRunDiscoveryRunnable, millisecondDelay);
        }
    }

    @Override
    public boolean login(String localPlayerId, String accessToken, String userName, boolean forceLogin, long tokenRefreshInterval) {
        mClient.handleDirective(new LoginDirective(localPlayerId, accessToken, userName, forceLogin, tokenRefreshInterval));
        return true;
    }

    @Override
    public boolean logout(String localPlayerId) {
        mClient.handleDirective(new LogoutDirective(localPlayerId));
        return true;
    }

    @Override
    public boolean play(String localPlayerId, String playContextToken, long index, long offset, boolean preload, Navigation navigation) {
        mClient.handleDirective(new PlayDirective(localPlayerId, playContextToken, index, offset, preload, navigation.toString()));
        return true;
    }

    @Override
    public boolean playControl(String localPlayerId, PlayControlType playControlType) {
        mClient.handleDirective(new PlayControlDirective(localPlayerId, playControlType.toString()));
        Log.i(TAG, "PLAYCONTROL: " + playControlType.toString());
        // enforce correct playback controller GUI states
        switch( playControlType ) {
            case ENABLE_REPEAT:
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.LOOP.toString(), true, true );
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.REPEAT.toString(), true, false );
                break;
            case DISABLE_REPEAT:
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.LOOP.toString(), true, false );
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.REPEAT.toString(), true, false );
                break;
            case ENABLE_REPEAT_ONE:
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.REPEAT.toString(), true, true);
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.LOOP.toString(), true, false );
                break;
            case ENABLE_SHUFFLE:
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.SHUFFLE.toString(), true, true );
                break;
            case DISABLE_SHUFFLE:
                mPlaybackController.updateControlToggle( PlaybackController.PlaybackToggle.SHUFFLE.toString(), true, false );
                break;
        }
        return true;
    }

    @Override
    public boolean seek(String localPlayerId, long offset) {
        setFocus(localPlayerId);
        mClient.handleDirective(new SeekDirective(localPlayerId, (int) offset));
        return true;
    }

    @Override
    public boolean adjustSeek(String localPlayerId, long deltaOffset) {
        setFocus(localPlayerId);
        mClient.handleDirective(new AdjustSeekDirective(localPlayerId, (int) deltaOffset));
        return true;
    }

    @Override
    public boolean authorize(AuthorizedPlayerInfo[] authorizedPlayers) {
        Log.i(TAG, "authorized");
        List<AuthorizedPlayer> mAuthorizedPlayers = new ArrayList<>();
        for (AuthorizedPlayerInfo playerInfo : authorizedPlayers) {
            mAuthorizedPlayers.add(new AuthorizedPlayer(playerInfo.localPlayerId, playerInfo.authorized));
        }
        mClient.onAuthorizedPlayers(mAuthorizedPlayers);
        return true;
    }

    @Override
    public boolean getState(String localPlayerId,ExternalMediaAdapter.ExternalMediaAdapterState stateToReturn) {
        ExternalMediaPlayerState state = mClient.getState(localPlayerId);
        if (state == null) {
            Log.i(TAG, "Something went wrong");
            return false;
        }
        MediaAppMetaData metaData = state.getMediaAppPlaybackState().getMediaAppMetaData();
        //Log.i(TAG, "Provider?" + metaData.getMediaProvider() + "source?" + metaData.getPlaybackSource()); //both null
        //ExternalMediaAdapter.ExternalMediaAdapterState stateToReturn = new ExternalMediaAdapterState();
        stateToReturn.playbackState = new PlaybackState();
        stateToReturn.playbackState.state = state.getMediaAppPlaybackState().getPlaybackState() == null ? PlayBackStateFields.State.IDLE.toString() : state.getMediaAppPlaybackState().getPlaybackState().toString();
        stateToReturn.playbackState.supportedOperations = getSupportedOperations(state.getMediaAppPlaybackState().getSupportedOperations());
        stateToReturn.playbackState.trackOffset = state.getMediaAppPlaybackState().getPositionMilliseconds();
        stateToReturn.playbackState.shuffleEnabled = (state.getMediaAppPlaybackState().isShuffleEnabled() == null) ? false : state.getMediaAppPlaybackState().isShuffleEnabled().equals(PlayBackStateFields.shuffle.SHUFFLED);
        stateToReturn.playbackState.repeatEnabled = (state.getMediaAppPlaybackState().getRepeatMode() == null) ? false : !state.getMediaAppPlaybackState().getRepeatMode().equals(PlayBackStateFields.repeat.NOT_REPEATED);
        stateToReturn.playbackState.favorites = getFavorites(state.getMediaAppPlaybackState().getFavourite());
        stateToReturn.playbackState.type = "ExternalMediaPlayerMusicItem";
        stateToReturn.playbackState.playbackSource = state.getMediaAppPlaybackState().getMediaAppMetaData().getPlaybackSource(); //
        stateToReturn.playbackState.playbackSourceId = state.getMediaAppPlaybackState().getMediaAppMetaData().getPlaybackSourceId();
        stateToReturn.playbackState.trackName = state.getMediaAppPlaybackState().getMediaAppMetaData().getTrackName();
        stateToReturn.playbackState.trackId = "";
        stateToReturn.playbackState.trackNumber = state.getMediaAppPlaybackState().getMediaAppMetaData().getTrackNumber();
        stateToReturn.playbackState.artistName = state.getMediaAppPlaybackState().getMediaAppMetaData().getArtist();
        stateToReturn.playbackState.artistId = "";
        stateToReturn.playbackState.albumName = state.getMediaAppPlaybackState().getMediaAppMetaData().getAlbum();
        stateToReturn.playbackState.albumId = "";
        stateToReturn.playbackState.mediaProvider = state.getMediaAppPlaybackState().getMediaAppMetaData().getMediaProvider(); //
        stateToReturn.playbackState.mediaType = getMediaType(state.getMediaAppPlaybackState().getMediaAppMetaData().getMediaType());
        stateToReturn.playbackState.duration = state.getMediaAppPlaybackState().getMediaAppMetaData().getDurationInMilliseconds();
        stateToReturn.sessionState = new SessionState();
        stateToReturn.sessionState.endpointId = state.getMediaAppSessionState().getPlayerid();
        stateToReturn.sessionState.loggedIn = true; //assumption, mechanism not yet included
        stateToReturn.sessionState.userName = "";
        stateToReturn.sessionState.isGuest = false;
        stateToReturn.sessionState.launched = state.getMediaAppSessionState().isLaunched();
        stateToReturn.sessionState.active = state.getMediaAppSessionState().isActive();
        stateToReturn.sessionState.accessToken = "";
        stateToReturn.sessionState.tokenRefreshInterval = 0;
        stateToReturn.sessionState.playerCookie = state.getMediaAppSessionState().getPlayerCookie();
        stateToReturn.sessionState.spiVersion = state.getMediaAppSessionState().getSpiVersion();
        return true;
    }

    private MediaType getMediaType(String mediaType) {
        if (mediaType == null) {
            return null;
        }
        switch(mediaType) {
            case "TRACK":
                return MediaType.TRACK;
        }
        return mediaType.equals("TRACK") ?  MediaType.TRACK : MediaType.AD;
    }

    private Favorites getFavorites(PlayBackStateFields.favorite favourite) {
        //Log.w(TAG,"Favorite="+favourite);
        if (favourite == null) return Favorites.NOT_RATED;

        switch(favourite) {
            case FAVORITED:
                return Favorites.FAVORITED;
            case UNFAVORITED:
                return Favorites.UNFAVORITED;
            case NOT_RATED:
            default:
                return Favorites.NOT_RATED;
        }
    }

    private SupportedPlaybackOperation[] getSupportedOperations(Set<String> supportedOperations) {
        List<SupportedPlaybackOperation> returnList = new ArrayList<>();


        for (String operation : supportedOperations) {
            switch (operation) {
                case PLAY_CONTROL_PLAY:
                    returnList.add(SupportedPlaybackOperation.PLAY);
                case PLAY_CONTROL_PAUSE:
                    returnList.add(SupportedPlaybackOperation.PAUSE);
                    break;
                case PLAY_CONTROL_STOP:
                    returnList.add(SupportedPlaybackOperation.STOP);
                    break;
                case PLAY_CONTROL_STARTOVER:
                    returnList.add(SupportedPlaybackOperation.START_OVER);
                    break;
                case PLAY_CONTROL_PREVIOUS:
                    returnList.add(SupportedPlaybackOperation.PREVIOUS);
                    break;
                case PLAY_CONTROL_NEXT:
                    returnList.add(SupportedPlaybackOperation.NEXT);
                    break;
                case PLAY_CONTROL_REWIND:
                    returnList.add(SupportedPlaybackOperation.REWIND);
                    break;
                case PLAY_CONTROL_FASTFORWARD:
                    returnList.add(SupportedPlaybackOperation.FAST_FORWARD);
                    break;
                case SEEK_CONTROL_SET_SEEK_POSITION:
                    returnList.add(SupportedPlaybackOperation.SEEK);
                    returnList.add(SupportedPlaybackOperation.ADJUST_SEEK);
                    break;
                case PLAY_CONTROL_FAVORITE:
                    returnList.add(SupportedPlaybackOperation.FAVORITE);
                    break;
                case PLAY_CONTROL_UNFAVORITE:
                    returnList.add(SupportedPlaybackOperation.UNFAVORITE);
                    break;
                case PLAY_CONTROL_ENABLE_SHUFFLE:
                    returnList.add(SupportedPlaybackOperation.ENABLE_SHUFFLE);
                    break;
                case PLAY_CONTROL_DISABLE_SHUFFLE:
                    returnList.add(SupportedPlaybackOperation.DISABLE_SHUFFLE);
                    break;
                case PLAY_CONTROL_ENABLE_REPEAT:
                    returnList.add(SupportedPlaybackOperation.ENABLE_REPEAT);
                    break;
                case PLAY_CONTROL_ENABLE_REPEAT_ONE:
                    returnList.add(SupportedPlaybackOperation.ENABLE_REPEAT_ONE);
                    break;
                case PLAY_CONTROL_DISABLE_REPEAT:
                    returnList.add(SupportedPlaybackOperation.DISABLE_REPEAT);
                    break;
            }
        }
        SupportedPlaybackOperation[] suppArr = new SupportedPlaybackOperation[returnList.size()];
        for (int i = 0; i < returnList.size(); i++) {
            suppArr[i] = returnList.get(i);
        }
        return suppArr;
    }

    @Override
    public boolean volumeChanged( float volume ) {
        if( mVolume != volume ) {
            mLogger.postInfo( TAG, String.format( "(%s) Handling volumeChanged(%s)", "MACC", volume ) );
            mVolume = volume;
            if ( mMutedState == MutedState.MUTED ) {
                // mPlayer.setVolume( 0 );
            } else {
                // mPlayer.setVolume( volume );
            }
        }
        return true;
    }

    @Override
    public boolean mutedStateChanged( MutedState state ) {
        if( state != mMutedState ) {
            mLogger.postInfo( TAG, String.format( "Muted state changed (%s) to %s.", "MACC", state ) );
            //mPlayer.setVolume( state == MutedState.MUTED ? 0 : mVolume );
            mMutedState = state;
        }
        return true;
    }
}
