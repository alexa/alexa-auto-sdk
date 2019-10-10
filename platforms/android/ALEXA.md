## Alexa API

### Overview

The Alexa Auto Alexa API provides interfaces for standard Alexa features. For an Android project you should consider creating a class per interface you wish to handle.


### Handling Alexa State Changes

The Alexa Auto SDK manages internal state information for Alexa and provides an interface for developers to handle Alexa state changes in their platform. To implement a custom handler for Alexa state changes, the `AlexaClient` class should be extended:

```
public class AlexaClientHandler extends AlexaClient
{
	@Override
	public dialogStateChanged( DialogState state ) {
        // handle dialog state change!
    }
    @Override
    public authStateChanged( AuthState state, AuthError error ) {
        // handle auth state change!
    }
    @Override
    public connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) {
        // handle connection status change!
    }
}
...
```

### Handling Authorization

It is the responsibility of the platform implementation to provide an authorization method for establishing a connection to AVS. The Alexa Auto SDK provides an interface to handle authorization state changes and storing context.

To implement the handler for authorization, the `AuthProvider` class should be extended:

```
public class AuthProviderHandler extends AuthProvider
{
	@Override
	public String getAuthToken()
	{
    	//return the currently obtained auth refresh token
	...
	@Override
	public AuthState getAuthState() {
		//return the current auth state
	}
	...
		//notify the engine that a valid refresh token has been obtained
		authStateChange( AuthProvider.AuthState.REFRESHED, AuthProvider.AuthError.NO_ERROR );
	...
```

### Handling Speech Input

It is the responsibility of the platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to the platform developer. The default platform implementation provides methods for writing raw PCM data to the Engine's audio input processor, which includes handling wake word recognition and end-of-speech detection.

To implement a custom handler for speech input, the `SpeechRecognizer` class should be extended:

```
public class SpeechRecognizerHandler extends SpeechRecognizer
{    
    @Override
    public boolean startAudioInput() {
        // implement start recording
    }
    @Override
    public boolean stopAudioInput() {
        // implement stop recording
    }
    ...
    	// call the write audio method while reading audio input data
    	write( (byte[]) data, (long) size )
    ...
...
```

The initiation of a speech recognition event must be handled by the platform. The simplest way to invoke the speech recognizer is via its `holdToTalk()` and `tapToTalk()` methods.

For wake word implementations, the following methods should be extended in the handler class:

```
...
    @Override
    public void endOfSpeechDetected() {
        // implement speech end detected after wake word detected, and play correct audio cue
    }
    @Override
    public boolean wakewordDetected( String wakeWord ) {
        // implement wake word detected, and play correct audio cue
    }
...
```

The interface also includes the methods `enableWakewordDetection()`, `disableWakewordDetection()`, and `isWakewordDetectionEnabled()`, for wakeword support.

### Handling Speech Output

The speech synthesizer is responsible for handling Alexa's speech. The `SpeechSynthesizer` class extends the abstract `AudioChannel` class. This is to separate each audio-capable class on the platform while allowing the Engine to control their interactive behaviors. The platform implementation provides a reference to an instance of a *media player* and a *speaker*. (The term "speaker" is used here in the sense of "output device") [**Read more about handling media and volume here**](#handling-media-and-volume).

 To implement a custom handler for speech output the `SpeechSynthesizer` class should be extended:

```
public class SpeechSynthesizerHandler extends SpeechSynthesizer
{
	// passing in AndroidMediaPlayer
	public SpeechSynthesizerHandler( Context context, AndroidMediaPlayer mediaPlayer ) {
		this( context, mediaPlayer, mediaPlayer.getSpeaker() );
    }
  ...
```  

### Handling Audio Player Output

When an audio media stream is received from Alexa it is the responsibility of the platform implementation to play the stream in a platform-specific media player and to notify the Engine when the audio starts and stops playing.

The class is similar to the `SpeechSynthesizer` class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for audio player output the `AudioPlayer` class should be extended:

```
public class AudioPlayerHandler extends AudioPlayer
{
	public AudioPlayerHandler( Context context, AndroidMediaPlayer mediaPlayer ) {
    this( context, mediaPlayer, mediaPlayer.getSpeaker() );
}
```

### Handling Notifications

