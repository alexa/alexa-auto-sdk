# Alexa Module

The Alexa Auto SDK Alexa module provides interfaces for standard Alexa features. The Engine handles some extra setup and steps to sequence events and handle directives so you can focus on using the provided API to interact with Alexa. You do this by registering platform interface implementations via the Engine object.

>**Note:** If you want to enable wake word support, [contact your Amazon Solutions Architect (SA)](../../NEED_HELP.md#requesting-additional-functionality).

**Table of Contents**

- [Alexa Module Sequence Diagrams](#alexa-module-sequence-diagrams)
- [Configuring Alexa Module](#configuring-alexa-module)
- [Configuring using a JSON File](#configuring-using-a-json-file)
  - [Using a JSON File](#using-a-json-file)
  - [Using Programmatic Configuration](#using-programmatic-configuration)
- [Handling Speech Input](#handling-speech-input)
- [Handling Speech Output](#handling-speech-output)
- [Handling Authorization](#handling-authorization)
  - [Using the Authorization Platform Interface to Carry out Auth Provider Authorization](#using-the-authorization-platform-interface-to-carry-out-auth-provider-authorization)
  - [Sequence Diagrams for Auth Provider Authorization](#sequence-diagrams-for-auth-provider-authorization)
  - [Optional Auth Provider Configuration](#optional-auth-provider-configuration)
  - [(Deprecated) Implementing a Custom Handler for AuthProvider](#deprecated-implementing-a-custom-handler-for-authprovider)
- [Handling Audio Output](#handling-audio-output)
  - [Custom Volume Control for Alexa Devices](#custom-volume-control-for-alexa-devices)
- [Handling Alexa Speaker](#handling-alexa-speaker)
  - [SpeakerManager Configuration](#speakermanager-configuration)
- [Handling Audio Player](#handling-audio-player)
- [Handling Playback Controller Events](#handling-playback-controller-events)
- [Handling Equalizer Control](#handling-equalizer-control)
- [Handling Display Card Templates](#handling-display-card-templates)
- [Handling External Media Apps](#handling-external-media-apps)
- [Handling Local Media Sources](#handling-local-media-sources)
- [Handling Global Presets](#handling-global-presets)
- [Handling Notifications](#handling-notifications)
- [Handling Alerts](#handling-alerts)
- [Handling Do Not Disturb](#handling-do-not-disturb)
- [Handling Alexa State Changes](#handling-alexa-state-changes)

## Alexa Module Sequence Diagrams

For a view of how the Alexa Auto SDK flow works in selected use cases, see these sequence diagrams:

* [Tap to Talk Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#tap-to-talk-sequence-diagram)
* [Wake Word Enabled Sequence Diagram](../../SEQUENCE_DIAGRAMS.md#wake-word-enabled-sequence-diagram)

## Configuring Alexa Module

The Alexa module can be configured in two different ways:
* Specifying Configuration Data Programmatically.
* Specifying Configuration Data Using a JSON File.

## Configuring using a JSON File

### Using a JSON File 
Include the following structure in the JSON file:

```
"aace.alexa": {
        "avsDeviceSDK": {
            "deviceInfo": {
                "clientId": "${YOUR_CLIENT_ID}",
                "deviceSerialNumber": "${YOUR_DEVICE_SERIAL_NUMBER}",
                "productId": "${YOUR_PRODUCT_ID}",
                "manufacturerName": "${YOUR_MANUFACTURER_NAME}",
                "description": "${YOUR_DESCRIPTION}"
            },
            "libcurlUtils": {
                "CURLOPT_CAPATH" : "<CA_CERTIFICATES_FILE_PATH>"
                "CURLOPT_INTERFACE" : "<NETWORK_INTERFACE_NAME>"
            },
            "miscDatabase": {
                "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
            },
            "certifiedSender": {
                "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
            },
            "alertsCapabilityAgent": {
                "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
            },
            "notifications": {
                "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
            },
            "capabilitiesDelegate": {
                "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
            },
            "deviceSettings": {
                "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>",
                "locales": [<LIST_OF_LOCALE_STRINGS>],
                "defaultLocale": "<DEFAULT_LOCALE_STRING>",
                "localeCombinations": [[<LOCALE_STRING_PAIR>]],
                "defaultTimezone": "<TIMEZONE>"
            }
        }
    }
```
The `deviceInfo` field contains the details of the device. The fields `libcurlUtils`, `miscDatabase`, `certifiedSender`, `alertsCapabilityAgent`, `notifications`, and `capabilitiesDelegate` specify the respective database file paths.

The `deviceSettings` field specifies the settings on the device. The `databaseFilePath` is the path to the SQLite database that stores persistent settings. The database will be created on initialization if it does not already exist.
`defaultLocale` specifies the default locale string. The default value is "en-US".
`locales` specifies the list of locales supported by the device. The default value is `["en-US","en-GB","de-DE","en-IN","en-CA","ja-JP","en-AU","fr-FR","it-IT","es-ES","es-MX","fr-CA","es-US", "hi-IN", "pt-BR"]`.
`localeCombinations` specifies the list of locale pairs available on the device, which enables the Engine to support Dual Language Switching. The permitted combinations are `[["en-CA","fr-CA"],["fr-CA","en-CA"],["en-US","es-US"],["es-US","en-US"],["en-IN","hi-IN"],["hi-IN","en-IN"]]`. Ensure each locale included in a pair in the `localeCombinations` field also exists in the "locales" field, which declares all locales supported by the device. 
Any application can select the pair as a locale setting on the device based on the primary locale ( first locale in the pair), only if the secondary locale ( second locale in the pair) is supported by the application. The Engine should also be configured with this permitted pair.

**Note:** Dynamic Language Switching is only available in online mode. 

For file-based configuration, if `locales` field is not specified, the Engine gets automatically configured with the default locale combinations.

### Using Programmatic Configuration
All of the above specified fields can be configured programmatically using the methods provided by the AlexaConfiguration platform interface. For example, to create `DeviceInfoConfig`:

```cpp
auto deviceConfig = aace::alexa::config::AlexaConfiguration::createDeviceInfoConfig
    ("<DEVICE_SERIAL_NUMBER>", "<CLIENT_ID>", "<PRODUCT_ID>", "<MANUFACTURER_NAME>", "<DESCRIPTION>");
```

```cpp
auto deviceSettingsConfig = aace::alexa::config::AlexaConfiguration::createDeviceSettingsConfig(
    "<SQLITE_DATABASE_FILE_PATH>", [<LIST_OF_LOCALE_STRINGS>], "<DEFAULT_LOCALE_STRING>", "<TIMEZONE>", [[<LOCALE_STRING_PAIR>]]);
```

Refer to the [core module](../core/README.md) documentation for steps to specify configuration data programmatically or through a JSON file.

## Handling Speech Input

It is the responsibility of the `AudioInputProvider` platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to you. An audio playback sound (earcon) is played whenever speech input is invoked. The playback is handled by whichever audio channel is assigned to the EARCON type. [Read more about handling media and volume here](#handling-audio-output).

You can programmatically generate encoder configuration using the `aace::alexa::config::AlexaConfiguration::createSpeechRecognizerConfig()` factory method, or provide the equivalent JSON values in a configuration file.

```
{
    "aace.alexa": {
       "speechRecognizer": {
           "encoder": {
                "name": "<ENCODER_NAME>"
           }
       }
    }
}
```
The only possible value for `name` is "opus". If you do not specify a value for `name`, speech is sent in PCM format, which consumes more upload bandwidth than it would if you set the value to "opus".

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
        
        // To notify the Engine of speech recognize event via press-and-hold initialization.
        // startCapture( Initiator::HOLD_TO_TALK ) is equivalent.
        holdToTalk();
        ...
        
        // To notify the Engine of a speech recognize event via tap-and-release initiation. 
        // startCapture( Initiator::TAP_TO_TALK ) is equivalent.
        tapToTalk(); 
        ... 
        
        // Notify the engine that the wake word engine has detected the wake word. 
        // Currently the only keyword value supported is "ALEXA".
        startCapture( Initiator::WAKEWORD, 0, 1000, "ALEXA" );
        ...
        
        // Notify the Engine to stop the speech recognition. 
        // For example, when hold-to-talk was used and is now being ended by the button release action
        stopCapture();
        ...
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MySpeechRecognizer>() );
```

## Handling Speech Output

The `SpeechSynthesizer` is responsible for handling Alexa's speech. In v2.0.0 and later of the Alexa Auto SDK this interface no longer has any platform-dependent implementation. You still must register it to enable the feature, however. The playback is handled by whichever audio channel is assigned to the TTS type. [Read more about handling media and volume here](#handling-audio-output).

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

## Handling Authorization

Every request to Alexa Voice Service (AVS) requires a Login with Amazon (LWA) access token.
Starting with Auto SDK v3.1.0, use the Authorization platform interface to start, cancel, and log out of authorization. The Auto SDK continues to support the AuthProvider platform interface, but it is on the deprecation path. For more information about how the Engine manages authorization, see the Core module [README](../core/README.md).

In this document, "Auth Provider authorization" refers to the general method through which the application implements acquiring access tokens itself and provides them to the Engine.

>**Note**: Auth Provider authorization and other authorization methods, such as [CBL](../cbl/README.md), are mutually exclusive. For example, if the device is already registered with CBL, starting Auth Provider authorization logs out the device from the previous authorization.

> **Note:** The access and refresh tokens must be cleared when the user logs out, and any time the access and refresh tokens are cleared, users must go through the authentication and authorization process again.

> **Note:** Your application should stop the audio player if the user logs out of a device when music is currently playing.

For more information about authorization, see the following documents:

* [Authorize from a Companion App](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/authorize-companion-app.html)
* [Authorize from an AVS Product](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/authorize-on-product.html)
* [Authorize from a Companion Site](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/authorize-companion-site.html)

### Using the Authorization Platform Interface to Carry out Auth Provider Authorization
This section describes how an application uses the Authorization platform interface to carry out Auth Provider authorization.
The service name used by Authorization for Auth Provider authorization is `alexa:auth-provider`.

#### Starting Authorization	
To start authorization, the application uses the `startAuthorization` API.
The data parameter in the `startAuthorization` API is an empty string, as shown in the following example: 

~~~
startAuthorization( "alexa:auth-provider", "" )
~~~

>**Note:** With the Authorization platform interface, it is the responsibility of the application to start authorization at every Engine start. Each time the Engine is restarted, it does not automatically start the authorization that was previously in effect before the Engine restart.

#### Receiving Events from Engine
This section describes the protocol for the application to receive events from the Engine. The API used is `eventReceived`. 
The application needs to receive an event in these scenarios:

* when the Engine requests the application to start authorization
* when the application needs to log out of the authorization

To request the application to start authorization, the Engine passes the request to the application by using the event parameter in the `eventReceived` API. The parameter has the following JSON structure:

~~~
{
    "type": "requestAuthorization"
}
~~~

The application receives the event from the Engine through the `eventReceived` API as follows:

~~~
eventReceived( "alexa:auth-provider", "{"type":"requestAuthorization"}" )
~~~

To log out, the event parameter in the `eventReceived` API has the following JSON structure:

~~~
{
    "type": "logout"
}
~~~

The Engine calls the `eventReceived` method to log out the device as follows:

~~~
eventReceived( "alexa:auth-provider", "{"type":"logout"}" )
~~~

#### Sending Events to Engine
To notify the Engine of an authorization state change, the application uses the event parameter of the `sendEvent` API. The parameter has the following JSON structure:

~~~
{
    "type":"authStateChangeEvent",
    "payload": {
        "state":"{STRING}"
    }
}
~~~

The possible state strings are AUTHORIZED and UNAUTHORIZED.

The application needs to notify the Engine of an authorization state change for one of the following reasons:
* There is a request from Engine via `eventReceived` with `type` set to `requestAuthorization`.
* The application transitions to a new state, as shown in the following example where state transitions from `UNAUTHORIZED` to `AUTHORIZED`:

~~~
sendEvent( "alexa:auth-provider",
    "{
        "type":"authStateChangeEvent"
        "payload": {
            "state":"AUTHORIZED"
        }  
    }"
 )
~~~

#### Getting Access Token
The Engine calls the application to get the access token by using the `accessToken` parameter in the `getAuthorizationData` API as follows:

~~~
getAuthorizationData("accessToken")
~~~

The application responds with an access token, as shown in the following example.
~~~
{
    "accessToken":"Atza|AAAAAABBBBBBCCCCCC"
}
~~~

#### Canceling Authorization
To cancel authorization, the API to use is `cancelAuthorization`, as follows:

~~~
cancelAuthorization( "alexa:auth-provider" );
~~~

You may call this API to cancel the authorization that is started by calling `startAuthorization` but before sending an event via `sendEvent`. Canceling authorization does not affect the device authorization state.


#### Logging Out

The application makes the `logout` API call to the Engine to log out, as follows:

~~~
logout( "alexa:auth-provider" );
~~~

#### Handling Errors
The Engine notifies the application about any error during the authorization process.

The following list describes possible errors during authorization:

* `UNKNOWN_ERROR`: An unrecoverable error happens in the authorization process.
* `AUTH_FAILURE`: An invalid or expired access token is provided.
* `LOGOUT_FAILED`: The application cannot complete the logout process.
* `START_AUTHORIZATION_FAILED`: The authorization process cannot start.

The following example shows how the API handles an error when the provided access token is either expired or invalid:
~~~
authorizationError( "alexa:auth-provider", "AUTH_FAILURE", "" )
~~~

### Sequence Diagrams for Auth Provider Authorization

The following sequence diagram illustrates the flow when the Authorization platform interface starts an authorization.

<p align="center">
<img src="./assets/authprovider-start-sequence.png"/>
</p>

The following sequence diagram illustrates the flow when the Authorization platform interface cancels an authorization.

<p align="center">
<img src="./assets/authprovider-cancel-sequence.png"/>
</p>

The following sequence diagram illustrates the flow when the Authorization platform interface logs out of an authorization.

<p align="center">
<img src="./assets/authprovider-logout-sequence.png"/>
</p>

### Optional Auth Provider Configuration
By default, the Engine supports one Auth Provider authorization. However, if your application supports more than one, provide the Engine with the following configuration. You can also build the configuration programmatically by using [`createAuthProviderConfig`](./platform/include/AACE/Alexa/AlexaConfiguration.h).

```json
{
    "aace.alexa" : {
        "authProvider" : {
           "providers" : [<LIST_OF_PROVIDER_NAMES_STRINGS>]
        }
    }
}
```

For example, if your application supports two Auth Provider authorizations, named "serviceA" and "serviceB," provide the following configuration:


```json
{
    "aace.alexa" : {
        "authProvider" : {
           "providers" : ["serviceA" , "serviceB"]
        }
    }
}
```

With this configuration, the Engine uses the service names "serviceA" and "serviceB" with the Authorization platform interface instead of using the default Auth Provider service name `alexa:auth-provider`.


### (Deprecated) Implementing a Custom Handler for AuthProvider
>**Note:** The information in this section is not applicable if you use the Authorization platform interface. The AuthProvider platform interface is deprecated.

To implement a custom handler for authorization, extend the `AuthProvider` class:

```
#include <AACE/Alexa/AuthProvider.h>
class MyAuthProvider : public aace::alexa::AuthProvider {
    public:
        std::string getAuthToken() override {
            // provide current access token
        }
        
        AuthState getAuthState() override {
            // provide current authentication status
        }
        
        void authFailure(std::string token) override {
            // The access token most recently provided through AuthProvider::getAuthToken() is invalid and was used in an unauthorized request to AVS. AVS responded to this request with a 403 code indicating the token was not valid. Get a new token!
        }
              
};
...
        
// Register the platform interface with the Engine
auto m_authProvider = std::make_shared<MyAuthProvider>();
engine->registerPlatformInterface( m_authProvider );

...

// To notify the Engine of a change in authentication state (such as when the user logs in):
AuthState currentAuthState = AuthState::REFRESHED;
AuthError currentAuthError = AuthError::NO_ERROR;

m_authProvider->authStateChanged(currentAuthState, currentAuthError);
```

#### Switching User Account

When switching from one authorized user account to another, you must call the `authStateChanged(UNINITIALIZED)` API before providing access tokens for the second user.

>**NOTE:** Starting with the Auto SDK 3.1, you don't have to restart the Engine after a user logs out. Immediately after the `authStateChanged(UNINITIALIZED)` API returns, you may start a fresh Auth Provider authorization by calling the `authStateChanged(REFRESHED)` API.

#### AuthProvider Login/Logout Sequence Diagrams

The following diagram illustrates the login sequence when using the AuthProvider platform interface to obtain access and refresh tokens.

![AuthProvider_login](./assets/AuthProvider_login.png)

The following diagram illustrates the logout sequence when using the AuthProvider platform interface to clear access and refresh tokens.

![AuthProvider_logout](./assets/AuthProvider_logout.png)

## Handling Audio Output

When audio data is received from Alexa it is the responsibility of the platform implementation to read the data from the Engine and play it using a platform-specific audio output channel. It is also the responsibility of the platform implementation to define how each `AudioOutput` channel is handled. Each `AudioOutput` implementation will handle one or more of the following media types depending on the behavior defined in the `AudioOutputProvider`:

* TTS
* MUSIC
* NOTIFICATION
* ALARM
* EARCON
* COMMUNICATION

`AudioOutput` also has methods to control the volume of the audio channel. These methods allow the Engine to set the volume, either when the user asks Alexa to adjust it or if the Engine internally needs to adjust it during audio focus management. All local volume changes (initiated via button press or system-generated, for example) must be reported to the Engine through the `AlexaSpeaker` platform interface. For further details about the `AudioOutput` and `AudioOutputProvider` platform interfaces, please refer to the [Core module README](../core/README.md#implementing-audio).

### Custom Volume Control for Alexa Devices

You can use a custom volume control to support an Alexa device's native input volume range. By default, Alexa supports voice utterances that specify volume values between 0 and 10, but some devices may support a different range (i.e. 0 to 100). By placing on Amazon's allow list your Alexa device's volume range for your target platform, you can specify input volume levels per your device's range. Your device's input volume range is then mapped appropriately to the Alexa volume range.

Contact your Alexa Auto Solution Architect (SA) for help with allow lists. Placing a device on the allow list requires the following parameters:

* DeviceTypeID: <YOUR_DEVICE_TYPE_ID>
* Min: <YOUR_MIN_VOLUME_VALUE>
* Max: <YOUR_MAX_VOLUME_VALUE>

This does not impact the range used in the directives to the device. You must continue to use the SDK 0-100 volume range used by `AudioOutput` and `AlexaSpeaker` and map these values to the correct range in your implementation.

## Handling Alexa Speaker

The Alexa service keeps track of two device volume types: `ALEXA_VOLUME` and `ALERTS_VOLUME`. The `aace::alexa::AlexaSpeaker` class should be implemented by the platform to both set the volume and mute state of these two speaker types and allow the user to set the volume and mute state of these two speaker types locally via GUI as applicable. 

### SpeakerManager Configuration

`SpeakerManager` is now a configurable option, enabled by default. When not enabled, user requests to change the volume or mute now have an appropriate Alexa response, e.g. "Sorry, I can't control the volume on your device".

You can programmatically generate speaker manager configuration using the `aace::alexa::config::AlexaConfiguration::createSpeakerManagerConfig()` factory method, or provide the equivalent JSON values in a configuration file.
```
{
    "aace.alexa": {
        "speakerManager": {
            "enabled": false
        }
    }
}
```

## Handling Audio Player

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
 
## Handling Playback Controller Events

The Engine provides a platform interface `aace::alexa::PlaybackController` for the platform implementation to report on-device transport control button presses for media playing through Alexa. For example, if the user presses the on-screen pause button while listening to Amazon Music through Alexa's `AudioPlayer` interface, the platform implementation calls a `PlaybackController` method to report the button press to the Engine.

>**Note:** `PlaybackController` method calls to manage `AudioPlayer`'s state or playback queue proactively report button presses or the equivalent so that Alexa can react; they do *not* report changes to the playback state that happen locally first. The Alexa cloud manages the playback queue for `AudioPlayer` content, so each `PlaybackController` method call is a request for Alexa to act on the user's local request. The result of the request will come as one or more method invocations on the `AudioOutput` associated with the channel used for `AudioPlayer`.

> **Note:** If your implementation needs to stop `AudioPlayer` media in response to system events, such as audio focus transitions to audio playing outside the scope of the Auto SDK, use `PlaybackController` to notify the Engine of such changes. However, bear in mind that the expected usage of the interface does not change when it is used in this use case.
 
>**Note:** `PlaybackController` only controls media coming from Alexa, i.e. the `AudioPlayer`. `PlaybackController` should not be used the expectation of controlling playback for non-media Alexa audio sources like `SpeechSynthesizer` or Alexa-aware external media sources integrated with `ExternalMediaAdapter` or `LocalMediaSource`. Additionally, calling a `PlaybackController` method while audio is playing through another Alexa-aware external media source will produce unexpected results and is not recommended.

Whenever Alexa plays media through `AudioPlayer`, the Engine calls the platform interface method `aace::alexa::TemplateRuntime::renderPlayerInfo()` to provide visual metadata associated with the media that your implementation should render for the end user. The payload of this method includes descriptions of GUI controls to be displayed and the state in which to display them. When the user interacts with these on-screen controls, your implementation must use the `PlaybackController` interface to report the button presses to the Engine.

The table below maps the controls from the `renderPlayerInfo()` payload to the corresponding calls in `PlaybackController`.

| RenderPlayerInfo control name | PlaybackController | 
|:---|:--- |
| **PlaybackButton** | | 
| "PLAY_PAUSE" | PLAY | 
| "PLAY_PAUSE" | PAUSE | 
| "NEXT" | NEXT |
| "PREVIOUS" | PREVIOUS |
| "SKIP_FORWARD" | SKIP_FORWARD | 
| "SKIP_BACKWARD" | SKIP_BACKWARD |
| **PlaybackToggle** |
| "SHUFFLE" | SHUFFLE | 
| "LOOP" | LOOP | 
| "REPEAT" | REPEAT | 
| "THUMBS_UP" | THUMBS_UP |
| "THUMBS_DOWN" | THUMBS_DOWN |

To implement a custom handler for the playback controller, extend the `PlaybackController` class:

```
#include <AACE/Alexa/PlaybackController.h>
class MyPlaybackController : public aace::alexa::PlaybackController {
    ...

    void platformPlayButtonPressed() { // called by some platform event
          buttonPressed(PlaybackButton::PLAY);
    }          
    ...
      
    void platformScrubForwardGUIButtonPressed(){ //called by the platform on an available GUI button event
        buttonPressed(PlaybackButton::SKIP_FORWARD);
    }
    ...
    
    void platformShuffleGUIButtonPressed(){ //called by the platform on an available GUI toggle event
        togglePressed(PlaybackToggle::SHUFFLE, true); //the action should send the value opposing the last playerinfo state for that toggle control
    }
    ...  
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyPlaybackController>() );
```    

## Handling Equalizer Control

The Equalizer Controller enables Alexa voice control of the device's audio equalizer settings, which includes making gain level adjustments to any of the supported frequency bands ("BASS", "MIDRANGE", and/or "TREBLE") using the device's onboard audio processing. 

The platform implementation is responsible for the following:

* Determining how each supported band affects the audio
* Mapping Alexa's equalizer bands to the bands supported on the device, if they do not directly correspond
* Scaling Alexa's level values as necessary so that each step corresponds to one decibel of amplitude gain on the device
* Applying equalization to only selected portions of the audio output so that Alexa's speech, alarms, etc. will not be affected
* Persisting settings across power cycles

You can programmatically generate Equalizer Controller configuration with details such as supported bands, default state, and decibel adjustment range using the `aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig()` factory method, or provide the equivalent JSON values in a configuration file.

```
{
    "aace.alexa" {
        "equalizer": {
            "bands": {
                "BASS": true,
                "MIDRANGE": false,
                "TREBLE": true
            },
            "defaultState": {
                "bands": {
                    "BASS": 4,
                    "TREBLE": -1
                }
            },
            "minLevel": -6,
            "maxLevel": 6
        }
    }
}
```

// For example, 2 supported bands with amplitude gains ranging from -8dB to +8dB, each with a default of 0dB

```
auto eqConfig = aace::alexa::config::AlexaConfiguration::createEqualizerControllerConfig(
    {EqualizerBand::BASS, EqualizerBand::TREBLE},
    -8,
     8,
    { {EqualizerBand::BASS, 0}, {EqualizerBand::TREBLE, 0} } );
engine->configure( { //other config objects..., eqConfig, ... } );

...
```

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
## Handling Display Card Templates

Alexa sends visual metadata (display card templates) for your device to display. When template information is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the information that is received from Alexa. There are two display card template types:

* The [Template](https://alexa.design/DevDocRenderTemplate) type provides visuals associated with a user request to accompany Alexa speech.
* The [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo) type provides visuals associated with media playing through the `AudioPlayer` interface. This includes playback control buttons, which must be used with the `PlaybackController` interface.

You can programmatically generate template runtime configuration using the `aace::alexa::config::AlexaConfiguration::createTemplateRuntimeTimeoutConfig()` factory method, or provide the equivalent JSON values in a configuration file.

```
{
    "aace.alexa" {
        "templateRuntimeCapabilityAgent": {
            "displayCardTTSFinishedTimeout": <TIMEOUT_IN_MS>,
            "displayCardAudioPlaybackFinishedTimeout": <TIMEOUT_IN_MS>,
            "displayCardAudioPlaybackStoppedPausedTimeout": <TIMEOUT_IN_MS>
        }
}
```

To implement a custom handler for GUI templates, extend the `TemplateRuntime` class:

```
#include <AACE/Alexa/TemplateRuntime.h>
class MyTemplateRuntime : public aace::alexa::TemplateRuntime {
    public:
        void renderTemplate( const std::string& payload, FocusState focusState ) override {
        // handle rendering the template data specified in payload
        }
        
        void renderPlayerInfo( const std::string& payload, PlayerActivity audioPlayerState, std::chrono::milliseconds offset, FocusState focusState ) override {
        // handle rendering the player info data specified in payload
        }
};
...

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyTemplateRuntime>() );
```
>**Note:** In the case of lists, it is the responsibility of the platform implementation to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of the first five elements, Alexa prompts the user whether or not to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

## Handling External Media Apps

The External Media Player (EMP) Adapter allows you to declare and use external media application sources in your application. In order to interface with the EMP Adapter, you must use one of the following:

* A media connection client to interface the EMP Adapter to the external app. The Android Sample App supports an example external media connection client called the Media App Command and Control (MACC) client. Use the MACC client to interface with the Spotify app running on Android. For details about the MACC client, see the Android version of the [Alexa module README](../../platforms/android/modules/alexa#handling-external-media-adapter-with-maccandroidclient).
* An embedded media app. For information about external embedded media app solutions, contact your SA or Partner Manager.

***NOTE: If the media app service requires additional customer experience details, incorporate the requirement in your implementation. For example, if the provider requires your application to show the provider's logo in a particular way, modify the implementation to meet the requirement.*** 

When advised by your SA or Partner Manager, configure the External Media Player Adapter to the device's capabilities. See `aace::alexa::config::AlexaConfiguration::createExternalMediaPlayerConfig` for details on configuring the supported agent, or provide the equivalent JSON values in a configuration file.

```
{
    "aace.alexa": {
        "externalMediaPlayer": {
            "agent": "<agent>"
     
  }
}
```

You must register and implement each ExternalMediaAdapter (along with its associated external client or library). After the engine establishes a connection to the Alexa service, you can run discovery to validate each external media application. You can report discovered external media players by calling `reportDiscoveredPlayers()` at any point during runtime. When the Alexa service recognizes the player, you will get a call to the `authorize()` method including the player's authorization status. Both the `reportDiscoveredPlayers()` method and the `authorize()` method can contain one or more players in their JSON payloads. Validating the application enables Alexa to exercise playback control over the registered source type. 

The `login()` and `logout()` methods inform AVS of login state changes, if applicable. If your application has the ability to handle cloud-based login and logout, you should also call the `loginComplete()` and `logoutComplete()` methods where appropriate. 

When the user makes an Alexa voice request (for example, "Play Spotify"), the `play()` method is invoked. This method contains various parameters, including the player id of the player to which the playback information should be routed. 

Whether through voice or GUI event, the `playControl()` method is called with the relevant `PlayControlType`. Similar to `play()` the control should be routed to the appropriate player.

The `PlayControlType` is determined by player's `supportedOperations`, which are specified by your implementation in the return value of `getState()`. 

The `ExternalMediaAdapter` interface provides methods `playerEvent()` and `playerError()` for your implementation to report events regarding the state of the playback session managed by your external player. Even though your player manages its own playback, including reacting to on-device transport control button presses from the user and reacting appropriately to other non-Alexa audio events on the system, the `playerEvent()` and `playerError()` calls provide important information to the Engine:
    
1. The Engine may use calls to these methods to synchronize the state of your player’s playback session with Alexa.

2. The Engine may react to these calls according to the event name specified to update its internal view of your player’s state. Particular event names indicate if the player is focused on the system (meaning it has an active playback session) or if it is un-focused (meaning it is not in use and is brought into use only by further on-device interaction by the user or a user voice request to Alexa). The Engine uses this information to sync its internal focus management.

The tables below describe each supported event name and what it means to the Engine. Usage of these events depends on the particular type of player controlled by the `ExternalMediaAdapter` instance, so contact your Solutions Architect (SA) or Partner Manager for guidance regarding supported embedded and external app solutions.

| playerEvent() event name | Description |
|:--|:--|
| "PlaybackSessionStarted" | A new playback session has started, either from a GUI interaction or as a result of a user voice request to Alexa. The Engine considers the player active and in focus (although it may or may not yet be playing). |
| "PlaybackStarted" | During an active session, the player has started to play or resumed from a paused state. The Engine considers the player active and in focus. |
| "TrackChanged" | During an active session, one track has ended and another has started. The Engine uses this primarily for state reporting.  |
| "PlaybackNext" | During an active session, the player skipped from one track to the next track, either as a result of a GUI interaction or a user voice request to Alexa. The Engine uses this primarily for state reporting. |
| "PlaybackPrevious" | During an active session, the player skipped from one track to the previous track, either as a result of a GUI interaction or a user voice request to Alexa. The Engine uses this primarily for state reporting. |
| "PlayModeChanged" | During an active session, some user setting for the track or playback session changed, such as the favorite setting or the shuffle mode. The Engine uses this primarily for state reporting.|
| "PlaybackStopped" | During an active session, the player has paused or stopped, either as a result of a GUI interaction or a user voice request to Alexa. The Engine considers the player active and in focus, just not currently playing. User voice requests to resume still control the player. |
| "PlaybackSessionEnded" | An active playback session has ended. The player should no longer be playing or playable until a new session is started by GUI interaction or user voice request to Alexa. The Engine considers the player inactive and no longer in focus. |

| playerError() event name | Description |
|:--|:--|
| "INTERNAL\_ERROR" | Any fatal player error has occurred
| "UNKNOWN\_ERROR" | An unknown error occurred
| "UNPLAYABLE\_BY\_AUTHORIZATION" | The media couldn't be played due to an unauthorized account
| "UNPLAYABLE\_BY\_STREAM\_CONCURRENCY" | The media couldn't be played due to the number of accounts currently streaming
| "UNPLAYABLE\_BY\_ACCOUNT" | The media couldn't be played due to the account type
| "UNPLAYABLE\_BY\_REGION" | The media couldn't be played due to the current region
| "UNPLAYABLE\_BY\_PARENTAL\_CONTROL" | The media couldn't be played due to parental settings
| "UNPLAYABLE\_BY\_SUBSCRIPTION" | The media couldn't be played due to the subscription type 
| "OPERATION\_REJECTED\_UNINTERRUPTIBLE" | The operation could not be performed due to non interruptible media
| "OPERATION\_REJECTED\_END\_OF\_QUEUE" | The operation could not be performed due to the end of media being reached
| "OPERATION\_UNSUPPORTED" | The operation was not supported
| "OPERATION\_REJECTED\_SKIP\_LIMIT" | The operation failed because a skip limit was reached 
| "PLAYER\_UNKNOWN" | An unknown player was detected
| "PLAYER\_NOT\_FOUND" | The player was not discovered
| "PLAYER\_CONNECTION\_REJECTED" | The connection to the player failed
| "PLAYER\_CONNECTION\_TIMEOUT" | The connection to the player timed out

The `seek()` and `adjustSeek()` methods are invokable via Alexa if the currently in-focus external player supports them. `seek()` specifies an absolute offset, whereas `adjustSeek()` specifies a relative offset. 

The `volumeChanged()` and `mutedStateChanged()` methods are invoked to change the volume and mute state of the currently-focused external player. `volumeChanged()` specifies the new volume. `mutedStateChanged()` specifies the new `MutedState`.

The `getState()` method is called to synchronize the external player's state with the cloud. This method is used to maintain correct state during startup, and after every Alexa request. 

You construct the `ExternalMediaAdapterState` object using the data taken from the media app connection client or embedded player app (associated via `localPlayerId`) and return the state information.

The following table describes the fields comprising a `ExternalMediaAdapterState`, which includes two sub-components: `PlaybackState`, and `SessionState`.

| State        | Type           | Required | Notes  |
| :------------- |:-------------| :-----| :-----|
| **PlaybackState**      |
| state      | String        | Yes | "IDLE"/"STOPPED"/"PLAYING" |
| supportedOperations | SupportedPlaybackOperation[] | Yes | see SupportedOperation |
| trackOffset      | long  | No |  optional |
| shuffleEnabled      | boolean       |  Yes | report shuffle status |
| repeatEnabled      | boolean       |  Yes | report repeat status |
| favorites      | Favorites  | No | see Favorites |
| type      | String  |  Yes | must be set as "ExternalMediaPlayerMusicItem" |
| playbackSource      | String       | No |   If available else use local player name |
| playbackSourceId      | String  | No |  empty |
| trackName      | String   | No |  If available else use local player name |
| trackId      | String    |  No | empty |
| trackNumber      | String   | No | optional |
| artistName      | String    | No | optional |
| artistId      | String   |  No | empty |
| albumName      | String |  No | optional |
| albumId      | String | No |  empty |
| tinyURL      | String | No |  optional |
| smallURL      | String | No |  optional |
| mediumURL      | String | No |  optional |
| largeURL      | String | No |  optional |
| coverId      | String  |  No | empty |
| mediaProvider      | String  | No |  optional |
| mediaType      | MediaType | Yes |  see MediaType |
| duration      | long  | No |  optional |
| **SessionsState** |
| endpointId      | String  | No |  empty |
| loggedIn      | boolean  | No |  empty |
| userName      | String  |  No | empty |
| isGuest      | boolean  | No |  empty |
| launched      | boolean  | Yes |  true if the source is enabled, false otherwise |
| active      | boolean  | Yes |  true if the application is in an active state  |
| accessToken      | String  |  No | empty |
| tokenRefreshInterval      | long  | No |   empty |
| playerCookie      | String  | No |  A player may declare arbitrary information for itself |
| spiVersion      | String  |  Yes | must be set as "1.0" |


`supportedOperations` should be a list of the operations that the external media adapter supports. Below is a list of all possible `supportedOperations`.

```
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
SupportedPlaybackOperation.ADJUST_SEEK,
SupportedPlaybackOperation.FAVORITE,
SupportedPlaybackOperation.UNFAVORITE,
SupportedPlaybackOperation.FAST_FORWARD,
SupportedPlaybackOperation.REWIND,
SupportedPlaybackOperation.START_OVER
```
>**Note:** Currently PLAY/PAUSE/STOP will always be supported for a source. Passing null will allow ALL supported operations for the source. 

## Handling Local Media Sources

The `LocalMediaSource` interface allows the platform to register a local media source by type (`BLUETOOTH`, `USB`, `AM_RADIO`, `FM_RADIO`, `SATELLITE_RADIO`, `LINE_IN`, `COMPACT_DISC`, `SIRIUS_XM`, `DAB`). Registering a local media source allows playback control of that source via Alexa (e.g. "Alexa, play the CD player"). It also enables playback initiation via Alexa by frequency, channel, or preset for relevant source types (e.g. "Alexa, play 98.7 FM").

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
    MyCDLocalMediaSource( LocalMediaSource::Source source ) {
        m_source = source;	
        ...
    }
    ...
}; 
... 
```

The `play()` method is called when Alexa invokes play by `ContentSelector` type (`FREQUENCY`, `CHANNEL`, `PRESET`) for a radio local media source (`AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`, `DAB`). The `payload` is a string that depends on the `ContentSelector` type and local media `Source` type (e.g., "1", "98.7 FM HD 1").

```	
bool play( ContentSelector type, std::string payload ) override {
    // play initiation for frequency, channel, or presets
    ...
}
``` 
The table below provides details about the supported `ContentSelector` types based on `Source` type:

| Source type | Supported content selector(s) |
|:------|:---------|
| FM | FREQUENCY, PRESET | 
| AM | FREQUENCY, PRESET | 
| SXM | CHANNEL, PRESET | 
| DAB | CHANNEL |

The support, ranges and increments for valid frequency, preset, and channel may vary, depending on the region you are in. Contact your partner manager for more detailed information. 

>**Note:** The `DAB` channel payload is the radio station name string. If supported, then the name string must be handled by the client's DAB implementation.

The `play()` method will not be invoked if a source cannot handle the specified `ContentSelector` type.

>**Note:** Cases in which a preset is requested but the source is unknown are handled by the `GlobalPreset` interface. See [Handling Global Presets](#handling-global-presets) for details.

The `playControl()` method is called with a `PlayControlType`(`RESUME`, `PAUSE`, `STOP`, `NEXT`, `PREVIOUS`, `START_OVER`, `FAST_FORWARD`, `REWIND`, `ENABLE_REPEAT_ONE`, `ENABLE_REPEAT`, `DISABLE_REPEAT`, `ENABLE_SHUFFLE`, `DISABLE_SHUFFLE`, `FAVORITE`, `UNFAVORITE`) when Alexa invokes a playback control on the local media source.

```	
bool playControl( PlayControlType controlType ) override {
    // handle the control type appropriately for CD player
    return true;
}
```

>**Note:** The `play()` method is used to initiate playback with specified content selection, whereas `playControl(RESUME)` is used to play or resume the source that is already in focus.

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

Use the `setFocus()` method to tell the Engine that the `LocalMediaSource` instance has become focused or un-focused on the system. The Engine already considers a source active when the user requests Alexa to play the source by name, so this should only be used when the state of the source is changed by a user GUI interaction.

Call `setFocus(true)` to tell the Engine that the user brought the `LocalMediaSource` to the foreground with a GUI interaction. The Engine considers the source to have an active playback session, although it may or may not be playing yet. If no other Alexa media source is playing, utterances such as “Alexa, play” target this source.

Call `setFocus(false)` to tell the Engine that the `LocalMediaSource` is no longer in the foreground, typically as a result of a GUI interaction from the user after the player is stopped. The Engine considers the source inactive, and starting a new playback session for the source requires a further GUI interaction or user voice request to Alexa that targets the source by name.

```	
class MyFMRadioLocalMediaSource : public aace::alexa::LocalMediaSource {
...
    // public method in source handler	
    void setAlexaFocusForFMRadio( bool isFocused ) {  
    ...  
    	// FM Radio begins playback independently of Alexa
       	setFocus( true ); // or setFocus();
    ...    
     	// Notify Alexa that FM Radio is no longer the active media source on the device as a result of platform driven change
       	setFocus( false );
...    
```
>**Note:** Only one `LocalMediaSource` type can have Alexa focus at a time.

The `LocalMediaSource` interface provides methods `playerEvent()` and `playerError()` for your implementation to report events regarding the state of the playback session managed by your local source. Even though your local source manages its own playback, including reacting to on-device transport control button presses from the user and reacting appropriately to other non-Alexa audio events on the system, the  `playerEvent()` and `playerError()` calls in conjunction with `setFocus()` provide important information to the Engine:
    
1. The Engine may use calls to these methods to synchronize the state of your local source's playback session with Alexa.

2. The Engine may react to these calls according to the event name specified to update its internal view of your local source's state. Particular event names indicate if the source is focused on the system (meaning it has an active playback session) or if it is un-focused (meaning it is not in use and is brought into use only by further on-device interaction by the user or a user voice request to Alexa). The Engine uses this information to sync its internal focus management.


| playerEvent() event name | Description |
|:--|:--|
| "PlaybackStarted" |  During an active session, the local source has started to play or resumed from a paused state. The Engine considers the source active and in focus. |
| "PlaybackStopped" | During an active session, the player stopped, either as a result of a GUI interaction or a user voice request to Alexa. |

| playerError() event name | Description |
|:--|:--|
| "INTERNAL\_ERROR" | During an active session, an internal error caused playback to stop.

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

| State        | Type           | Required | Notes  |
| :------------- |:-------------| :-----| :-----|
| **PlaybackState**      | 
| state      | String        |  Yes | "IDLE"/"STOPPED"/"PLAYING" |
| supportedOperations | SupportedPlaybackOperation[] | Yes | see SupportedPlaybackOperation |
| trackOffset      | long  |  No | optional | 
| shuffleEnabled      | boolean       |   No | optional |
| repeatEnabled      | boolean       |   No | optional |
| favorites      | Favorites  |   No  | see Favorites |
| type      | String  |   Yes | must be set to "ExternalMediaPlayerMusicItem" |
| playbackSource      | String       |  No | If available else use local player name |
| playbackSourceId      | String  |   No | optional |
| trackName      | String   | No | If available else use local player name |
| trackId      | String    | No |  empty |
| trackNumber      | String   | No | optional |
| artistName      | String    | No | optional |
| artistId      | String   | No |  empty |
| albumName      | String |  No | optional |
| albumId      | String |  No | empty |
| tinyURL      | String | No | optional |
| smallURL      | String | No |  optional |
| mediumURL      | String | No |  optional |
| largeURL      | String | No |  optional |
| coverId      | String  | No |  empty |
| mediaProvider      | String  | No | optional |
| mediaType      | MediaType | No |  see MediaType |
| duration      | long  | No |  optional |
| **SessionsState** | 
| endpointId      | String  | No |  empty |
| loggedIn      | boolean  |  No | empty |
| userName      | String  | No |  empty |
| isGuest      | boolean  | No |  empty |
| launched      | boolean  |  Yes | true if the source is enabled, false otherwise |
| active      | boolean  | No |  empty |
| accessToken      | String  | No |  empty |
| tokenRefreshInterval      | long  | No |  empty |
| supportedContentSelectors      | ContentSelector[]  | No | see ContentSelector |
| spiVersion      | String  |  Yes | must be "1.0"  |

`supportedOperations` should list the operations which the local media source supports. Below is a list of all `SupportedPlaybackOperation`:

```
LocalMediaSource::SupportedPlaybackOperation::PLAY,
LocalMediaSource::SupportedPlaybackOperation::PAUSE,
LocalMediaSource::SupportedPlaybackOperation::STOP,
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
>**Note:** Currently PLAY/PAUSE/STOP are always supported for a source. Passing null allows ALL supported operations for the source. 


`supportedContentSelectors` should list the content selection types the local source can support. Below is a table of valid pairs.

| Source | Supportable `ContentSelector` Values |
|:---|:---|
| `AM_RADIO` |  `PRESET`, `FREQUENCY` |
| `FM_RADIO` |  `PRESET`, `FREQUENCY` |
| `SIRIUS_XM` |  `PRESET`, `CHANNEL` |

`launched` specifies whether the source is enabled. The player is disabled for use with Alexa when this value is false, such as when a removable source like USB is disconnected.

## Handling Global Presets

The Global Preset interface handles "Alexa, play preset \<number>\" utterances without requiring that users explicitly say which local media source (`AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`) actually corresponds to the preset.

The meaning of the `preset` passed through `setGlobalPreset()` is determined by the `GlobalPreset` platform implementation registered with the Engine and should suit the needs of the vehicle's infotainment system. When the `setGlobalPreset()` method is called, your implementation should map the preset to a preset that makes sense for your current client context.

You must register a `GlobalPreset` implementation with the Engine in order for Alexa to set presets for any local media `Source` type that may use them (e.g. `AM_RADIO`, `FM_RADIO`, `SIRIUS_XM`). 

```
#include <AACE/Alexa/GlobalPreset.h>
...
class MyGlobalPresetHandler : public aace::alexa::GlobalPreset {	
public:
void setGlobalPreset( int preset ) override {
    // handle the preset, via routing to local media source
}
...
```    

## Handling Notifications

It is the responsibility of the platform implementation to provide a visual indication to the user when notifications (for example, package shipment notifications, notifications from skills, etc.) are available from Alexa. See the [AVS Notifications interface documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/notifications.html) for more information about notifications. The Engine uses the registered Notifications implementation to notify you when a notification indicator should be displayed or removed. It does not give any information about the notifications. Audio playback for the notification is handled by whichever audio channel is assigned to the `NOTIFICATION` type.

To implement a custom handler for Notifications extend the `Notifications` class:

```
#include <AACE/Alexa/Notifications.h>
using IndicatorState = aace::alexa::Notifications::IndicatorState;
class MyNotificationsHandler : public aace::alexa::Notifications {

  public:
    void setIndicator( IndicatorState state ) override {
        // set your notifications indicator!
    }

};
...

// Register the platform interface with the Engine
auto m_notificationsHandler = std::make_shared<MyNotificationsHandler>();
engine->registerPlatformInterface(m_notificationsHandler);

```

## Handling Alerts

When an alert is received from Alexa, it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player. See the [AVS Alerts interface documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/alerts.html) for more information about alerts. The state of the alert is also made available for the platform to react to. The playback is handled by whichever audio channel is assigned to the `ALERT` type.

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

## Handling Do Not Disturb

The DoNotDisturb (DND) interface allows users to block all incoming notifications, announcements, and calls to their devices, and to set daily recurring schedules that turn DND off and on.  For details, see the [DND Interface documentation](https://developer.amazon.com/docs/alexa-voice-service/donotdisturb.html). The Engine uses the registered DND implementation to notify the client when DND has been set or unset. A user's voice request to change the DND state triggers audio playback, but no audio playback occurs when a user sets the DND state using the touch screen.

To implement a custom handler for DND extend the `DoNotDisturb` class:

```
#include <AACE/Alexa/DoNotDisturb>

class MyDoNotDisturbHandler : public aace::alexa::DoNotDisturb {

  public:
    void setDoNotDisturb( bool doNotDisturb ) override {
        // set your DoNotDisturb indicator
    }
    // on user GUI setting change
    ...
        bool doNotDisturb = userSetState;
        doNotDisturbChanged(doNotDisturb);
    ...

};
...

// Register the platform interface with the Engine
auto m_doNotDisturbHandler = std::make_shared<MyDoNotDisturbHandler>();
engine->registerPlatformInterface(m_doNotDisturbHandler);

```
    
## Handling Alexa State Changes

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