# Alexa Module

The Alexa Auto SDK Alexa module provides interfaces for standard Alexa features. The Engine handles some extra setup and steps to sequence events and handle directives so you can focus on using the provided API to interact with Alexa. You do this by registering platform interface implementations via the Engine object. 

**Table of Contents**

* [Requesting Wake Word Support](#requesting-wake-word-support)
* [Handling Speech Input](#handling-speech-input)
* [Handling Speech Output](#handling-speech-output)
* [Handling Authorization](#handling-authorization)
* [Handling Audio Output](#handling-gui-templates)
* [Handling Alexa Speaker](#handling-alexa-speaker)
* [Handling Audio Player Output](#handling-audio-player-output)
* [Handling Playback Controller Events](#handling-playback-controller-events)
* [Handling Equalizer Control](#handling-equalizer-control)
* [Handling Display Card Templates](#handling-gui-templates)
* [Handling Global Presets](#global-presets)
* [Handling External Media Adapter with MACCAndroidClient](#handling-external-media-adapter)
* [Handling Local Media Sources](#handling-local-media-sources)
* [Handling Notifications](#handling-notifications)
* [Handling Alerts](#handling-alerts)
* [Handling Alexa State Changes](#handling-alexa-state-changes)
* [Alexa Engine Properties](#alexa-engine-properties)

## Requesting Wake Word Support<a id = "requesting-wake-word-support"></a>

If you want to enable wake word support for your Alexa Auto integration, you need to make a request with your Alexa Auto Solution Architect (SA).

There are 3 steps to this process:

1. Let your SA know you want to enable wake word support.
2. Your SA processes your request with the appropriate Alexa teams.
3. You'll receive a single zip file containing the necessary packages, instructions, and scripts.
4. Follow the instructions in the README file to install and build the AmazonLite Wake Word extension.

## Handling Speech Input<a id="handling-speech-input"></a>

It is the responsibility of the `AudioInputProvider` platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to you. An audio playbaplayck noise (earcon) is played whenever speech input is invoked. The playback is handled by whichever audio channel is assigned to the EARCON type [Read more about handling media and volume](#handling-media-and-volume).

To implement a custom handler for speech input, extend the `SpeechRecognizer` class:

```
public class SpeechRecognizerHandler extends SpeechRecognizer {

    @Override
    public boolean wakewordDetected( String wakeWord ) {
        // play audio cue and/or update GUI
        return true;
    }
    ...
    
    @Override
    public void endOfSpeechDetected() {
        // play audio cue and/or update GUI
    }
    ...
    
    // Notify engine of speech recognize event via press-and-hold initiation. 
    // startCapture( Initiator.HOLD_TO_TALK ) is equivalent.
    holdToToalk();
    ...
    
    // Notify the Engine of a speech recognize event press-and-release initiation.
    // startCapture( Initiator.TAP_TO_TALK ) is equivalent.
    tapToTalk();
    ... 
    
    // Notify the Engine that the wakeword engine has detected the wakeword. 
    // Currently the only keyword value supported is "ALEXA".  
    startCapture( Initiator::WAKWORD, 0, 1000, "ALEXA" );
    ...
    
    // Notify the Engine to stop speech recognition. 
    // For example, when hold-to-talk was used and is now being ended by the button release action
    stopCapture();
    ...
        
    // If the implmenetation has configured a wakeword engine, this call enables it. 
    enableWakewordDetection();
    ... 
        
    // If the implmenetation has configured a wakeword engine, this call disables it.
    disableWakewordDetection();
    ...
    
    // To check whether the wakeword engine is enabled
    isWakewordDetectionEnabled();
    ... 
}
```

## Handling Speech Output<a id="handling-speech-output"></a>

The `SpeechSynthesizer` is responsible for handling Alexa's speech. In v2.0.0 and later of the Auto SDK, this interface no longer has any platform-dependent implementation. However, you must still register it to enable the feature. The playback is handled by whichever audio channel is assigned to the TTS type. [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for speech output extend the `SpeechSynthesizer` class:

```
public class SpeechSynthesizerHandler extends SpeechSynthesizer {
    ...
}
...

// Register the platform interface with the Engine
mEngine.registerPlatformInterface( mSpeechSynthesizer = new SpeechSynthesizerHandler() );
```  

## Handling Authorization<a id="handling-authorization"></a>

It is the responsibility of the platform implementation to provide an authorization method for establishing a connection to AVS. The Alexa Auto SDK provides an interface to handle authorization state changes and storing context.

To implement the handler for authorization, extend the `AuthProvider` class:

```
public class AuthProviderHandler extends AuthProvider {

    @Override
    public String getAuthToken() {
        // return the currently obtained access token
    }
	
    @Override
    public AuthState getAuthState() {
        // return the current auth state
	}
	...
	
    // Notify the Engine that the access token has been refreshed
    authStateChange( AuthProvider.AuthState.REFRESHED, AuthProvider.AuthError.NO_ERROR );
	...
}
...

// Register the platform interface with the Engine
mEngine.registerPlatformInterface( mAuthProvider = new AuthProviderHandler() );
```

## Handling Audio Output<a id="handling-media-and-volume"></a>

When audio data is received from Alexa it is the responsibility of the platform implementation to read the data from the Engine and play it using a platform-specific audio output channel. It is also the responsibility of the platform implementation to define how each `AudioOutput` channel is handled. Each `AudioOutput` implementation will handle one or more of the following media types depending on the behavior defined in the `AudioOutputProvider`:

* TTS
* MUSIC
* NOTIFICATION
* ALARM
* EARCON
* COMMUNICATION

`AudioOutput` also has methods to control the volume of the audio channel. These methods allow the Engine to set the volume, either when the user asks Alexa to adjust it or if the Engine internally needs to adjust it during audio focus management. All local volume changes (initiated via button press or system-generated, for example) must be reported to the Engine through the `AlexaSpeaker`platform interface. For further details about the `AudioOutput` and `AudioOutputProvider` platform interfaces, please refer to the [Core module README](../core/README.md).

### Custom Volume Control for Alexa Devices

You can use a custom volume control to support an Alexa device's native input volume range. By default, Alexa supports voice utterances that specify volume values between 0 and 10, but some devices may support a different range (i.e. 0 to 100). By whitelisting your Alexa devices volume range with Amazon for your target platform, you can specify input volume levels per your device's range. Your device's input volume range is then mapped appropriately to the Alexa volume range.

Contact your Alexa Auto Solution Architect (SA) for help with whitelisting. Whitelisting requires the following parameters:

* DeviceTypeID
* Min:
* Max:

This does not impact the range used in the directives to the device. You must continue to use the SDK 0-100 volume range used by `AudioOutput` and `AlexaSpeaker` and map these values to the correct range in your implementation.

## Handling Alexa Speaker<a id="handling-alexa-speaker"></a>

The Alexa service keeps track of two device volume types. One is called `ALEXA_VOLUME`, and the other is `ALERTS_VOLUME`. The `AlexaSpeaker` class should be implemented by the platform to both set the volume and mute state of these two speaker types and allow the user to set the volume and mute state of these two speaker types locally via GUI if applicable. 

```
public class AlexaSpeakerHandler extends AlexaSpeaker {
    ...    
    @Override
    public void speakerSettingsChanged( SpeakerType type, boolean local, byte volume, boolean mute ) {
        // When invoked, set the speaker volume and mute state
    }
    ...
    
    // Notify the Engine of local UI volume setting change
    SpeakerType type = SpeakerType.AVS_VOLUME;
    localSetVolume( type, 100 );
    ...
    
    // Notify the Engine of local UI volume adjustment
    SpeakerType type = SpeakerType.ALERTS_VOLUME; 
    localAdjustVolume( type, 10 );
    ...
    
    // Notify the Engine of local UI mute setting change
    SpeakerType type = SpeakerType.AVS_VOLUME;
    localSetMute( type, true );
```  

## Handling Audio Player Output<a id="handling-audio-player-output"></a>

When an audio media stream is received from Alexa it is the responsibility of the platform implementation to play the stream in a platform-specific media player. The `AudioPlayer` class informs the platform of the changes in player state being tracked by the Engine. This can be used to update the platform GUI.

To implement a custom handler for audio player output, extend the `AudioPlayer` class:

```
public class AudioPlayerHandler extends AudioPlayer {
    ...
    @Override
    public void playerActivityChanged( AudioPlayer.PlayerActivity state) {
        // on state change, update playback control UI
    }
    ...
}
```
## Handling Playback Controller Events<a id="handling-playback-controller-events"></a>

The Engine provides methods to notify it of media playback control events that happen without voice interaction, for example, a "pause" button press. The platform implementation must inform the Engine of these events using the PlaybackController interface any time the user uses on-screen or physical button presses to control media provided by the Engine, such as AudioPlayer source music or ExternalMediaPlayer sources, if applicable.

**Note: ** PlaybackController events that control AudioPlayer report button presses or the equivalent; they do not report changes to the playback state that happen locally first. The Alexa cloud manages the playback queue for AudioPlayer content, so each PlaybackController event can be considered a request for the cloud to act on the user's local request. The result of the request will come as a method invocation on the AudioOutput associated with the channel used for AudioPlayer.

**Note: ** If your implementation needs to stop AudioPlayer media in response to system events, such as audio focus transitions to audio playing outside the scope of Auto SDK, PlaybackController is the interface to use to notify the Engine of such changes. However, expected usage of the interface does not change when using it for this use case.

To implement a custom handler for the playback controller, extend the `PlaybackController` class:

```
public class PlaybackControllerHandler extends PlaybackController {
    @Override
    public void playButtonPressed() {
    	// Notify the engine the play button was pressed
    	buttonPressed(PlaybackButton.PLAY);
    }
    // Other buttons are similar
    ...
	
    // After displaying controls given by PlayerInfo
    public void skipForwardPressed() {
    	// Notify the Engine that the skip forward button was pressed
    	buttonPressed(PlaybackButton.SKIP_FORWARD);
    }
    ...

    // toggle control state is given by PlayerInfo
    togglePressed(PlaybackButton.SHUFFLE, true); // should be called with opposing state from the PlayerInfo template
```
### PlayerInfo Only controls

The Template Runtime's PlayerInfo template specifies the display of some additional controls. This is for GUI implementations that use the PlayerInfo template as a reference for their GUI displays during `AudioPlayer` media playback. The available controls for the active `AudioPlayer` stream are enabled in the payload of the PlayerInfo template and vary by media service provider. When the toggle button is pressed, another RenderPlayerInfo directive is issued with the new toggle state in the payload to synchronize the display. 

* Buttons
	- Skip Forward ( service defined scrub forward )
	- Skip Backward ( service defined scrub backward )
* Toggles
	- Shuffle ( toggle shuffle songs )
	- Loop ( toggle playlist looping )
	- Repeat ( toggle repeat current media once )
	- Thumbs Up ( toggle thumbs up state )
	- Thumbs Down ( toggle thumbs down state )

## Handling Equalizer Control<a id="handling-equalizer-control"></a>

The Equalizer Controller enables Alexa voice control of the device's audio equalizer settings, which includes making gain level adjustments to any of the supported frequency bands ("BASS", "MIDRANGE", and/or "TREBLE") using the device's onboard audio processing. 

The platform implementation is responsible for the following:
* Determining how each supported band affects the audio
* Mapping Alexa's equalizer bands to the bands supported on the device, if they do not directly correspond
* Scaling Alexa's level values as necessary so that each step corresponds to one decibel of amplitude gain on the device
* Applying equalization to only selected portions of the audio output so that Alexa's speech, alarms, etc. will not be affected
* Persisting settings across power cycles

The Equalizer Controller is configurable to the device's capabilities. See `com.amazon.aace.alexa.config.AlexaConfiguration.createEqualizerControllerConfig` for details on configuring the supported bands, default state, and decibel adjustment range.

To implement a custom handler for Equalizer Controller extend the `EqualizerController` class:

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

// Register the platform interface with the Engine
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

## Handling Display Card Templates<a id="handling-gui-templates"></a>

Alexa sends visual metadata (display card templates) for your device to display. When template information is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the information that is received from Alexa. There are two display card template types:

* The [Template](https://alexa.design/DevDocRenderTemplate) type provides visuals associated with a user request to accompany Alexa speech.
* The [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo) type provides visuals associated with media playing through the `AudioPlayer` interface. This includes playback control buttons, which must be used with the `PlaybackController` interface.

To implement a custom handler for GUI templates, extend the `TemplateRuntime` class:

```
public class TemplateRuntimeHandler extends TemplateRuntime {
    ...
    @Override
    public void renderTemplate( String payload ) {
    	// Handle rendering the JSON string payload as a display card
    }
    ...
    
    @Override
    public void renderPlayerInfo( String payload ) {
    	// Handle rendering the JSON string payload as a display card
    }
    ...
    
    @Override
    public void clearTemplate() {
        // Handle dismissing the display card
    }
    ...
    @Override
    public void clearPlayerInfo() {
        // Handle clearing the player info display card here
    }
```
>**Note:** In the case of lists, it is the responsibility of the platform implementation to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of the first five elements, Alexa prompts the user whether or not to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

## Handling Global Presets<a id="global-presets"></a>

Use the Global Preset interface to handle "Alexa, play preset \<number\>" utterances. The meaning of the preset `number` passed through `setGlobalPreset()` is determined by the `GlobalPreset` platform implementation registered with the Engine, and the implementation should suit the needs of the vehicle's infotainment system. 

You must register a `GlobalPreset` implementation with the Engine in order for Alexa to set presets for any local media `Source` type that may use them (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). 

```
...
@Override
public void setGlobalPreset( int number ) {
    // set global preset
...
```	
## Handling External Media Adapter with MACCAndroidClient <a id="handling-external-media-adapter"></a> 

The External Media Adapter interface allows the platform to declare and integrate with an external media source. In the following MACC Player example, we handle the registration of the MACC Client callbacks for discovery and authorization.

The following is an example of registering the external media interface:

```
mEngine.registerPlatformInterface( new MACCPlayer( this,  mLogger ) );
```

To implement the MACC player handler, extend the `ExternalMediaAdapter` class:

```
public class MACCPlayer extends ExternalMediaAdapter {
    private final MACCAndroidClient mClient; // the MACC client
    ...
    
    public MACCPlayer( Context context) {
        mClient = new MACCAndroidClient(mContext);
        mClient.registerCallback(mMACCAndroidClientCallback);
        ...
    }
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

In our `MACCAndroidClientCallback` class `onPlayerDiscovered()` we call `reportDiscoveredPlayers()` with the discovered players array populated according to the spec. This notifies the cloud of the discovered players.

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

The cloud confirms the players by invoking the authorize method. Here we submit the authorized players list back to the MACCClient:

```
@Override
public boolean authorize(AuthorizedPlayerInfo[] authorizedPlayers) {
    ...
    mClient.onAuthorizedPlayers(mAuthorizedPlayers);
```

Once the discovery and authorization has been completed, the MACCClient callback reports a player event with at least `SessionsStarted` (this only happens once during lifecycle). Depending on the state of the MACC external app, focus should be set or not. We can also show the current media information if the external media app is logged in or playing on a separate device. You should call two interface methods whenever the MACC client calls this player event. The `playerEvent()` method informs the cloud of the player event to stay in sync. `playerError()` should be handled in the same way.

The `playerId` is static and is associated with one unique MACC application (e.g. Spotify).

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

You should call the `loginComplete()` and `logoutComplete()` methods if the application has the ability to listen for external media player login and logout events. This will inform AVS of the logged in state change. 

You should implement the `login`, and `logout` callbacks to pass the information to the MACC client. 

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

When an Alexa voice request (i.e. "Play Spotify") occurs, the `play()` method is invoked. In our example, we `setFocus()` and send the play directive info to the MACC client. `setFocus()` will ensure that the external media player will take priority for playing audio, and responding to playback control events.

```
@Override
public boolean play(String localPlayerId, String playContextToken, long index, long offset, boolean preload, Navigation navigation) {
    setFocus(localPlayerId);
    mClient.handleDirective(new PlayDirective(localPlayerId, playContextToken, index, offset, preload, navigation.toString()));
    return true;
```   

Whether through voice or GUI event, the `playControl()` method is called with the relevant playControlType. Similar to `play()`, we set the focus, and then pass the directive to the MACC Client.

You can use the `PlaybackController` APIs to control playback of an `ExternalMediaAdapter` implementation such as MACCPlayer when it is the player in focus, if needed. This may be useful for scenarios such as when the Spotify app is playing, its own GUI is not displayed for any reason, and you wish to use the same GUI to control Spotify as traditional Alexa-managed music that plays through the `AudioPlayer` interface.

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

The `seek()` and `adjustSeek()` methods are invoked to seek the currently focused `LocalMediaSource`. These methods are used by sources that are capable of seeking. `seek()` is for specifying an absolute offset, whereas `adjustSeek()` is for specifying a relative offset. 

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

The `volumeChanged()` and `mutedStateChanged()` methods are invoked to change the volume and mute state of the currently focused MACC player. `volumeChanged()` specifies the new volume. `mutedStateChanged()` specifies the new `MutedState`.

```
@Override
public boolean volumeChanged( float volume ) {
    // handle volume change
...
@Override
public boolean mutedStateChanged( MutedState state ) {
    // handle setting mute state
...
```

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

The following table describes the fields comprising a `LocalMediaSourceState`, which includes two sub-components: `PlaybackState`, and `SessionState`.

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
## Handling Local Media Sources<a id ="handling-local-media-sources"></a>

The `LocalMediaSource` interface allows the platform to register a local media source by type(`BLUETOOTH`, `USB`, `LINE_IN`, `AM_RADIO` etc.). Registering a local media source allows playback control of a source via Alexa (e.g. "Alexa, play the CD player") or via button press through through the PlaybackController interface, if desired. It will also enable playback initiation via Alexa by frequency, channel, or preset for relevant source types (e.g. "Alexa, play 98.7 FM")

>**NOTE:** Local media source control with Alexa is currently supported in the US region only.

The following is an example of registering a CD player local media source using type `Source.COMPACT_DISC`:

```
mEngine.registerPlatformInterface(mCDLocalMediaSource = new CDLocalMediaSource(this, mLogger, Source.COMPACT_DISC);
```

To implement a custom handler for a CD player local media source extend the `LocalMediaSource` class:

```
public class CDLocalMediaSource extends LocalMediaSource {
    ...
```    

The `play()` method is called when Alexa invokes play by `ContentSelector` type for a radio local media source (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). The `payload` is a string that depends on the `ContentSelector` type and local media `Source` type. 

```
@Override
public boolean play( ContentSelector type, String payload ) {
    setFocus();
    // handle the play for CD player
    return true;
}
```

This method will not be invoked if a source cannot handle the specified content selection type.

The implementation depends on the local media source; however, if the call is handled successfully, `setFocus()` should always be called. This informs the Engine that the local player is in focus. 

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
}
```

The implementation depends on the local media source; however, if the call is handled successfully, `setFocus()` should always be called. This informs the Engine that the local player is in focus. 

>**NOTE:** The `play()` method is used to initiate playback with specified content selection, whereas `playControl(RESUME)` is used to play or resume whatever is already playing.

The `seek()` and `adjustSeek()` methods are invoked to seek the currently focused `LocalMediaSource`. These methods are only used by sources that are capable of seeking. `seek()` is for specifying an absolute offset, whereas `adjustSeek()` is for specifying a relative offset. 

```
@Override
public boolean seek( long offset ) {
    // handle seeking CD player
}
...
@Override
public boolean adjustSeek( long offset ) {
    // handle adjust seek CD player
}
```

The `volumeChanged()` and `mutedStateChanged()` methods are invoked to change the volume and mute state of the currently focused local media player. `volumeChanged()` specifies the new volume. `mutedStateChanged()` specifies the new `MutedState`.

```
@Override
public boolean volumeChanged( float volume ) {
    // handle volume change
}
...
@Override
public boolean mutedStateChanged( MutedState state ) {
    // handle setting mute state
}
...
```

The `getState()` method is called to synchronize the local player's state with the cloud. This method is used to maintain correct state during startup and with every Alexa request. All relevant information should be added to the `LocalMediaSourceState` and returned. 

Many fields of the `LocalMediaSourceState` are not required for local media source players. You should omit these, as noted below.

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

The following table describes the fields comprising a `LocalMediaSourceState`, which includes two sub-components: `PlaybackState` and `SessionState`.

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

>**Note:** For local media sources, do not use `SupportedPlaybackOperation.RESUME`.

`supportedContentSelectors` should list the content selection types the local source supports. Below is a table of valid pairs.

| Source | supportable `ContentSelector`'s |
|---|---|
| `AM_RADIO` |  `PRESET`, `FREQUENCY` |
| `FM_RADIO` |  `PRESET`, `FREQUENCY` |
| `SIRIUS_XM` |  `PRESET`, `CHANNEL` |

## Handling Notifications<a id="handling-notifications"></a>

It is the responsibility of the platform implementation to provide a visual indication to the user when notifications (for example, package shipment notifications, notifications from skills, etc.) are available from Alexa. [Read more about Notifications here](https://alexa.design/AVSDevNotifications). The Engine uses the registered Notifications implementation to notify you when a notification indicator should be displayed or removed. It does not give any information about the notifications. Audio playback for the notification is handled by whichever audio channel is assigned to the NOTIFICATION type [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for Notifications extend the `Notifications` class:

```
public class NotificationsHandler extends Notifications {
	@Override
	public void setIndicator( IndicatorState state ) {
		// set your notifications indicator ON/OFF
	}
```

## Handling Alerts<a id="handling-alerts"></a>

When an alert is received from Alexa, it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player. The state of the alert is also made available for the platform to react to. The playback is handled by whichever audio channel is assigned to the `ALERT` type [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for alerts extend the `Alerts` class:

```
public class AlertsHandler extends Alerts {
	...
	@Override
	public void alertStateChanged( String alertToken, AlertState state, String reason ) {
		// handle alert state change
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
		// handle the alert when alert is deleted (optional)
	}
```

For local Alerts control, you should use the the methods `localStop` (stop current playing alert), and `removeAllAlerts` (remove all locally stored alerts).

## Handling Alexa State Changes<a id="handling-alexa-state-changes"></a>

The Alexa Auto SDK manages internal state information for Alexa and provides an interface for you to handle Alexa state changes in your platform. The information provided by method invocations of this class might be useful, for instance, to enable or disable certain functionality or trigger state changes in you Alexa attention state UI (such as Voice Chrome). To implement a custom handler for Alexa state changes, extend the `AlexaClient` class:

```
public class AlexaClientHandler extends AlexaClient {
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

## Alexa Engine Properties<a id="alexa-engine-properties"></a>

The Alexa module defines several constants that are used to get and set runtime properties in the Engine. To use these properties call the Engine's `getProperty()` and `setProperty()` methods.

```
// get the current locale setting from the Engine
String locale = mEngine.getProperty( com.amazon.aace.alexa.AlexaProperties.LOCALE );

// set the current locale setting in the Engine
mEngine.setProperty( com.amazon.aace.alexa.AlexaProperties.LOCALE, "en-US" );
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
<td>The current locale setting for AVS. The value should be a valid locale accepted by AVS. Calling <code>Engine.getProperty()</code> with the <code>SUPPORTED_LOCALES</code> property provides the list of supported locales.
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

 See the API reference documentation for  [AlexaProperties](./src/main/java/com/amazon/aace/alexa/AlexaProperties.java) for more information.