It is the responsibility of the platform implementation to provide a visual and audio indication to the user when notifications (package shipment notifications, notifications from skills, etc.) are available from Alexa. [**Read more about Notifications here**](https://alexa.design/AVSDevNotifications).

To implement a custom handler for Notifications the `Notifications` class should be extended:

```
public class NotificationsHandler extends Notifications
{
	@Override
	public void setIndicator( IndicatorState state )
	{
		// set your notifications indicator ON/OFF
	...
```

### Handling Alerts

When an alert is received from Alexa it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player and notify the Engine when the audio starts and stops playing. The state of the alert is also made available for the platform to react to. The class is similar to the `SpeechSynthesizer` class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for alerts the `Alerts` class should be extended:

```
public class AlertsHandler extends Alerts
{
	// pass in a media player
	public AlertsHandler( Context context, LoggerHandler logger, AndroidMediaPlayer mediaPlayer ) {
        this( context, logger, mediaPlayer, mediaPlayer.getSpeaker() );
    }
	...
	@Override
	public void alertStateChanged( String alertToken, AlertState state, String reason ) {
		//handle alert state change
	}
	...
	@Override
	public void alertCreated( String alertToken, String detailedInfo ) {
		//handle the alert detailed info when alert is created (optional)
		/*
		* JSON string detailedInfo :
		* {
		*	 "time" : <String>
		*	 "type" : <String>
		*	 "label" : <String>
		* }
		*/
	}
	...
	@Override
	public void alertDeleted( String alertToken ) {
		//handle the alert when alert is deleted (optional)
	}
```

For local Alerts control, the methods `localStop` (stop current playing alert), and `removeAllAlerts` (remove all locally stored alerts) should be used.

### Handling Media and Volume

When audio data is received from Alexa it is the responsibility of the platform implementation to read the data from the Engine and play it using a platform-specific media player and speaker. It is also the responsibility of the platform implementation to notify the Engine when the speech audio actually begins playing in the media player and when it is finished. Alexa will notify the Engine that the platform should play/stop/pause/resume playing the speech or other audio in which case the platform implementation must play/stop/pause/resume the media player playback. The platform should extend both `MediaPlayer` and `Speaker` and implement the platform-specific functionality. These classes will then be instantiated by other platform interfaces (i.e. `Alerts`, `SpeechSynthesizer`, `AudioPlayer`) as separate audio channels.

To implement a custom handler for MediaPlayer the `MediaPlayer` class should be extended:

```
//extending the Alexa Auto SDK Media Player using the Android MediaPlayer
public class AndroidMediaPlayer extends com.amazon.aace.alexa.MediaPlayer implements android.media.MediaPlayer.OnCompletionListener, android.media.MediaPlayer.OnErrorListener
{
	...
	@Override
    public boolean prepare()
    {
    	//handle preparing for audio
	...
	@Override
    public boolean prepare( String url )
    {
    	//handle preparing to play a url
	...
	@Override
    public boolean play()
    {
    	//handle playing audio
    	...
    	//inform engine playback started
    	playbackStarted();
	...
	@Override
    public boolean pause()
    {
    	//handle pausing the audio
    	...
    	//inform engine playback paused
    	playbackPaused();
	...
	@Override
    public boolean resume()
    {
    	//handle resuming the audio
    	...
    	//inform engine playback resumed
    	playbackResumed();
	...
	@Override
    public boolean stop()
    {
    	//handle stopping audio
    	...
    	//inform engine playback stopped
    	playbackStopped();
	...
	@Override
    public long getPosition()
    {
    	//handle getting the current position of the audio
	...
	@Override
    public long setPosition()
    {
    	//handle setting the current position of the audio
	...
	@Override
    public void onCompletion( android.media.MediaPlayer mediaPlayer )
    {
    	//handle media completing
    	if( isRepeating() )
		{
			//handle repeating audio condition
			...
		} else {
			//inform engine playback finished
			playbackFinished();
	...
	@Override
    public boolean onError( android.media.MediaPlayer mediaPlayer, int what, int extra )
    {
       	...
       	//inform engine of playback error
       	playbackError(...);
    }


```

The platform implementation's Speaker class will likely need a reference to the media player class. For an Android implementation, the Speaker can be subclassed in the MediaPlayer for example.

To implement a custom handler for Speaker, the `Speaker` class should be extended:

```
...
	private class SpeakerHandler extends Speaker
	{
		...
		@Override
		public boolean setVolume( byte volume )
		{
			//handle setting the volume
		...
		@Override
		public boolean adjustVolume( byte value ) {
			//handle ajdusting the volume by the value
		...
		@Override
		public byte getVolume() {
			//return the volume
		...
		@Override
		public boolean setMute( boolean mute )
		{
			//handle set mute
		...
		@Override
		public boolean isMuted() {
			//return mute state
		...
```

### Handling GUI Templates

When template info is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the info that is received from Alexa. There are two template flavors: [Templates](https://alexa.design/DevDocRenderTemplate) and [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo). You can view sample JSON payloads from these AVS documentation links as well.

In case of lists, it is the responsibility of the platform to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of reading the first five elements it will prompt the user to see if they want to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

To implement a custom handler for GUI templates, the `TemplateRuntime` class should be extended:

```
public class TemplateRuntimeHandler extends TemplateRuntime
{
	...
	@Override
	public void renderTemplate( String payload )
	{
		//handle JSON string payload
	...
	@Override
	public void renderPlayerInfo( String payload )
	{
		//handle JSON string payload
	...
	@Override
    public void clearTemplate() {
        // Handle dismissing display card here
    ...
    @Override
    public void clearPlayerInfo() {
        // Handle clearing player info here
```

### Handling Playback Controller Events

The Engine provides methods for notifying it of playback controller events. If the platform implements playback control features, it must inform the Engine.

#### PlayerInfo Only controls

There are some playback controls, which are specific to the PlayerInfo template. This is for GUI implementations which use the PlayerInfo template as a reference for their GUI displays. The controls available, for a given service, come down with the playerInfo template. For toggles, the synced state is also provided by the PlayerInfo template.

* Buttons
	- Skip Forward ( service defined scrub forward )
	- Skip Backward ( service defined scrub backward )
* Toggles
	- Shuffle ( toggle shuffle songs )
	- Loop ( toggle playlist looping )
	- Repeat ( toggle repeat current media once )
	- Thumbs Up ( toggle thumbs up state )
	- Thumbs Down ( toggle thumbs down state )

To implement a custom handler for the playback controller, the `PlaybackController` class should be extended:

```
public class PlaybackControllerHandler extends PlaybackController
{
	@Override
    public void playButtonPressed()
    {
		//notify the engine the play button was pressed
		buttonPressed(PlaybackButton.PLAY);
	...
		//notify the engine the pause button was pressed
		buttonPressed(PlaybackButton.PAUSE);
	...
	// after displaying controls given by PlayerInfo
	public void skipForwardPressed()
	{
		//notify the engine that the skip forward button was pressed
		buttonPressed(PlaybackButton.SKIP_FORWARD);
	...

	...
		// toggle controls state is given by PlayerInfo
		togglePressed(PlaybackButton.SHUFFLE, true); // should be called with opposing state from the PlayerInfo template

```

### Handling equalizer control

The Equalizer Controller enables Alexa voice control of the device's audio equalizer settings, which includes making gain level adjustments to any of the supported frequency bands ("BASS", "MIDRANGE", and/or "TREBLE") using the device's onboard audio processing. 

The platform implementation is responsible for the following:
* Determining how each supported band affects the audio
* Mapping Alexa's equalizer bands to the bands supported on the device, if they do not directly correspond
* Scaling Alexa's level values as necessary so that each step corresponds to one decibel of amplitude gain on the device
* Applying equalization to only selected portions of the audio output so that Alexa's speech, alarms, etc. will not be affected
* Persisting settings across power cycles

The Equalizer Controller is configurable to the device's capabilities. See `com.amazon.aace.alexa.config.AlexaConfiguration.createEqualizerControllerConfig` for details on configuring the supported bands, default state, and decibel adjustment range.

To implement a custom handler for Equalizer Controller the `com.amazon.aace.alexa.EqualizerController` class should be extended:

```
public class EqualizerControllerHandler extends EqualizerController {
	@Override
	public void setBandLevels( EqualizerBandLevel[] bandLevels ) {
		// Handle performing audio equalization on the device
		// according to the provided band dB level settings
		
		// This invocation may come from "Alexa, reset bass", 
		// "Alexa, reset my equalizer", "Alexa, increase treble", etc.
	}

	@Override
	public EqualizerBandLevel[] getBandLevels() {
		// Return the current band level settings on the device
		return mCurrentBandLevels;
	}

	...
}
...

// Configure the Engine
// For example, 2 supported bands with amplitude gains ranging from -8dB to +8dB, each with a default of 0dB
EqualizerBand[] bands = new EqualizerBand[]{ EqualizerBand.BASS, EqualizerBand.TREBLE };
EqualizerBandLevel[] defaults = new EqualizerBandLevel[]{ 
	new EqualizerBandLevel( EqualizerBand.BASS, 0 ), 
	new EqualizerBandLevel( EqualizerBand.TREBLE, 0 )};
EngineConfiguration eqConfig = AlexaConfiguration.createEqualizerControllerConfig( bands, -8, 8, defaults );

mEngine.configure( new EngineConfiguration[]{
	// other config objects,
	eqConfig,
	...
});

...

// Register the platform interface implementation with the Engine
EqualizerController mEqController = new EqualizerControllerHandler();
mEngine.registerPlatformInterface( mEqController );

...

// If level changes are adjusted using local on-device controls, call inherited methods to notify the Engine:

// To set a band to an absolute gain level in decibels
EqualizerBandLevel[] settings = new EqualizerBandLevel[]{ 
	new EqualizerBandLevel( EqualizerBand.BASS, 4 )}; // Sets bass amplitude to +4dB
mEqController.localSetBandLevels( settings );

// To make a relative adjustment to level settings
EqualizerBandLevel[] adjustments = new EqualizerBandLevel[]{ 
	new EqualizerBandLevel( EqualizerBand.BASS, -2 )}; // Decreases bass gain by 2dB
mEqController.localAdjustBandLevels( adjustments );

// To reset gain levels to the configured defaults (usually 0dB)
EqualizerBand[] bands = new EqualizerBand[]{ EqualizerBand.BASS, EqualizerBand.TREBLE }; // Resets bass and treble bands
mEqController.localResetBands( bands );
```


### Handling External Media Adapter with MACCAndroidClient  

The External Media Adapter interface allows the platform to declare and integrate with an external media source. In our MACC Player example, we handle the registration of the MACC Client callbacks for discovery and authorization.

The following is an example of registering the external media interface:

```
mEngine.registerPlatformInterface( new MACCPlayer( this,  mLogger ) );
```

To implement the MACC player handler, the `ExternalMediaAdapter` class should be extended:

```
public class MACCPlayer extends ExternalMediaAdapter
{
    ...
    private final MACCAndroidClient mClient; // the MACC client
    ...
    public MACCPlayer( Context context, LoggerHandler logger) {
        super(new MACCSpeaker());
        mClient = new MACCAndroidClient(mContext);
        mClient.registerCallback(mMACCAndroidClientCallback);
        ...
``` 

After registering the platform we make a call to `initAndRunDiscovery()`. This initializes and tells the MACC client to discover all MACC applications available on the device.

```
public void runDiscovery() {
    mHandler.postDelayed(new Runnable() {
        @Override
        public void run() {
            mClient.initAndRunDiscovery();
        }
    }, 100);
```    

In our `MACCAndroidClientCallback` class `onPlayerDiscovered()` we call `reportDiscoveredPlayers()` with the discovered players array populated according to the spec. This will notify the cloud of the discovered players.

```
private final MACCAndroidClientCallback mMACCAndroidClientCallback = new MACCAndroidClientCallback() {
    ...
    @Override
    public void onPlayerDiscovered(List<DiscoveredPlayer> list) {
        DiscoveredPlayerInfo[] discoveredPlayers = new DiscoveredPlayerInfo[list.size()];
        //populate
        ...
        //call to interface
        reportDiscoveredPlayers(discoveredPlayers);
```

The cloud will confirm the players by invoking the authorize method. Here we submit the authorized players list back to the MACCClient.

```
    @Override
    public boolean authorize(AuthorizedPlayerInfo[] authorizedPlayers) {
        ...
        mClient.onAuthorizedPlayers(mAuthorizedPlayers);
```

Once the discovery, and authorization has been completed, the MACCClient callback will report a player event with at least `SessionsStarted` (this only happens once during lifecycle). Depending on the state of the MACC external app, focus should be set or not. We can also show the current media information if the external media app is logged in or playing on a separate device for example. Two interface methods should be called whenever the MACC client calls this player event. The `playerEvent()` method will inform the cloud of the player event to stay in sync. `playerError()` should be handled in the same way.

The `playerId` will be static and will be associated with one unique MACC application(e.g. Spotify).

```
    @Override
    public void onPlayerEvent(String playerId, Set<PlayerEvents> playerEvents, String skillToken, UUID playbackSessionId) {
    ...
        playerEvent(playerId, event.getName());
    ...
    @Override
    public void onError(String errorName, int errorCode, boolean fatal, String playerId, UUID playbackSessionId) {
        playerError(playerId, errorName, errorCode, "none", fatal);
    ...
```

The `loginComplete()` and `logoutComplete()` methods should be called if the application has the ability to listen for external media player login and logout events. This will inform AVS of the logged in state change. 

The `login`, and `logout` callbacks should be implemented to pass the information to the MACC client. 

```
    @Override
    public boolean login(String localPlayerId, String accessToken, String userName, boolean forceLogin, long tokenRefreshInterval) {
        mClient.handleDirective(new LoginDirective(localPlayerId, accessToken, userName, forceLogin, tokenRefreshInterval));
    ...
    @Override
    public boolean logout(String localPlayerId) {
        mClient.handleDirective(new LogoutDirective(localPlayerId));
    ...
```

When an Alexa voice request (i.e. "Play Spotify") occurs, the `play()` method will be invoked. In our example, we `setFocus()` and send the play directive info to the MACC client. `setFocus()` will ensure that the external media player will take priority for playing audio, and responding to playback control events.

```
    @Override
    public boolean play(String localPlayerId, String playContextToken, long index, long offset, boolean preload, Navigation navigation) {
        setFocus(localPlayerId);
        mClient.handleDirective(new PlayDirective(localPlayerId, playContextToken, index, offset, preload, navigation.toString()));
        return true;
```   

Whether through voice or GUI event, the `playControl()` method will be called with the relevant playControlType. Similar to in `play()`, we set the focus, and then pass the directive to the MACC Client.

You may use the `PlaybackController` APIs to control playback of an `ExternalMediaAdapter` implementation like MACCPlayer when it is the player in focus, if needed. This may be useful for scenarios such as when the Spotify app is playing, its own GUI is not displayed for any reason, and you wish to use the same GUI to control Spotify as traditional Alexa-managed music that plays through the `AudioPlayer` interface.

```
    @Override
    public boolean playControl(String localPlayerId, PlayControlType playControlType) {
        setFocus(localPlayerId);
        mClient.handleDirective(new PlayControlDirective(localPlayerId, playControlType.toString()));
        ...
```

`PlayControlType` will be sent corresponding to the player's `supportedOperations`. Below is a list of every `supportedOperations` that an external media player can report.

```
SupportedPlaybackOperation.PLAY,
SupportedPlaybackOperation.PAUSE,
SupportedPlaybackOperation.STOP,
SupportedPlaybackOperation.RESUME,
SupportedPlaybackOperation.PREVIOUS,
SupportedPlaybackOperation.NEXT,
SupportedPlaybackOperation.ENABLE_SHUFFLE,
SupportedPlaybackOperation.DISABLE_SHUFFLE,
SupportedPlaybackOperation.ENABLE_REPEAT_ONE,
SupportedPlaybackOperation.ENABLE_REPEAT,
SupportedPlaybackOperation.DISABLE_REPEAT,
SupportedPlaybackOperation.SEEK,
SupportedPlaybackOperation.ADJUST_SEEK,
SupportedPlaybackOperation.FAVORITE,
SupportedPlaybackOperation.UNFAVORITE,
SupportedPlaybackOperation.FAST_FORWARD,
SupportedPlaybackOperation.REWIND,
SupportedPlaybackOperation.START_OVER
```

The `seek()` and `adjustSeek()` methods will be invoked to seek the currently focused `LocalMediaSource`. These methods will only be used by sources that are capable of seeking. `seek()` is for specifying an absolute offset, whereas `adjustSeek()` if for specifying a relative offset. 

```
    @Override
    public boolean seek(String localPlayerId, long offset) {
        setFocus(localPlayerId);
        mClient.handleDirective(new SeekDirective(localPlayerId, (int) offset));
        ...
    @Override
    public boolean seek(String localPlayerId, long deltaOffset) {
        setFocus(localPlayerId);
        mClient.handleDirective(new AdjustSeekDirective(localPlayerId, (int) deltaOffset));
        ...
```

The External Media Adapter handler should also create its own Speaker implementation. It should handle adjusting volume, and setting mute for the source. It does not need to specify a type.

```
private static class MACCSpeaker extends Speaker {
	...
	MACCSpeaker() {
        super();
    	...
```

For more details on the `Speaker` class see [handling media and volume.](#handling-media-and-volume)

The `getState()` method is called to synchronize the MACC player's state with the cloud. This method is used to maintain correct state during start up, and after every Alexa request. 

We construct the `ExternalMediaAdapterState` object using the data taken from the MACCClient (associated via `localPlayerId`) and return the state information.

```
@Override
public ExternalMediaAdapter.ExternalMediaAdapterState getState(String localPlayerId) {
    ExternalMediaPlayerState state = mClient.getState(localPlayerId);
    ...
    ExternalMediaAdapterState stateToReturn = new ExternalMediaAdapterState();
    stateToReturn.playbackState = new PlaybackState();
    stateToReturn.playbackState.state = state.getMediaAppPlaybackState().getMediaAppMetaData();
    stateToReturn.playbackState.type = "ExternalMediaPlayerMusicItem";
    stateToReturn.playbackState.supportedOperations = getSupportedOperations(state.getMediaAppPlaybackState().getSupportedOperations());
     stateToReturn.playbackState.shuffleEnabled = state.getMediaAppPlaybackState().isShuffleEnabled();
     stateToReturn.playbackState.repeatEnabled = state.getMediaAppPlaybackState().getRepeatMode();
     ...
    stateToReturn.sessionState = new SessionState();
    stateToReturn.sessionState.active = state.getMediaAppSessionState().isActive();
    ...
    stateToReturn.sessionState.spiVersion = state.getMediaAppSessionState().getSpiVersion();   
    return stateToReturn;
```

The following table describes the fields comprising a `LocalMediaSourceState`, which includes two sub-components `PlaybackState`, and `SessionState`.

| State        | Type           | Notes  |
| ------------- |:-------------:| -----:|
| **PlaybackState**      |
| state      | String        |   "IDLE/STOPPED/PLAYING" required |
| supportedOperations | SupportedOperations[] | (see SupportedOperation) required |
| trackOffset      | long  |   optional |
| shuffleEnabled      | boolean       |   required |
| repeatEnabled      | boolean       |   required |
| favorites      | Favorites  | {FAVORITED/UNFAVORITED/NOT_RATE} optional |
| type      | String  |   "ExternalMediaPlayerMusicItem" required |
| playbackSource      | String       |   If available else use local player name. optional|
| playbackSourceId      | String  |   empty |
| trackName      | String   |   If available else use local player name. optional|
| trackId      | String    |   empty |
| trackNumber      | String   |  optional |
| artistName      | String    |  optional |
| artistId      | String   |   empty |
| albumName      | String |   optional |
| albumId      | String |   empty |
| tinyURL      | String |   optional |
| smallURL      | String |   optional |
| mediumURL      | String |   optional |
| largeURL      | String |   optional |
| coverId      | String  |   empty |
| mediaProvider      | String  |   optional |
| mediaType      | MediaType |   {TRACK, PODCAST, STATION, AD, SAMPLE, OTHER} required |
| duration      | long  |   optional |
| **SessionsState** |
| endpointId      | String  |   empty |
| loggedIn      | boolean  |   empty |
| userName      | String  |   empty |
| isGuest      | boolean  |   empty |
| launched      | boolean  |   True if MediaController was successfully connected and MediaControllerCompat.Callback.onSessionDestroyed has not been invoked. |
| active      | boolean  |   Media session state. required  |
| accessToken      | String  |   empty |
| tokenRefreshInterval      | long  |   empty |
| playerCookie      | String  |   A player may declare arbitrary information for itself. optional |
| spiVersion      | String  |   "1.0" required  |

`supportedOperations` should be a list the operations that the external media adapter supports. Below is a list of every `supportedOperations`.

```
SupportedPlaybackOperation.PLAY,
SupportedPlaybackOperation.PAUSE,
SupportedPlaybackOperation.STOP,
SupportedPlaybackOperation.RESUME,
SupportedPlaybackOperation.PREVIOUS,
SupportedPlaybackOperation.NEXT,
SupportedPlaybackOperation.ENABLE_SHUFFLE,
SupportedPlaybackOperation.DISABLE_SHUFFLE,
SupportedPlaybackOperation.ENABLE_REPEAT_ONE,
SupportedPlaybackOperation.ENABLE_REPEAT,
SupportedPlaybackOperation.DISABLE_REPEAT,
SupportedPlaybackOperation.SEEK,
SupportedPlaybackOperation.ADJUST_SEEK,
SupportedPlaybackOperation.FAVORITE,
SupportedPlaybackOperation.UNFAVORITE,
SupportedPlaybackOperation.FAST_FORWARD,
SupportedPlaybackOperation.REWIND,
SupportedPlaybackOperation.START_OVER
```

### Handling Local Media Sources

The `LocalMediaSource` interface allows the platform to register a local media source by type(`BLUETOOTH`, `USB`, `LINE_IN`, `AM_RADIO` etc.). Registering a local media source allows playback control of a source via Alexa (e.g. "Alexa, play the CD player") or via button press through the PlaybackController APIs. It will also enable playback initiation via Alexa by frequency, channel, or preset for relevant source types (e.g. "Alexa, play 98.7 FM")

**NOTE: Local media source control with Alexa is currently supported in the US region only**

The following is an example of registering a CD player local media source using type `Source.COMPACT_DISC`:

```
mEngine.registerPlatformInterface(mCDLocalMediaSource = new CDLocalMediaSource(this, mLogger, Source.COMPACT_DISC);
```

To implement a custom handler for a CD player local media source the `LocalMediaSource` class should be extended:

```
public class CDLocalMediaSource extends LocalMediaSource
{
    public CDLocalMediaSource( Context context, LoggerHandler logger, Source cd ) {
        super( context, logger, cd );
    }
    ...
```    

The `play()` method is called when Alexa invokes play by `ContentSelector` type for a radio local media source (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). The `payload` is a string that depends on the `ContentSelector` type and local media `Source` type. 

```
    @Override
    public boolean play( ContentSelector type, String payload ) {
        setFocus();
        // handle the play for CD player
        return true;
```

This method will not be invoked if a source cannot handle the specified content selection type.

The implementation depends on the local media source; however if the call is handled successfully, `setFocus()` should always be called. This informs the engine that the local player is in focus. 

Content selector type details are as shown below:

| type | example supported payload | supported range | increment |
|------|---------|---|---|
| FREQUENCY(FM) | "98.7" | 88.1 - 107.9 | 0.2 |
| FREQUENCY(AM) | "1050" | 540 - 1700 | 10 |
| FREQUENCY(FM) | "93.7 HD 2" | 88.1 - 107.9, HD 1-3 | 0.2, 1 |
| CHANNEL(SXM) | "1" | 1-999 | 1 |
| PRESET | "2" | 1-99 | 1 | 

The `playControl()` method is called with a `PlayControlType`(e.g. `RESUME`, `PAUSE`, `NEXT`, `SHUFFLE`, `REPEAT` etc.) when either Alexa or the GUI (using the `PlaybackController` APIs) invokes a playback control on the local media source.
 

```
    @Override
    public boolean playControl( PlayControlType controlType ) {
        setFocus();
        // handle the control type appropriately for CD player
        return true;
```

The implementation depends on the local media source; however if the call is handled successfully, `setFocus()` should always be called. This informs the engine that the local player is in focus. 

**NOTE: The `play()` method is used to initiate playback with specified content selection, whereas `playControl(RESUME)` is used to play or resume whatever is already playing.**

The `seek()` and `adjustSeek()` methods will be invoked to seek the currently focused `LocalMediaSource`. These methods will only be used by sources that are capable of seeking. `seek()` is for specifying an absolute offset, whereas `adjustSeek()` if for specifying a relative offset. 

```
    @Override
    public boolean seek( long offset ) {
        // handle seeking CD player
    ...
    @Override
    public boolean adjustSeek( long offset ) {
        // handle adjust seek CD player
```

The Local Media Source handler should also create its own Speaker implementation. It should handle adjusting volume, and setting mute for the source. It does not need to specify a type.

```
private static class LocalMediaSourceSpeaker extends Speaker {
	...
	LocalMediaSourceSpeaker() {
        super();
    	...
```

For more details on the `Speaker` class see [handling media and volume.](#handling-media-and-volume)


The `getState()` method is called to synchronize the local player's state with the cloud. This method is used to maintain correct state during startup and after every Alexa request. All relevant information should be added to the `LocalMediaSourceState` and returned. 

Many fields of the `LocalMediaSourceState` are not required for local media source players. These may be omitted as noted below.

```
    @Override
    public LocalMediaSourceState getState() {
        LocalMediaSourceState stateToReturn = new LocalMediaSourceState();
        stateToReturn.playbackState = new PlaybackState();
        stateToReturn.playbackState.state = "IDLE";
        stateToReturn.playbackState.type = "ExternalMediaPlayerMusicItem";
        stateToReturn.playbackState.supportedOperations = {
            SupportedOperations.PLAY, SupportedOperations.PAUSE, SupportedOperations.STOP
        };
        stateToReturn.sessionState = new SessionState();
        stateToReturn.sessionState.spiVersion = "1.0";   
        return stateToReturn;
```

The following table describes the fields comprising a `LocalMediaSourceState`, which includes two sub-components `PlaybackState` and `SessionState`.

| State        | Type           | Notes  |
| ------------- |:-------------:| -----:|
| **PlaybackState**      |
| state      | String        |   "IDLE/STOPPED/PLAYING" required |
| supportedOperations | SupportedOperations[] | (see SupportedOperations) required |
| trackOffset      | long  |   empty |
| shuffleEnabled      | boolean       |   empty |
| repeatEnabled      | boolean       |   empty |
| favorites      | Favorites  |   {FAVORITED/UNFAVORITED/NOT_RATED} optional  |
| type      | String  |   "ExternalMediaPlayerMusicItem" required |
| playbackSource      | String       |   empty |
| playbackSourceId      | String  |   empty |
| trackName      | String   |   empty |
| trackId      | String    |   empty |
| trackNumber      | String   |  empty |
| artistName      | String    |  empty |
| artistId      | String   |   empty |
| albumName      | String |   empty |
| albumId      | String |   empty |
| tinyURL      | String |   empty |
| smallURL      | String |   empty |
| mediumURL      | String |   empty |
| largeURL      | String |   empty |
| coverId      | String  |   empty |
| mediaProvider      | String  |   empty |
| mediaType      | MediaType |   {TRACK, PODCAST, STATION, AD, SAMPLE, OTHER} optional |
| duration      | long  |   empty |
| **SessionState** |
| endpointId      | String  |   empty |
| loggedIn      | boolean  |   empty |
| userName      | String  |   empty |
| isGuest      | boolean  |   empty |
| launched      | boolean  |   empty |
| active      | boolean  |   empty |
| accessToken      | String  |   empty |
| tokenRefreshInterval      | long  |   empty |
| supportedContentSelectors      | ContentSelector[]  |  A player declares it's supported content selectors (see ContentSelector) optional |
| spiVersion      | String  |   "1.0" required  |

`supportedOperations` should list the operations which the local media source supports. Below is a list of all `supportedOperations`:

```
SupportedPlaybackOperation.PLAY,
SupportedPlaybackOperation.PAUSE,
SupportedPlaybackOperation.STOP,
SupportedPlaybackOperation.RESUME,
SupportedPlaybackOperation.PREVIOUS,
SupportedPlaybackOperation.NEXT,
SupportedPlaybackOperation.ENABLE_SHUFFLE,
SupportedPlaybackOperation.DISABLE_SHUFFLE,
SupportedPlaybackOperation.ENABLE_REPEAT_ONE,
SupportedPlaybackOperation.ENABLE_REPEAT,
SupportedPlaybackOperation.DISABLE_REPEAT,
SupportedPlaybackOperation.SEEK,
SupportedPlaybackOperation.ADJUST_SEEK,
SupportedPlaybackOperation.FAVORITE,
SupportedPlaybackOperation.UNFAVORITE,
SupportedPlaybackOperation.FAST_FORWARD,
SupportedPlaybackOperation.REWIND,
SupportedPlaybackOperation.START_OVER
```

**Note: For local media sources, do not use SupportedPlaybackOperation.RESUME**

`supportedContentSelectors` should list the content selection types the local source can supports. Below is a table of valid pairs.

| Source | supportable `ContentSelector`'s |
|---|---|
| `AM_RADIO` |  `PRESET`, `FREQUENCY` |
| `FM_RADIO` |  `PRESET`, `FREQUENCY` |
| `SIRIUS_XM` |  `PRESET`, `CHANNEL` |

### Handling Global Presets

The Global Preset interface is used to handle "Alexa, play preset \<number\>" utterances. The meaning of the preset `number` passed through `setGlobalPreset()` is determined by the `GlobalPreset` platform implementation registered with the Engine, and the implementation should suit the needs of the vehicle's infotainment system. 

Registering a `GlobalPreset` implementation with the engine is required for Alexa to set presets for any local media `Source` type that may use them (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). 

```
    public GlobalPresetHandler (Context context, LoggerHandler logger) {
    ...
    @Override
    public void setGlobalPreset( int number ) {
        // set global preset
    ...
```	

## Alexa Engine Properties

The Alexa module defines several constants that are used to get and set runtime properties in the Engine. To use these properties call the Engine's `getProperty()` and `setProperty()` methods.

```
// get the current locale setting from the Engine
String locale = m_engine.getProperty( com.amazon.aace.alexa.AlexaProperties.LOCALE );

// set the current locale setting in the Engine
m_engine.setProperty( com.amazon.aace.alexa.AlexaProperties.LOCALE, "en-US" );
```

The following constants are defined in the Alexa module:

<table>
<tr>
<th>Property</th>
<th>Description</th>
</tr>
<tr>
<td>
<code>com.amazon.aace.alexa.AlexaProperties.AVS_ENDPOINT</code>
</td>
<td>The value must be a valid AVS endpoint URL.
</td>
</tr>
<tr>
<td>
<code>com.amazon.aace.alexa.AlexaProperties.WAKEWORD_SUPPORTED</code>
</td>
<td><p>Describes if wake word support is enabled. If wake word is not supported in the Engine, attempts to enable wake word detection by
the <code>SpeechRecognizer</code> will fail.</p>
<p><strong>Note</strong>: This is a read-only property.</p>
</td>
</tr>
<tr>
<td>
<code>com.amazon.aace.alexa.AlexaProperties.FIRMWARE_VERSION</code>
</td>
<td>The firmware version that is reported to AVS. The value must be a positive, signed 32-bit integer represented as a string.
</td>
</tr>
<tr>
<td>
<code>com.amazon.aace.alexa.AlexaProperties.LOCALE</code>
</td>
<td>The current locale setting for AVS. The value should be a valid locale accepted by AVS. Calling <code>Engine::getProperty()</code> with the <code>SUPPORTED_LOCALES</code> property provides the list of supported locales.
</td>
</tr>
<tr>
<td>
<code>com.amazon.aace.alexa.AlexaProperties.SUPPORTED_LOCALES</code></li>
</td>
<td><p>AVS supported locales. The value is a comma-separated list, e.g. "de-DE,en-AU,..."</p>
<p><strong>Note</strong>: This is a read-only property.</p>

</td>
</tr>
</table>

 See the API reference documentation for  [AlexaProperties](./aace/src/main/java/com/amazon/aace/alexa/AlexaProperties.java) for more information.
