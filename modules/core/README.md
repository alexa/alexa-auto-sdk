# Core Module


The Alexa Auto SDK Core module contains the Engine base classes and the abstract platform interfaces that can be utilized by the platform and/or other modules. It also provides an easy way to integrate the Alexa Auto SDK into an application or a framework. This involves configuring and creating an instance of `aace::core::Engine`, overriding default platform implementation classes, and registering the custom interface handlers with the instantiated Engine.

**Table of Contents:**

* [Creating the Engine](#creating-the-engine)
* [Configuring the Engine](#configuring-the-engine)
* [Registering Platform Interface Handlers](#registering-platform-interface-handlers)
* [Starting the Engine](#starting-the-engine)
* [Extending the Default Platform Implementation](#extending-the-default-platform-implementation)
* [Core Engine Properties](#core-engine-properties)

## Creating the Engine <a id="creating-the-engine"></a>

You create an instance of the Engine by calling the static function `aace::core::Engine::create()`:

    std::shared_ptr<aace::core::Engine> engine = aace::core::Engine::create();

## Configuring the Engine <a id="configuring-the-engine"></a>

Before the Engine can be started, you must configure it using the required `aace::core::config::EngineConfiguration` object(s) for the services you will be using. You can specify the configuration data using a JSON file or programmatically.

### Specifying Configuration Data Using a JSON File

The SDK provides classes for reading the configuration data from a JSON file:
    `aace::core::config::ConfigurationFile::create("AlexaClientSDKConfig.json`   

The class `aace::core::config::ConfigurationFile` creates an Engine configuration object from a file:

(example `.json` config file)

```
{
    "deviceInfo": {
        "deviceSerialNumber": "<DEVICE_SERIAL_NUMBER>",
        "clientId": "<CLIENT_ID>",
        "productId": "<PRODUCT_ID>"
    },
    "certifiedSender": {
        "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
    },
    "alertsCapabilityAgent": {
        "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>"
    },
    "settings": {
        "databaseFilePath": "<SQLITE_DATABASE_FILE_PATH>",
        "defaultAVSClientSettings": {
        "locale": "en-US"
        }
    }
}
```

You can include all of the configuration data for the Engine in the JSON file or break it up into several files with multiple `aace::core::config::ConfigurationFile` objects.

### Specifying Configuration Data Programmatically

You can also specify the configuration data programmatically by using the configuration factory methods provided in the library. For example, you can configure the `alertsCapabilityAgent` settings by instantiating a configuration object with the following method:

```
auto alertsConfig = aace::alexa::config::AlexaConfiguration::createAlertsConfig
    ("<SQLITE_DATABASE_FILE_PATH>" );
```

After you have created your configuration object(s) you should call the Engine's `configure()` function, passing in the configuration object(s): 

`engine->configure( config );`

OR

`engine->configure( { deviceInfoConfig, alertsConfig, ... } );`

> **Note**: The Engine's `configure()` method can only be called once and must be called before registering any platform interfaces or starting the Engine.

### Vehicle Information Requirements

Vehicle configuration information should be supplied to the SDK through regular Engine configuration. A sample configuration is detailed below. You can generate the `aace::core::config::EngineConfiguration` object including this information by using this schema in a `.json` config file or programmatically through the `VehicleConfiguration::createVehicleInfoConfig()` factory method.

The following JSON example lists all of the information that can be used to enhance the Alexa experience in the car.

```
{
  "aace.vehicle":
  {
     "info": {
         "make": "<MAKE>",
         "model": "<MODEL>",
         "year": "<YEAR>",
         "trim": "<TRIM>",
         "geography": "<GEOGRAPHY>",
         "version": "<SOFTWARE_VERSION>",
         "os": "<OPERATING_SYSTEM>",
         "arch": "<HARDWARE_ARCH>",
         "language": "<LANGUAGE>",
         "microphone": "<MICROPHONE>"
         "countries": "<COUNTRY_LIST>",
         "vehicleIdentifier": "<VEHICLE_IDENTIFIER>"
     }
  }
}
```
The vehicle information should be provided with a unique vehicle identifier that is not the vehicle identification number (VIN). This information is required to pass the certification process. Read detailed information about the [vehicle configuration class](platform/include/aace/Vehicle/VehicleConfiguration.h) in the API reference documentation.

### Update Locale Setting

If the configuration database files have already been created, you can update the `locale` setting in either of these ways:

* Delete the SQLite database files to force an update of all the configuration settings, or
* Use the `setProperty()` method in the Engine Class: 

```
engine->setProperty(aace::alexa::property::LOCALE,"<LOCALE_STRING>");
```

If the property value was updated, the `setProperty()` method returns true. If the update failed, the `setProperty()` method returns false.

## Registering Platform Interface Handlers <a id="registering-platform-interface-handlers"></a>

The Engine class provides two methods for registering platform interface handlers, which allows you to register one or more interfaces at a time for convenience:

```
class MyInterface : public SpeechRecognizer {
...

    engine->registerPlatformInterface( std::make_shared<MyInterface>() );

OR

    std::shared_ptr::MyInterface1 myInterface1 =
    std::make_shared<MyInterface1>();
    std::shared_ptr::MyInterface2 myInterface2 =
    std::make_shared<MyInterface2>();
    engine->registerPlatformInterface({ myInterface1, myInterface2 });
```

For details about extending the default platform interfaces, see [Extending the Default Platform Implementation](#extending-the-default-platform-implementation).

## Starting the Engine <a id="starting-the-engine"></a>

After you have created and configured the Engine and registered all of the platform interfaces, you must call the `Engine::start()` method:

`engine->start();`

## Extending the Default Platform Implementation <a id="extending-the-default-platform-implementation"></a>

You can extend the default Alexa Auto SDK platform implementation by overriding the various classes in the library that, when registered with the Engine, allow you to interact with Amazon services.

### Implementing a LocationProvider

The Engine provides a callback for implementing location requests from Alexa and other modules and a Location type definition. This is optional and dependent on the platform implementation.

To implement a custom `LocationProvider` handler to provide location using the default Engine `LocationProvider` class, extend the `LocationProvider` class:

```
#include <AACE/Location/LocationProvider.h>

class MyLocationProvider : public aace::location::LocationProvider {

  Location getLocation() override {
    // get platform location
    return m_platformLocation;
  }
  ...

  m_platformLocation = aace::location::Location(...);
};

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyLocationProvider>());
```

### Implementing NetworkInfoProvider

The Engine provides callbacks for implementing network information requests and informing the Engine of network changes. This is dependent on the platform implementation.

> **Important!** Connectivity monitoring is the responsibility of the platform. The Alexa Auto SDK doesn't monitor network connectivity.

To implement a custom `NetworkInfoProvider` handler to provide network information using the default Engine NetworkInfoProvider class, extend the `NetworkInfoProvider` class:

```
#include <AACE/Network/NetworkInfoProvider.h>

class MyNetworkInfoProvider : public aace::network::NetworkInfoProvider {

    NetworkStatus getNetworkStatus() override {
        // get platform network status
        return platformStatus;
    }
  
    int getWifiSignalStrength() override{
        // get current network RSSI
        return platformRSSI;
    }
    ...

    void MyNetworkStatusChangedHandler(...) {
        //provide platform network status inform
        int rrsi;
        NetworkStatus status;
        networkStatusChanged( status, rssi);
    }
    ...
};

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyNetworkInfoProvider>());
```

### Implementing Log Events

The Engine provides a callback for implementing log events from the Auto SDK. This is entirely optional for the platform implementation.

To implement a custom `Logger` event handler for logging events from Auto SDK, extend the `Logger` class:

```
#include <AACE/Logger/Logger.h>

class MyLogger : public aace::logger::Logger {

  void logEvent(aace::logger::Logger::Level level, std::chrono::system_clock::time_point time, const std::string& source, const std::string& message) override {
    //handle the log message
  };
  ...

};

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyLogger>());
```        
    
### Implementing Audio

The platform should implement audio input and audio output handling. Other Auto SDK components can then make use of the provided implementation to provision audio input and output channels. 
  
The `AudioInputProvider` should provide a platform-specific implementation of the `AudioInput` interface, for the type of input specified by the `AudioInputType` parameter when its `openChannel()` method is called, with only one instance of `AudioInput` per `AudioInputType`. For example, the `SpeechRecognizer` engine implementation requests an audio channel for type `VOICE`, while AlexaComms requests a channel for type `COMMUNICATION` - the implementation determines if these are shared or separate input channels. If it is a shared input channel, then whenever the platform implementation writes data to the interface, any instance in the Engine that has opened that channel will receive a callback with the audio data.
  
```
#include <AACE/Audio/AudioInputProvider.h>
...
class AudioInputProviderHandler : public aace::audio::AudioInputProvider {
public:
    std::shared_ptr<aace::audio::AudioInput> openChannel(
        const std::string& name, AudioInputType type ) override {
        // Create a shared AudioInput for VOICE and COMMUNICATION input channels. In this
        // use case both SpeechRecognizer and AlexaComms are sharing a single microphone.
        if( type == AudioInputType::VOICE || type == AudioInputType::COMMUNICATION ) {
            if( m_audioInputHandler == nullptr ) {
                m_audioInputHandler = AudioInputHandler::create();
            }
            return m_audioInputHandler;
        }
        // If a loopback audio can be provided then return the AudioInput implementation
        // for the loopback audio source.
        else if( type == AudioInputType::LOOPBACK ) {
            if( m_loopbackInputHandler == nullptr ) {
                m_loopbackInputHandler = LoopbackInputHandler::create();
            }
            return m_loopbackInputHandler;
        }
        else {
            return nullptr;
        }
    }
    ...
};
```

The `AudioInput` interface is required to implement platform-specific support for providing audio data from a specific input channel when requested by the Auto SDK. When a request for audio input is made (for example tap-to-talk from `SpeechRecognizer`), the `AudioInput`'s `startAudioInput()` method is called and the implementation must start writing audio data until `stopAudioInput()` is called. 

In the case where two components in the Auto SDK both request audio from the same channel, `startAudioInput()` will only be called when the first component requests the audio. All of the components that have requested audio from the input channel will receive a callback when audio data is written to the interface, until they explicitly cancel the request. The `stopAudioInput()` method will only be called after the last component has canceled its request to receive audio from the channel.

The audio input format for all input types should be encoded as:

* 16bit Linear PCM

* 16kHz sample rate

* Single channel

* Signed, little endian byte order

```
#include <AACE/Audio/AudioInput.h>
...
class AudioInputHandler : public aace::audio::AudioInput {
public:
    bool startAudioInput() override
    {
        // start receiving audio data from the platform specific input device
        m_device.start( [this](const int16_t* data, const size_t size) {
            // provide the audio data to engine by calling the
            // AudioInput write() method...
            write( data, size );
        });
    }
 
    bool stopAudioInput() override {
        // stop requesting audio data from the platform specific input device
        m_device.stop();
    }
    ...
};
```

The `AudioOutputProvider` provides a platform-specific implementation of the `AudioOutput` interface, for the type of input specified by the `AudioOutputType` parameter, when its `openChannel()` method is called. The `AudioOutputProvider` should create a new instance of `AudioOutput` each time `openChannel()` is called. The `openChannel()` method will be called from components in the Auto SDK that require support for playing back audio. The characteristics of the audio that will be played on the channel are specified by the `AudioOutputType` parameter. The following types are currently defined by `AudioOutputType`:

 * TTS
 * MUSIC
 * NOTIFICATION
 * ALARM
 * EARCON
 * COMMUNICATION

The `AudioOutputProvider` implementation determines how to handle each different audio type. The simplest implementation, for example, may ignore the audio type and return the same `AudioOutput` implementation for all channels. 

A more sophisticated implementation may provide completely different `AudioOutput` implementations depending on the audio type - for example, providing a low level audio implementation for `NOTIFICATION` and `EARCON` types, and a high level implementation (such as ExoPlayer) for `TTS` and `MUSIC`. The best approach is highly dependent on your system-specific use case.

```
#include <AACE/Audio/AudioOutputProvider.h>
...
class AudioOutputProviderHandler : public aace::audio::AudioOutputProvider {
public:
    std::shared_ptr<aace::audio::AudioOutput> openChannel(
        const std::string& name, AudioOutputType type ) override {
        // Simple implementation which returns a new instance of AudioOutputHandler
        // whenever openChannel() is called. All types will have the sample implementation.
        return AudioOutputHandler::create();
    }
    ...
};
```

The `AudioOutput` describes a platform-specific implementation of an audio output channel. The platform should have one or more implementations of `AudioOutput`, depending on on the desired behavior. In addition to implementing the callback methods, each `AudioOutput` channel implementation should report the state of its media, when appropriate. 

The full `AudioOutput` API is described below. 

```
#include <AACE/Audio/AudioOutput.h>
class AudioOutputHandler :
    public aace::audio::AudioOutput,
    public std::enable_shared_from_this<AudioOutputHandler> {
    
    bool prepare( std::shared_ptr<aace::audio::AudioStream> stream, bool repeating ) override{
        ... // tell the platform media player to prepare audio stream
        mediaStateChanged( MediaState::BUFFERING );
        m_player->prepareStream( stream, repeating );
    ...
    
    bool prepare( const std::string& url, bool repeating ) override {
        ... // tell the platform media player to prepare the url
        mediaStateChanged( MediaState::BUFFERING ); 
        m_player->prepareUrl( url, repeating );
    ...
 
    bool play() override {
        ... // tell the platform media player to start playing
        mediaStateChanged( MediaState::PLAYING );
        m_player->play();
    ...
 
    bool stop() override {
        ... // tell the platform media player to stop playing
        mediaStateChanged( MediaState::STOPPED );
        m_player->stop();
    ...
 
    bool pause() override {
        ... // tell the platform media player to pause playback
        mediaStateChanged( MediaState::STOPPED );
        m_player->pause();
    ...
 
    bool resume() override {
        ... // tell the platform media player to resume playback
        mediaStateChanged( MediaState::PLAYING );
        m_player->resume();
    ...
 
    int64_t getPosition() override {
        ... // return the current media position of the platform media player
        m_player->position();
    ...
 
    bool setPosition( int64_t position ) override {
        ... // set the current media position of the platform media player
        m_player->setPosition( position );
    ...
    
    int64_t getDuration() override {
        ... // return the current media duration of the platform media player
        return m_player.getCurrentDuration();
    ...
    
    bool volumeChanged( float volume ) override {
        ... // set the current media volume of the platform media player
        m_player->setVolume( volume );
    ...

    bool mutedStateChanged( MutedState state ) override {
        ... // set the current media mute state of the platform media player    
        if( state == MutedState::MUTED ) m_player->setMuted( true );
        else m_player->setMuted( false );
    ...

};
```

  
## Core Engine Properties <a id="core-engine-properties"></a>

The Core module defines several constants that are used to get and set runtime properties in the Engine. To use these properties, include the `CoreProperties.h` header in your source code and call the Engine's `getProperty()` and `setProperty()` methods.

    #include <AACE/Core/CoreProperties.h>

    // get the SDK version number from the Engine
    auto version = m_engine->getProperty( aace::core::property::VERSION );

The following constants are defined in the Core module:

<table>
<tr>
<th>Property</th>
<th>Description</th>
</tr>
<tr>
<td><code>aace::core::property::VERSION</code>
</td>
<td>The Alexa Auto SDK version.
<p>
<b>Note:</b> This is a read-only property.</td>

</tr>
</table>


See the API reference documentation for [CoreProperties](./platform/include/AACE/Core/CoreProperties.h) for more information.
