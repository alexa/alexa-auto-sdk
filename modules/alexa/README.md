# Alexa Module

The Alexa Auto SDK Alexa module provides interfaces for standard Alexa features. The Engine handles some extra setup and steps to sequence events and handle directives so you can focus on using the provided API to interact with Alexa. You do this by registering platform interface implementations via the Engine object.

**Table of Contents**

* [Alexa Module Sequence Diagrams](#sequencediagrams)
* [Requesting Wake Word Support](#requesting-wake-word-support)
* [Handling Speech Input](#handling-speech-input)
* [Handling Speech Output](#handling-speech-output)
* [Handling Audio Output](#handling-audio-output)
* [Handling Alexa Speaker](#handling-alexa-speaker)
* [Handling Audio Player](#handling-audio-player)
* [Handling Playback Controller Events](#handling-playback-controller-events)
* [Handling Equalizer Control](#handling-equalizer-control)
* [Handling Display Card Templates](#handling-gui-templates)
* [Handling Global Presets](#handling-global-presets)
* [Handling External Media Apps](#external-media-adapter)
* [Handling Local Media Sources](#handling-local-media-sources)
* [Handling Notifications](#handling-notifications)
* [Handling Alerts](#handling-alerts)
* [Handling Alexa State Changes](#handling-alexa-state-changes)
* [Alexa Engine Properties](#alexa-engine-properties)


## Alexa Module Sequence Diagrams<a id="sequencediagrams"> </a>

For a view of how the Alexa Auto SDK flow works in selected use cases, see these sequence diagrams.

* [Login/Logout Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#loginlogout)
* [Tap to Talk Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#taptotalk)
* [Wake Word Enabled Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#wakewordenabled)

## Requesting Wake Word Support<a id = "requesting-wake-word-support"></a>

If you want to enable wake word support for your Alexa Auto integration, you need to make a request with your Alexa Auto Solution Architect (SA).

There are 3 steps to this process:

1. Let your SA know you want to enable wake word support.
2. Your SA processes your request with the appropriate Alexa teams.
3. You'll receive a single zip file containing the necessary packages, instructions, and scripts.
4. Follow the instructions in the README file to install and build the AmazonLite Wake Word extension.

## Handling Speech Input<a id="handling-speech-input"></a>

It is the responsibility of the `AudioInputProvider` platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to you. An audio playback noise (earcon) is played whenever speech input is invoked. The playback is handled by whichever audio channel is assigned to the EARCON type [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for speech input, extend the `SpeechRecognizer` class:

```
#include <AACE/Alexa/SpeechRecognizer.h>
class MySpeechRecognizer : public aace::alexa::SpeechRecognizer {
    public:
        
        bool wakewordDetected( const std::string& wakeword ) override {
        // called when a wake word is detected
        }
        ... 
        
        void endOfSpeechDetected() override {
        // called to inform the platform of speech end detected
        }
        ...
        
        // To notify the Engine of speech recognize event via press-and-hold initiiation.
        // startCapture( Initiator::HOLD_TO_TALK ) is equivalent.
        holdToTalk();
        ...
        
        // To notify the Engine of a speech recognize event via tap-and-release initiation. 
        // startCapture( Initiator::TAP_TO_TALK ) is equivalent.
        tapToTalk(); 
        ... 
        
        // Notify the engine that the wakeword engine has detected the wakeword. 
        // Currenlty the only keyword value supported is "ALEXA".
        startCapture( Initiator::WAKWORD, 0, 1000, "ALEXA" );
        ...
        
        // Notify the Engine to stop the speech recognition. 
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
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MySpeechRecognizer>() );
```

## Handling Speech Output <a id="handling-speech-output"></a>

The `SpeechSynthesizer` is responsible for handling Alexa's speech. In v2.0.0 and later of the Alexa Auto SDK this interface no longer has any platform-dependent implementation. You still must register it to enable the feature, however. The playback is handled by whichever audio channel is assigned to the TTS type. [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for speech output extend the `SpeechSynthesizer` class:

```
#include <AACE/Alexa/SpeechSynthesizer.h>
class MySpeechSynthesizer : public aace::alexa::SpeechSynthesizer {
    ...
};
...

// Register the platform interface with the Engine
auto mySpeechSynthesizer = std::make_shared<MySpeechSynthesizer>();
engine->registerPlatformInterface( mySpeechSynthesizer );
```   
 
## Handling Audio Output<a id ="handling-audio-output"></a>

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

## Handling Alexa Speaker <a id="handling-alexa-speaker"></a>

The Alexa service keeps track of two device volume types: `ALEXA_VOLUME` and `ALERTS_VOLUME`. The `aace::alexa::AlexaSpeaker` class should be implemented by the platform to both set the volume and mute state of these two speaker types and allow the user to set the volume and mute state of these two speaker types locally via GUI as applicable. 


## Handling Audio Player<a id ="handling-audio-player"></a>

When an audio media stream is received from Alexa it is the responsibility of the platform implementation to play the stream in a platform-specific media player. The `aace::alexa::AudioPlayer` class informs the platform of the changes in player state being tracked by the Engine. This can be used to update the platform GUI, for example.

To implement a custom handler for audio player output, extend the `AudioPlayer` class:

```
#include <AACE/Alexa/AudioPlayer.h>
class MyAudioPlayer : public aace::alexa::AudioPlayer {
    public:
        void playerActivityChanged( PlayerActivity state ) override {
            // on state change, update playback control UI
        }
};
...
        
// Register the platform interface with the Engine
auto myAudioPlayer = std::make_shared<MyAudioPlayer>();
engine->registerPlatformInterface( myAudioPlayer );
```
 
## Handling Playback Controller Events<a id="handling-playback-controller-events"></a>

The Engine provides methods to notify it of media playback control events that happen without voice interaction; for example, a "pause" button press. The platform implementation must inform the Engine of these events using the `PlaybackController` interface any time the user uses on-screen or physical button presses to control media provided by the Engine, such as AudioPlayer source music or `ExternalMediaPlayer` sources, if applicable.

>**Note:** PlaybackController events that control `AudioPlayer` report button presses or the equivalent; they do not report changes to the playback state that happen locally first. The Alexa cloud manages the playback queue for `AudioPlayer` content, so each `PlaybackController` event can be considered a request for the cloud to act on the user's local request. The result of the request will come as a method invocation on the `AudioOutput` associated with the channel used for `AudioPlayer`.

> **Note:** If your implementation needs to stop `AudioPlayer` media in response to system events, such as audio focus transitions to audio playing outside the scope of the Auto SDK, use `PlaybackController` to notify the Engine of such changes. However, bear in mind that the expected usage of the interface does not change when it is used in this use case.

To implement a custom handler for the playback controller, extend the `PlaybackController` class:

```
#include <AACE/Alexa/PlaybackController.h>
class MyPlaybackController : public aace::alexa::PlaybackController {
    ...

    void platformPlayButtonPressed() { // called by some platform event
          buttonPressed(PlaybackButton::PLAY);
    }          
    ...
      
    void platformScrubFordGUIButtonPressed(){ //called by the platform on an avaiable GUI button event
        buttonPressed(PlaybackButton::SKIP_FORWARD);
    }
    ...
    
    void platformShuffleGUIButtonPressed(){ //called by the platform on an avaiable GUI toggle event
        togglePressed(PlaybackToggle::SHUFFLE, true); //the action should send the value opposing the last playerinfo state for that toggle control
    }
    ...  
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyPlaybackController>() );
```     
### PlayerInfo Only Controls

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

The Equalizer Controller is configurable to the device's capabilities. See `aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig` for details on configuring the supported bands, default state, and decibel adjustment range.

To implement a custom handler for Equalizer Controller extend the `EqualizerController` class:

```
#include <AACE/Alexa/EqualizerController.h>

using EqualizerBand = aace::alexa::EqualizerController::EqualizerBand;
using EqualizerBandLevel = aace::alexa::EqualizerController::EqualizerBandLevel;

class MyEqualizerControllerHandler : public aace::alexa::EqualizerController {
  public:
    void setBandLevels( std::vector<EqualizerBandLevel> bandLevels ) override {
    // Handle performing audio equalization on the device 
    // according to the provided band dB level settings

    // This invocation may come from "Alexa, reset bass", 
    // "Alexa, reset my equalizer", "Alexa, increase treble", etc.
    }

    std::vector<EqualizerBandLevel> getBandLevels() override {
        // Return the current band level settings on the device
        return m_currentBandLevels;
    }
};
...

// Configure the Engine
// For example, 2 supported bands with amplitude gains ranging from -8dB to +8dB, each with a default of 0dB
auto eqConfig = aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig(
    {EqualizerBand::BASS, EqualizerBand::TREBLE},
    -8,
     8,
    { {EqualizerBand::BASS, 0}, {EqualizerBand::TREBLE, 0} } );
engine->configure( { //other config objects..., eqConfig, ... } );

...

// Register the platform interface with the Engine
auto m_equalizerController = std::make_shared<MyEqualizerControllerHandler>();
engine->registerPlatformInterface( m_equalizerController );

...

// If levels are adjusted using local on-device controls, call inherited methods to notify the Engine:

// To set a band to an absolute gain level in decibels
std::vector<EqualizerBandLevel> settings{ {EqualizerBand::BASS, 4} }; // Sets bass amplitude to +4dB
m_equalizerController->localSetBandLevels( settings );

// To make a relative adjustment to level settings
std::vector<EqualizerBandLevel> adjustments{ {EqualizerBand::BASS, -2} }; // Decreases bass gain by 2dB
m_equalizerController->localAdjustBandLevels( adjustments );

// To reset gain levels to the configured defaults (usually 0dB)
std::vector<EqualizerBand> bands{EqualizerBand::BASS, EqualizerBand::TREBLE}; // Resets bass and treble bands
m_equalizerController->localResetBands( bands );
```
## Handling Display Card Templates<a id ="handling-gui-templates"></a>

Alexa sends visual metadata (display card templates) for your device to display. When template information is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the information that is received from Alexa. There are two display card template types:

* The [Template](https://alexa.design/DevDocRenderTemplate) type provides visuals associated with a user request to accompany Alexa speech.
* The [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo) type provides visuals associated with media playing through the `AudioPlayer` interface. This includes playback control buttons, which must be used with the `PlaybackController` interface.

To implement a custom handler for GUI templates, extend the `TemplateRuntime` class:

```
#include <AACE/Alexa/TemplateRuntime.h>
class MyTemplateRuntime : public aace::alexa::TemplateRuntime {
    public:
        void renderTemplate( const std::string& payload ) override {
        // handle rendering the template data specified in payload
        }
        
        void renderPlayerInfo( const std::string& payload ) override {
        // handle rendering the player info data specified in payload
        }
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyTemplateRuntime>() );
```
>**Note:** In the case of lists, it is the responsibility of the platform implementation to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of the first five elements, Alexa prompts the user whether or not to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

## Handling Global Presets<a id ="handling-global-presets"></a>

The Global Preset interface is used to handle "Alexa, play preset \<number>\" utterances. The meaning of the preset `number` passed through `setGlobalPreset()` is determined by the `GlobalPreset` platform implementation registered with the Engine, and the implementation should suit the needs of the vehicle's infotainment system. 

Registering a `GlobalPreset` implementation with the Engine is required for Alexa to set presets for any local media `Source` type that may use them (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). 

```
#include <AACE/Alexa/GlobalPreset.h>
...
class MyGlobalPresetHandler : public aace::alexa::GlobalPreset {	
public:
void setGlobalPreset( int number ) override {
    // handle the preset, via routing to local media source
}
...
```    

## Handling External Media Apps<a id="external-media-adapter"></a>

The External Media Adapter allows the platform to declare and use external media application sources such as Spotify. You must register and implement each External Media Adapter along with its associated external client. On startup, you must run discovery in order to validate each external media application. This allows AVS to excercise playback control over that source type. Currently this feature is only officially supported on the Android platform. 

## Handling Local Media Sources<a id ="handling-local-media-sources"></a>

The `LocalMediaSource` interface allows the platform to register a local media source by type(`BLUETOOTH`, `USB`, `LINE_IN`, `AM_RADIO` etc.). Registering a local media source allows playback control of a source via Alexa (e.g. "Alexa, play the CD player") or via button press through the `PlaybackController` interface, if desired. It also enables playback initiation via Alexa by frequency, channel, or preset for relevant source types (e.g. "Alexa, play 98.7 FM")

>**NOTE:** Local media source control with Alexa is currently supported in the US region only.

The following is an example of registering a CD player local media source using type `Source.COMPACT_DISC`:

```
auto m_CDLocalMediaSource = std::make_shared<MyCDLocalMediaSource>( Source.COMPACT_DISC );
engine->registerPlatformInterface( m_CDLocalMediaSource );
```

To implement a custom handler for a CD player local media source extend the `LocalMediaSource` class:

```
#include <AACE/Alexa/LocalMediaSource.h>
class MyCDLocalMediaSource : public aace::alexa::LocalMediaSource {
  public:
    MyCDLocalMediaSource( LocalMedidaSource::Source source ) {
        m_source = source;	
        ...
    }
    ...
}; 
... 
```

The `play()` method is called when Alexa invokes play by `ContentSelector` type for a radio local media source (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). The `payload` is a string that depends on the `ContentSelector` type and local media `Source` type.

```	
bool play( ContentSelector type, std::string payload ) override {
    setFocus();
    // play initiation for frequency, channel, or presets
    ...
}
``` 

This method will not be invoked if a source cannot handle the specified content selection type.

The implementation depends on the local media source; however if the call is handled successfully, `setFocus()` should always be called. This informs the Engine that the local player is in focus. 

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
bool playControl( PlayControlType controlType ) override {
    setFocus();
    // handle the control type appropriately for CD player
    return true;
}
```

>**NOTE:** The `play()` method is used to initiate playback with specified content selection, whereas `playControl(RESUME)` is used to play or resume whatever is already playing.

The `seek()` and `adjustSeek()` methods are invoked to seek the currently focused `LocalMediaSource`. These methods are only used by sources that are capable of seeking. `seek()` is for specifying an absolute offset, whereas `adjustSeek()` if for specifying a relative offset. 

```	
bool seek( long offset ) override {
    // handle seeking CD player
}
...

bool adjustSeek( long offset ) override {
    // handle adjusting seek for CD player
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

Many fields of the `LocalMediaSourceState` are not required for local media source players. You should omit these as noted below.

```
LocalMediaSourceState getState() override {
    LocalMediaSourceState stateToReturn = std::make_shared<LocalMediaSourceState>();
    stateToReturn.playbackState.albumName = "mock albumName";
    // fill in all required state information (see below)
    return stateToReturn;
}
```

The following table describes the fields comprising a `LocalMediaSourceState`, which includes two sub-components: `PlaybackState` and `SessionState`.

| State        | Type           | Notes  |
| ------------- |:-------------:| -----:|
| **PlaybackState**      | 
| state      | String        |   "IDLE/STOPPED/PLAYING" required |
| supportedOperations | SupportedOperations[] | (see SupportedOperation) required |
| trackOffset      | long  |   optional |
| shuffleEnabled      | boolean       |   optional |
| repeatEnabled      | boolean       |   optional |
| favorites      | Favorites  |   {FAVORITED/UNFAVORITED/NOT_RATED} optional  |
| type      | String  |   "ExternalMediaPlayerMusicItem" required |
| playbackSource      | String       |   If available else use local player name. optional|
| playbackSourceId      | String  |   empty |
| trackName      | String   |   If available else use local player name. optional |
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
| mediaType      | MediaType |   {TRACK, PODCAST, STATION, AD, SAMPLE, OTHER} optional |
| duration      | long  |   optional |
| **SessionsState** | 
| endpointId      | String  |   empty |
| loggedIn      | boolean  |   empty |
| userName      | String  |   empty |
| isGuest      | boolean  |   empty |
| launched      | boolean  |   empty |
| active      | boolean  |   empty |
| accessToken      | String  |   empty |
| tokenRefreshInterval      | long  |   empty |
| playerCookie      | String  |   A player may declare arbitrary information for itself. optional |
| spiVersion      | String  |   "1.0" required  |

`supportedOperations` should list the operations which the local media source supports. Below is a list of all `supportedOperations`:

```
LocalMediaSource::SupportedPlaybackOperation::PLAY,
LocalMediaSource::SupportedPlaybackOperation::PAUSE,
LocalMediaSource::SupportedPlaybackOperation::STOP,
LocalMediaSource::SupportedPlaybackOperation::RESUME,
LocalMediaSource::SupportedPlaybackOperation::PREVIOUS,
LocalMediaSource::SupportedPlaybackOperation::NEXT,
LocalMediaSource::SupportedPlaybackOperation::ENABLE_SHUFFLE,
LocalMediaSource::SupportedPlaybackOperation::DISABLE_SHUFFLE,
LocalMediaSource::SupportedPlaybackOperation::ENABLE_REPEAT_ONE,
LocalMediaSource::SupportedPlaybackOperation::ENABLE_REPEAT,
LocalMediaSource::SupportedPlaybackOperation::DISABLE_REPEAT,
LocalMediaSource::SupportedPlaybackOperation::SEEK,
LocalMediaSource::SupportedPlaybackOperation::ADJUST_SEEK,
LocalMediaSource::SupportedPlaybackOperation::FAVORITE,
LocalMediaSource::SupportedPlaybackOperation::UNFAVORITE,
LocalMediaSource::SupportedPlaybackOperation::FAST_FORWARD,
LocalMediaSource::SupportedPlaybackOperation::REWIND,
LocalMediaSource::SupportedPlaybackOperation::START_OVER
```

>**Note:** For local media sources, do not use `LocalMediaSource::SupportedPlaybackOperation::RESUME`

`supportedContentSelectors` should list the content selection types the local source can support. Below is a table of valid pairs.

| Source | supportable `ContentSelector`'s |
|---|---|
| `AM_RADIO` |  `PRESET`, `FREQUENCY` |
| `FM_RADIO` |  `PRESET`, `FREQUENCY` |
| `SIRIUS_XM` |  `PRESET`, `CHANNEL` |

## Handling Notifications<a id ="handling-notifications"></a>

It is the responsibility of the platform implementation to provide a visual indication to the user when notifications (for example, package shipment notifications, notifications from skills, etc.) are available from Alexa. [Read more about Notifications here](https://alexa.design/AVSDevNotifications). The Engine uses the registered Notifications implementation to notify you when a notification indicator should be displayed or removed. It does not give any information about the notifications. Audio playback for the notification is handled by whichever audio channel is assigned to the NOTIFICATION type [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for Notifications extend the `Notifications` class:

```
#include <AACE/Alexa/Notifications.h>
using IndicatorState = aace::alexa::Notifications::IndicatorState;
class MyNotificationsHandler : public aace::alexa::Notifications {

  public:
    void setIndicator( IndicatorState state ) override {
        // set your notifcations indicator!
    }

};
...

// Register the platform interface with the Engine
auto m_notificationsHandler = std::make_shared<MyNotificationsHandler>();
engine->registerPlatformInterface(m_notificationsHandler);

```

## Handling Alerts<a id = "handling-alerts"></a>

When an alert is received from Alexa, it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player. The state of the alert is also made available for the platform to react to. The playback is handled by whichever audio channel is assigned to the ALERT type [Read more about handling media and volume here](#handling-media-and-volume).

To implement a custom handler for alerts, extend the `Alerts` class:

```
#include <AACE/Alexa/Alerts.h>
class MyAlerts : public aace::alexa::Alerts {
    public:
        void MyAlerts::alertStateChanged( const std::string& alertToken, AlertState state, const std::string& reason ) override {
        //handle the alert state change
        }

        void MyAlerts::alertCreated( const std::string& alertToken, const std::string& detailedInfo ) override {
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
        
        void MyAlerts::alertDeleted( const std::string& alertToken ) override {
            //handle the alert when alert is deleted (optional)
        }
};
...

// Register the platform interface with the Engine
auto myAlertsMediaPlayer = std::make_shared<MyMediaPlayer>(...);
auto myAlertsSpeaker = std::make_shared<MySpeaker>(...);
auto myAlerts = std::make_shared<MyAlerts>(myAudioPlayerMediaPlayer, myAudioPlayerSpeaker);
engine->registerPlatformInterface( myAlerts );
```
    
## Handling Alexa State Changes <a id="handling-alexa-state-changes"></a>

The Alexa Auto SDK manages internal state information for Alexa and provides an interface for you to handle state changes in your platform implementation. The information provided by method invocations of this class might be useful, for instance, to enable or disable certain functionality or trigger state changes in your Alexa attention state UI (such as Voice Chrome). To implement a custom handler for Alexa state changes, extend the `AlexaClient` class:

```
#include <AACE/Alexa/AlexaClient.h>
class MyAlexaClient : public aace::alexa::AlexaClient {
    public:
        void dialogStateChanged( DialogState state ) override {
        // handle dialog state change!
        }
        void authStateChanged( AuthState state, AuthError error ) override {
        // handle auth state change!
        }
        void connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) override {
        // handle connection status change!
        }
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyAlexaClient>() );
```

## Alexa Engine Properties<a id="alexa-engine-properties"></a>

The Alexa module defines several constants that are used to get and set runtime properties in the Engine. To use these properties call the Engine's `getProperty()` and `setProperty()` methods.

```
// get the current locale setting from the Engine
String locale = m_engine->getProperty( com.amazon.aace.alexa.AlexaProperties.LOCALE );

// set the current locale setting in the Engine
m_engine->setProperty( com.amazon.aace.alexa.AlexaProperties.LOCALE, "en-US" );
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

 See the API reference documentation for  [AlexaProperties](./platform/include/AACE/Alexa/AlexaProperties.h) for more information.
