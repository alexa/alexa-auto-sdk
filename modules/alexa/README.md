## Alexa API

### Overview

The Alexa Auto SDK Alexa API provides interfaces for standard Alexa features. The Engine handles some extra setup and steps to sequence events and directive handling. The platform developer can focus on just using the provided API to interact with Alexa. This is done by registering platform interfaces via the Engine object.

### Sequence Diagrams for the Alexa Module in the Alexa Auto SDK<a id="sequencediagrams"> </a>

You can read more about how the Alexa Auto SDK flow works by checking out some sequence diagrams.

* [Login/Logout Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#loginlogout)
* [Tap to Talk Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#taptotalk)
* [Wake Word Enabled Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#wakewordenabled)

### Request Wake Word Support for Alexa

If you want to enable wake word support for your Alexa Auto integration, you need to make a request with your Alexa Auto Solution Architect (SA).

There are 3 steps to this process:

1. Let your SA know you want to enable wake word support.
2. Your SA processes your request with the appropriate Alexa teams.
3. You'll receive a single zip file containing the necessary packages, instructions, and scripts.

All that's left for you to do is create an "extras" directory under your "aac-sdk" directory for the packages and follow the instructions in the README file included in the package.

### Handling Alexa state changes

The Alexa Auto SDK manages internal state information for Alexa and provides an interface for developers to handle state changes in their platform. To implement a custom handler for Alexa state changes, the `aace::alexa::AlexaClient` class should be extended:

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

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MyAlexaClient>() );

### Handling speech input

It is the responsibility of the platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to the platform developer. The default platform implementation provides methods for writing raw PCM data to the Engine's audio input processor, which includes handling wakeword recognition and end-of-speech detection.

The audio input format should be encoded as:

* 16bit Linear PCM

* 16kHz sample rate

* Single channel

* Little endian byte order

To implement a custom handler for speech input, the `aace::alexa::SpeechRecognizer` class should be extended:

    #include <AACE/Alexa/SpeechRecognizer.h>
    class MySpeechRecognizer : public aace::alexa::SpeechRecognizer {
      public:
        // possible platform implementation that has an audio input callback
        void startAudioInput(){
          MyAudioInput->start(); //start platform-specific audio input
          ...

        }
        ...

        void MyAudioInputOnWriteCallback( const int16_t* data, const size_t size ) {
            // call inherited write() method to provide audio samples
            // to the Engine for input processing
            write( data, size );  
            ...

        }
    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MySpeechRecognizer>() );

### Handling speech output

The speech synthesizer is responsible for handling Alexa's speech. The `SpeechSynthesizer` class extends the abstract `aace::alexa::AudioChannel` class. This is to separate each audio-capable class on the platform while allowing the nEgine to control their interactive behaviors. The platform implementation provides a reference to an instance of a *media player* and a *speaker*. (The term "speaker" is used here in the sense of an "output device") [**Read more about handling media and volume here**](#handling-media-and-volume).

 To implement a custom handler for speech output the `aace::alexa::SpeechSynthesizer` class should be extended:

      #include <AACE/Alexa/SpeechSynthesizer.h>
      class MySpeechSynthesizer : public aace::alexa::SpeechSynthesizer {
        public:
          MySpeechSynthesizer(std::shared_ptr<MyMediaPlayer> myMediaPlayer, std::shared_ptr<MySpeaker> mySpeaker);
          ...
      }
      ...

      MySpeechSynthesizer::MySpeechSynthesizer(std::shared_ptr<MyMediaPlayer> mediaPlayer, std::shared_ptr<MySpeaker> mySpeaker) : aace::alexa::SpeechSynthesizer(mediaPlayer, mySpeaker) { }
      ...

      // Configure the Engine
      auto mySpeechSynthesizerMediaPlayer = std::make_shared<MyMediaPlayer>(...);
      auto mySpeechSynthesizerSpeaker = std::make_shared<MySpeaker>(...);
      auto mySpeechSynthesizer = std::make_shared<MySpeechSynthesizer>( mySpeechSynthesizerMediaPlayer, mySpeechSynthesizerSpeaker);
      engine->registerPlatformInterface( mySpeechSynthesizer );

### Handling audio player output

When an audio media stream is received from Alexa it is the responsibility of the platform implementation to play the stream in a platform-specific media player and to notify the Engine when the audio starts and stops playing.

The class is similar to the SpeechSynthesizer class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for audio player output the `aace::alexa::AudioPlayer` class should be extended:

    #include <AACE/Alexa/AudioPlayer.h>
    class MyAudioPlayer : public aace::alexa::AudioPlayer {
      public:
        MyAudioPlayer(std::shared_ptr<MyMediaPlayer> myMediaPlayer, std::shared_ptr<MySpeaker> mySpeaker);
          ...

    MyAudioPlayer::MyAudioPlayer(std::shared_ptr<MyMediaPlayer> mediaPlayer, std::shared_ptr<MySpeaker> mySpeaker) : aace::alexa::AudioPlayer(mediaPlayer, mySpeaker) { }
    ...

    // Configure the Engine
    auto myAudioPlayerMediaPlayer = std::make_shared<MyMediaPlayer>(...);
    auto myAudioPlayerSpeaker = std::make_shared<MySpeaker>(...);
    auto myAudioPlayer = std::make_shared<MyAudioPlayer>(myAudioPlayerMediaPlayer, myAudioPlayerSpeaker);
    engine->registerPlatformInterface( myAudioPlayer );

### Handling notifications

It is the responsibility of the platform implementation to provide a visual and audio indication to the user when notifications (package shipment notifications, notifications from skills, etc.) are available from Alexa. [**Read more about Notifications here**](https://alexa.design/AVSDevNotifications).

To implement a custom handler for Notifications the `aace::alexa::Notifications` class should be extended:

```
#include <AACE/Alexa/Notifications.h>
using IndicatorState = aace::alexa::Notifications::IndicatorState;
class MyNotificationsHandler : public aace::alexa::Notifications {
	 MyNotificationsHandler(std::shared_ptr<aace::alexa::MediaPlayer> player, std::shared_ptr<aace::alexa::Speaker> speaker)

  public:
    void setIndicator( IndicatorState state ) override {
        // set your notifcations indicator!
    }

};
...

// Configure the Engine
auto notificationsMediaPlayer = std::make_shared<MyMediaPlayer>(...);
auto notificationsSpeaker = std::make_shared<MySpeaker>(...);
auto m_notificationsHandler = std::make_shared<MyNotificationsHandler>(notificationsMediaPlayer, notificationsSpeaker);
engine->registerPlatformInterface(m_notificationsHandler);

```

### Handling alerts

When an alert is received from Alexa, it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player and notify the Engine when the audio starts and stops playing. The state of the alert is also made available for the platform to react to. The class is similar to the `SpeechSynthesizer` class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for alerts, the `aace::alexa::Alerts` class should be extended:

    #include <AACE/Alexa/Alerts.h>
    class MyAlerts : public aace::alexa::Alerts {
      public:
        MyAlerts(std::shared_ptr<MyMediaPlayer> myMediaPlayer, std::shared_ptr<MySpeaker> mySpeaker)
    ...

    MyAlerts::MyAlerts(std::shared_ptr<MyMediaPlayer> mediaPlayer, std::shared_ptr<MySpeaker> mySpeaker) : aace::alexa::Alerts(mediaPlayer, mySpeaker) { }

    void MyAlerts::alertStateChanged( const std::string& alertToken, AlertState state, const std::string& reason ) override {
      //handle the alert state change
    }
    ...
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


    // Configure the Engine

    auto myAlertsMediaPlayer = std::make_shared<MyMediaPlayer>(...);
    auto myAlertsSpeaker = std::make_shared<MySpeaker>(...);
    auto myAlerts = std::make_shared<MyAlerts>(myAudioPlayerMediaPlayer, myAudioPlayerSpeaker);
    engine->registerPlatformInterface( myAlerts );

### Handling media and volume

When audio data is received from Alexa it is the responsibility of the platform implementation to read the data from the Engine and play it using a platform-specific media player and speaker. It is also the responsibility of the platform implementation to notify the Engine when the speech audio actually begins playing in the media player and when it is finished. Alexa will notify the Engine that the platform should play/stop/pause/resume playing the speech or other audio in which case the platform implementation must play/stop/pause/resume the media player playback. The platform should extend both `MediaPlayer` and `Speaker` and implement the platform-specific functionality. These classes will then be instantiated by other platform interfaces (`Alerts`, `SpeechSynthesizer`, `AudioPlayer`) as separate audio channels.

To implement a custom handler for MediaPlayer the `aace::alexa::MediaPlayer` class should be extended:

    #include <AACE/Alexa/MediaPlayer.h>
    class MyMediaPlayer : public aace::alexa::MediaPlayer{
      public:
        bool prepare() override {
          ...

          while(read(data, size) > 0)
             //write readable data to media location
          ...

          if(successful) return true;
          else playbackError(type, error);// if platform mediaplayer fails, notify the engine with type and description
        }
        bool prepare(std::string url) override {
          MyMediaPlayerReference->MySetMediaUrl(url)// set platform media when url is given
          ...

          return true; //if successful
        }
        bool play() override {
          ...

          MyMediaPlayerReference->MyPlay(); //play platform mediaplayer
          ...

          return true; //if successful
        }
        ...

      PlatformMediaPlayer* MyMediaPlayerReference = ...//platform-specific
    }    

The platform implementation's Speaker class will likely need a reference to the media player class.

To implement a custom handler for Speaker, the `aace::alexa::Speaker` class should be extended:

    #include <AACE/Alexa/Speaker.h>
      class MySpeaker : public aace::alexa::Speaker{
        public:
          MySpeaker(std::shared_ptr<MyMediaPlayer> mediaPlayer){
              m_mediaPlayer = mediaPlayer;
          }; //One way to get the media player reference
          ...

          bool setVolume( int8_t volume ) override {
            m_mediaPlayer->MySetVolume(volume);//platform-specific.
            m_myPlatformVolume = volume;
            return true;
            ...

          int8_t getVolume() override {
            return m_myPlatformVolume;//using a member variable
          }
          ...
    }

#### Custom Volume Control for Alexa Devices

You can use a custom volume control to support an Alexa device's native input volume range. By default, Alexa supports voice utterances that specify volume values between 0 and 10, but some devices may support a different range (i.e. 0 to 100). By whitelisting your Alexa devices volume range with Amazon for your target platform, you can specify input volume levels per your device's range. Your device's input volume range is then mapped appropriately to the Alexa volume range.

Contact your Alexa Auto Solution Architect (SA) for help with whitelisting. Whitelisting requires the following parameters:

* DeviceTypeID
* Min:
* Max:

This does not impact the range used in the directives to the device.

### Handling GUI templates (display cards)

When template info is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the info that is received from Alexa. There are two template types: [Templates](https://alexa.design/DevDocRenderTemplate) and [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo).

In the case of lists, it is the responsibility of the platform to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of reading the first five elements it will prompt the user to see if they want to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

To implement a custom handler for GUI templates, the `aace::alexa::TemplateRuntime` class should be extended:

    #include <AACE/Alexa/TemplateRuntime.h>
    class MyTemplateRuntime : public aace::alexa::TemplateRuntime {
      public:
        void doRenderTemplate( const std::string& payload ) override {
            // handle rendering the template data specified in payload
        }
        void doRenderPlayerInfo( const std::string& payload ) override {
            // handle rendering the player info data specified in payload
        }
    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MyTemplateRuntime>() );

### Handling playback controller events

The Engine provides methods for notifying it of playback controller events. If the platform has playback control features, it must inform the Engine.

#### PlayerInfo Only controls

The Template Runtime's Player Info template specifies some additional controls to be displayed. This is for GUI implementations which use the PlayerInfo template as a reference for their GUI displays. The controls available, for a given service, come down with the playerInfo template. For toggles, the synced state is also provided by the PlayerInfo template.

* Buttons
	- Skip Forward ( service defined scrub forward )
	- Skip Backward ( service defined scrub backward )
* Toggles
	- Shuffle ( toggle shuffle songs )
	- Loop ( toggle playlist looping )
	- Repeat ( toggle repeat current media once )
	- Thumbs Up ( toggle thumbs up state )
	- Thumbs Down ( toggle thumbs down state )


To implement a custom handler for the playback controller, the `aace::alexa::PlaybackController` class should be extended:

    #include <AACE/Alexa/PlaybackController.h>
    class MyPlaybackController : public aace::alexa::PlaybackController {
      ...

      void platformPlayButtonPressed(){ //called by some platform event
          buttonPressed(PlaybackButton::PLAY);
          ...
      void platformScrubFordGUIButtonPressed(){ //called by the platform on an avaiable GUI button event
          buttonPressed(PlaybackButton::SKIP_FORWARD);
          ...
      void platformShuffleGUIButtonPressed(){ //called by the platform on an avaiable GUI toggle event
          togglePressed(PlaybackToggle::SHUFFLE, true); //the action should send the value opposing the last playerinfo state for that toggle control
          ...  

    };
    ...

    // Configure the Engine
    engine->registerPlatformInterface( std::make_shared<MyPlaybackController>() );

### Handling equalizer control

The Equalizer Controller enables Alexa voice control of the device's audio equalizer settings, which includes making gain level adjustments to any of the supported frequency bands ("BASS", "MIDRANGE", and/or "TREBLE") using the device's onboard audio processing. 

The platform implementation is responsible for the following:
* Determining how each supported band affects the audio
* Mapping Alexa's equalizer bands to the bands supported on the device, if they do not directly correspond
* Scaling Alexa's level values as necessary so that each step corresponds to one decibel of amplitude gain on the device
* Applying equalization to only selected portions of the audio output so that Alexa's speech, alarms, etc. will not be affected
* Persisting settings across power cycles

The Equalizer Controller is configurable to the device's capabilities. See `aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig` for details on configuring the supported bands, default state, and decibel adjustment range.

To implement a custom handler for Equalizer Controller the `aace::alexa::EqualizerController` class should be extended:

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

// Register the platform interface implementation with the Engine
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
    
### Handling Local Media Sources 

The Local Media Source allows the platform to declare and use local media sources such as radio, bluetooth, compact disc, or line in.
Each local media source should be registered using the corresponding local Media source type. This will allow AVS to excercise playback control over that source type. The cloud will authorize a registered Local Media Source, and if successful that source will be controllable via Alexa voice interaction. Currently, the interactions are sent to the `playControl` method. Below is an example of a CD player local media source implementation. 

```
	#include <AACE/Alexa/LocalMediaSource.h>
    class MyCDLocalMediaSource : public aace::alexa:: LocalMediaSource {	
    public:
		MyCDLocalMediaSource( LocalMedidaSource::Source source, std::shared_ptr<MyCDSpeaker> speaker){
	 		m_source = source;
			m_speaker = speaker;
		}; 
		...
		  
		bool authorize( boolean authorized ) override {
			// if true, CD playersource was authorized by cloud
			m_authorized = authorized; // save auth state
		...
		bool play( std::string payload ) override {
		// currently not used. may be used in future to handle play initiation
		...
      		
  		bool playControl( PlayControlType controlType ) override {
  			if ( m_authorized ) {
				setFocus();
				// handle the control type appropriately for CD player
				return true;
		...
		bool seek( long offset ) override {
			if ( m_authorized ) {
				// handle seeking CD player
		...
		LocalMediaSourceState getState() override {
			LocalMediaSourceState stateToReturn = std::make_shared<LocalMediaSourceState>();
			stateToReturn.playbackState.albumName = "mock albumName";
			... // fill in all required state information (see below)
			return stateToReturn;
 
// Configure the Engine
	auto myCDSpeaker = std::make_shared<MyCDSpeaker>(...);
    engine->registerPlatformInterface( std::make_shared<MyLocalMediaSource>(LocalMedidaSource::Source::COMPACT_DISC, myCDSpeaker) );
```

The platform implementation should create it's own speaker, and can be used to listen to volume control directives. It need not specify an AVS Speaker type. 

The following table describes the possible values for `LocalMediaSourceState`, and additional details. 

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


> **Note**: Local Media Source Switching currently works only in US region.


### Using External Media Adapter to handle external media apps

The External Media Adapter allows the platform to declare and use external media application sources such as Spotify. Each External media adapter should be registered and implemented along with it's associated external client. On startup, discovery must be run in order to validate each external media application. This will allow AVS to excercise playback control over that source type. Currently this feature is only officially supported on the Android platform. 

## Alexa Engine Properties

The Alexa module defines several constants that are used to get and set runtime properties in the Engine. To use these properties, include the `AlexaProperties.h` header in your source code and call the Engine's `getProperty()` and `setProperty()` methods.

    #include <AACE/Alexa/AlexaProperties.h>

    // get the current locale setting from the Engine
    auto locale = m_engine->getProperty( aace::alexa::property::LOCALE );

    // set the current locale setting in the Engine
    m_engine->setProperty( aace::alexa::property::LOCALE, "en-US" );

The following constants are defined in the Alexa module:

<table>
<tr>
<th>Property</th>
<th>Description</th>
</tr>
<tr>
<td>
<code>aace::alexa::property::AVS_ENDPOINT</code>
</td>
<td>The value must be a valid AVS endpoint URL.
</td>
</tr>
<tr>
<td>
<code>aace::alexa::property::WAKEWORD_SUPPORTED</code>
</td>
<td><p>Describes if wake word support is enabled. If wake word is not supported in the Engine, attempts to enable wake word detection by
the <code>SpeechRecognizer</code> will fail.</p>
<p><strong>Note</strong>: This is a read-only property.</p>
</td>
</tr>
<tr>
<td>
<code>aace::alexa::property::FIRMWARE_VERSION</code>
</td>
<td>The firmware version that is reported to AVS. The value must be a positive, signed 32-bit integer represented as a string.
</td>
</tr>
<tr>
<td>
<code>aace::alexa::property::LOCALE</code>
</td>
<td>The current locale setting for AVS. The value should be a valid locale accepted by AVS. Calling <code>Engine::getProperty()</code> with the <code>SUPPORTED_LOCALES</code> property provides the list of supported locales.
</td>
</tr>
<tr>
<td>
<code>aace::alexa::property::SUPPORTED_LOCALES</code></li>
</td>
<td><p>AVS supported locales. The value is a comma-separated list, e.g. "de-DE,en-AU,..."</p>
<p><strong>Note</strong>: This is a read-only property.</p>

</td>
</tr>
</table>

 See the API reference documentation for  [AlexaProperties](./platform/include/AACE/Alexa/AlexaProperties.h) for more information.
