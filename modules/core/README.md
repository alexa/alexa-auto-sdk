# Core Module


The Auto SDK Core module contains the Engine base classes and the abstract platform interfaces that the platform or other modules can use.

<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [Creating the Engine](#creating-the-engine)
- [Configuring the Engine](#configuring-the-engine)
  - [Configuration Database Files](#configuration-database-files)
  - [Specifying Configuration Data Using a JSON File](#specifying-configuration-data-using-a-json-file)
  - [Specifying Configuration Data Programmatically](#specifying-configuration-data-programmatically)
  - [Vehicle Information Requirements](#vehicle-information-requirements)
- [Extending the Default Platform Implementation](#extending-the-default-platform-implementation)
  - [Implementing a Location Provider](#implementing-a-location-provider)
  - [Implementing a Network Information Provider](#implementing-a-network-information-provider)
  - [Implementing Log Events](#implementing-log-events)
  - [Implementing Audio](#implementing-audio)
- [Starting the Engine](#starting-the-engine)
- [Stopping the Engine](#stopping-the-engine)
- [Managing Runtime Properties with the Property Manager](#managing-runtime-properties-with-the-property-manager)
  - [Property Manager Sequence Diagrams](#property-manager-sequence-diagrams)
  - [Implementing a Custom Property Manager Handler](#implementing-a-custom-property-manager-handler)
  - [Property Definitions](#property-definitions)
- [Managing Authorization](#managing-authorization)
  - [Authorization Sequence Diagrams](#authorization-sequence-diagrams)
  - [Using the Authorization Module](#using-the-authorization-module)

## Overview

The Core module provides an easy way to integrate the Auto SDK into an application or a framework. To do this, follow these steps:

1. [Create](#creating-the-engine) and [configure](#configuring-the-engine) an instance of `aace::core::Engine`.
2. [Override default platform implementation classes](#extending-the-default-platform-implementation) to extend the default Auto SDK platform implementation and register the platform interface handlers with the instantiated Engine.
4. [Start the Engine](#starting-the-engine).
5. [Change the runtime settings](#getting-and-setting-core-engine-properties) if desired.

## Creating the Engine

To create an instance of the Engine, call the static function `aace::core::Engine::create()`:

    std::shared_ptr<aace::core::Engine> engine = aace::core::Engine::create();

## Configuring the Engine

Before you can start the Engine, you must configure it using the required `aace::core::config::EngineConfiguration` object(s) for the services you will be using:

1. Generate the `EngineConfiguration` object(s). You can do this [using a JSON configuration file](#specifying-configuration-data-using-a-json-file), [programmatically (using factory methods)](#specifying-configuration-data-programmatically), or using a combination of both approaches.

    >**Note:** You can generate a single `EngineConfiguration` object that includes all configuration data for the services you will be using, or you can break the configuration data into logical sections and generate multiple `EngineConfiguration` objects. For example, you might generate one `EngineConfiguration` object for each module.
    
2. Call the Engine's `configure()` function, passing in the `EngineConfiguration` object(s): 

  * For a single `EngineConfiguration` object, use:

      `engine->configure( config );`
  
 *  For multiple `EngineConfiguration` objects, use:

      `engine->configure( { xConfig, yConfig, zConfig, ... } );`
      
      replacing `xConfig, yConfig, zConfig` with logical names to identify the `EngineConfiguration` objects you generated; for example: `coreConfig, alexaConfig, navigationConfig`

> **Note**: You can call the Engine's `configure()` method only once, and you must call it before you register any platform interfaces or start the Engine.

### Configuration Database Files

Some values in the Engine configuration, such as `"defaultlocale"`, are used only to configure the Engine the first time it is started. After the first start, the Auto SDK engine creates configuration database files (also referred to as SQLite database files) so that these settings are preserved the next time you start the application. You can change the default settings if desired.

By default, the Auto SDK stores the configuration database files in the `/opt/AAC/data/` directory, but you have the option to change the path to the configuration database files as part of your Engine configuration. If you delete the database files, the Auto SDK will create new ones the next time you run the application.

### Specifying Configuration Data Using a JSON File

The Auto SDK provides a class in [`EngineConfiguration.h`](./platform/include/AACE/Core/EngineConfiguration.h) that reads the configuration from a specified JSON file and creates an `EngineConfiguration` object from that configuration:

`aace::core::config::ConfigurationFile::create( “<filename.json>” )`
 
You can include all the configuration data in a single JSON file to create a single `EngineConfiguration` object; for example:

`auto config = aace::core::config::ConfigurationFile::create( “config.json” );`

or break the configuration data into multiple JSON files to create multiple `EngineConfiguration` objects; for example:

```cpp
auto coreConfig = aace::core::config::ConfigurationFile::create( “core-config.json” );
auto alexaConfig = aace::core::config::ConfigurationFile::create( “alexa-config.json” );
auto navigationConfig = aace::core::config::ConfigurationFile::create( “navigation-config.json” );
```

The [config.json.in](../../samples/cpp/assets/config.json.in) file provides an example of a JSON configuration file. If desired, you can use this file as a starting point for customizing the Engine configuration to suit your needs.

### Specifying Configuration Data Programmatically

You can also specify the configuration data programmatically by using the configuration factory methods provided in the library. For example, you can configure the `alertsCapabilityAgent` settings by instantiating an `EngineConfiguration` object with the following method:

```cpp
auto alertsConfig = aace::alexa::config::AlexaConfiguration::createAlertsConfig
    ("<SQLITE_DATABASE_FILE_PATH>" );
```

See the API reference documentation for the [`AlexaConfiguration` class](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html) for details about the configurable methods used to generate the `EngineConfiguration` object.

### Vehicle Information Requirements

You must configure vehicle information in the Engine configuration. A sample configuration is detailed below. You can generate the `EngineConfiguration` object including this information by using this schema in a `.json` config file or programmatically through the `VehicleConfiguration::createVehicleInfoConfig()` factory method.

```cpp
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
         "microphone": "<MICROPHONE>",
         "vehicleIdentifier": "<VEHICLE_IDENTIFIER>"
     }
  }
}
```
For details about the vehicle properties included in the `VehicleConfiguration` class, see the[`VehicleConfiguration.h`](./platform/include/AACE/Vehicle/VehicleConfiguration.h) file.

>**Important!** To pass the certification process, the vehicle information that you provide in the Engine configuration must include a `"vehicleIdentifier"` that is NOT the vehicle identification number (VIN).

## Extending the Default Platform Implementation

To extend each Auto SDK interface you will use in your platform implementation:

1. Create a handler for the interface by overriding the various classes in the library that, when registered with the Engine, allow your application to interact with Amazon services.
2. Register the handler with the Engine. The Engine class provides two methods for registering platform interface handlers, which allows you to register one or more interfaces at a time for convenience:

    ```
    class MyInterface : public SpeechRecognizer {
    ...

    engine->registerPlatformInterface( std::make_shared<MyInterface>() );
    ```
    
    OR  
    
    ```
    std::shared_ptr::MyInterface1 myInterface1 =
    std::make_shared<MyInterface1>();
    std::shared_ptr::MyInterface2 myInterface2 =
    std::make_shared<MyInterface2>();
    engine->registerPlatformInterface({ myInterface1, myInterface2 });
    ```

The functions that you override in the interface handlers are typically associated with directives from Alexa Voice Service (AVS). The functions that are made available by the interfaces are typically associated with events or context sent to AVS. It is not always a one-to-one mapping, however, because the Auto SDK attempts to simplify the interaction with AVS.

The sections below provide information about and examples for creating and registering [location provider](#implementing-a-location-provider), [network information provider](#implementing-a-network-information-provider), [logging](#implementing-log-events), and [audio](#implementing-audio) interface handlers with the Engine. For details about creating handlers for the various Auto SDK modules, see the README files for those modules.

### Implementing a Location Provider

The Engine provides a callback for implementing location requests from Alexa and other modules and a Location type definition. This is optional and dependent on the platform implementation.

To implement a custom `LocationProvider` handler to provide location using the default Engine `LocationProvider` class, extend the `LocationProvider` class:

```cpp
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

### Implementing a Network Information Provider

The `NetworkInfoProvider` platform interface provides methods that you can implement in a custom handler to allow your application to monitor network connectivity and send network status change events whenever the network status changes. Methods such as `getNetworkStatus()` and `getWifiSignalStrength()` allow the Engine to retrieve network status information, while the `networkStatusChanged()` method informs the Engine about network status changes.

The `NetworkInfoProvider` methods are dependent on your platform implementation and are required by various internal Auto SDK components to get the initial network status from the network provider and update that status appropriately. When you implement the `NetworkInfoProvider` platform interface correctly, Auto SDK components that use the methods provided by this interface work more effectively and can adapt their internal behavior to the initial network status and changing network status events as they come in.

> **Important!** Network connectivity monitoring is the responsibility of the platform. The Auto SDK doesn't monitor network connectivity.

To implement a custom handler to monitor network connectivity and send network status change events, extend the `NetworkInfoProvider` class:

```cpp
#include <AACE/Network/NetworkInfoProvider.h>
...
class MyNetworkInfoProvider : public aace::network::NetworkInfoProvider {
public:
    aace::network::NetworkInfoProvider::NetworkStatus getNetworkStatus() override {
        // Return the current network status as determined on the platform.
        // Here we return the default, but you should return the real network status.
        return aace::network::NetworkInfoProvider::NetworkStatus::CONNECTED;
    }

    int getWifiSignalStrength() override {
        // Return the current WiFi signal strength RSSI (Received Signal Strength Indicator)
        // as determined on the platform.
        // Here we return the default, but you should return the real WiFi signal strength.
        return 100;
    }

};

// Register the platform interface with the Engine
auto myNetworkInfoProvider = std::make_shared<MyNetworkInfoProvider>();
engine->registerPlatformInterface( myNetworkInfoProvider );

...

// Send networkStatusChanged() notifications as Network Status and WiFi signal strength change on the platform
myNetworkInfoProvider->networkStatusChanged( networkStatus, wifiSignalStrength );

```

### Implementing Log Events

The Engine provides a callback for implementing log events from the AVS SDK. This is optional, but useful for the platform implementation.

To implement a custom log event handler for logging events from AVS using the default engine Logger class, extend the `Logger` class:

```cpp
#include <AACE/Logger/Logger.h>

class MyLogger : public aace::logger::Logger {

  void logEvent(aace::logger::Logger::Level level, std::chrono::system_clock::time_point time, const std::string& source, const std::string& message) override {
    //handle the log events from Auto SDK, AVS, or other source
  };
  ...

};

// Register the platform interface with the Engine
engine->registerPlatformInterface( std::make_shared<MyLogger>());
```        
    
### Implementing Audio

The platform should implement audio input and audio output handling. Other Auto SDK components can then make use of the provided implementation to provision audio input and output channels. 
  
The `AudioInputProvider` should provide a platform-specific implementation of the `AudioInput` interface, for the type of input specified by the `AudioInputType` parameter when its `openChannel()` method is called, with only one instance of `AudioInput` per `AudioInputType`. For example, the `SpeechRecognizer` engine implementation requests an audio channel for type `VOICE`, while AlexaComms requests a channel for type `COMMUNICATION` - the implementation determines if these are shared or separate input channels. If it is a shared input channel, then whenever the platform implementation writes data to the interface, any instance in the Engine that has opened that channel will receive a callback with the audio data.
  
```cpp
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

```cpp
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

```cpp
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

```cpp
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
## Starting the Engine

After creating and registering handlers for all required platform interfaces, you can start the Engine by calling the Engine's `start()` method. The Engine will first attempt to register all listed interface handlers, and then attempt to establish a connection with the given authorization implementation.

```
engine->start();
```

## Stopping the Engine
If you need to stop the engine for any reason except for logging out the user, use the Engine's `stop()` method. You can then restart the Engine by calling `start()` again.

```
engine->stop();
```

You should call `shutdown()` on the Engine when the app is being destroyed or the user is logged out. This makes sure when the next user logs in, a corresponding publish message is sent to the cloud with new capabilities and configuration that is tied to the new user. 

```
engine->shutdown();
```
## Managing Runtime Properties with the Property Manager

Certain modules in the Auto SDK define constants (for example `FIRMWARE_VERSION` and `LOCALE`) that are used to get and set the values of runtime properties in the Engine. Changes to property values may also be initiated from the Alexa Voice Service (AVS). For example, the `TIMEZONE` property may be changed through AVS when the user changes the timezone setting in the Alexa Companion App.

The Auto SDK Property Manager maintains the runtime properties by storing properties and listeners to the properties and delegating the `setProperty()` and `getProperty()` calls from your application to the respective Engine services. It also calls `propertyChanged()` to notify your application about property value changes originating in the Engine. The Property Manager includes a `PropertyManager` platform interface that provides the following methods:

* `setProperty()` - called by your application to set a property value in the Engine.
* `getProperty()` - called by your application to retrieve a property value from the Engine.
    >**Note:** `setProperty()` is asynchronous. After calling `setProperty()`, `getProperty()` returns the updated value only after the Engine calls `propertyStateChanged()` with `PropertyState::SUCCEEDED`.
* `propertyStateChanged()` - notifies your application about the status of a property value change (`SUCCEEDED` or `FAILED`). This is an asynchronous response to your application's call to `setProperty()`.
* `propertyChanged()` - notifies your application about a property value change in the Engine that was initiated internally, either by AVS or an Engine component.

### Property Manager Sequence Diagrams

#### Application Changes a Property Value
The following sequence diagram illustrates the flow when your application calls `setProperty()` to set a property value in the Engine.

![Set_Property](./assets/PropertyManager_set.png)

#### Application Retrieves a Property Value
The following sequence diagram illustrates the flow when your application calls `getProperty()` to retrieve a property value from the Engine.

![Get_Property](./assets/PropertyManager_get.png)

#### Notification of Property Value Change Initiated by AVS
The following sequence diagram illustrates the flow when a property value change is initiated by AVS and the Property Manager notifies your application.

![Property_Changed](./assets/PropertyManager_changed.png)

### Implementing a Custom Property Manager Handler
To implement a custom Property Manager handler to set and retrieve Engine property values and be notified of property value changes, extend the `PropertyManager` class:

```cpp
#include <AACE/PropertyManager/PropertyManager.h>

class MyPropertyManager : public aace::propertyManager::PropertyManager {
    
    void propertyStateChanged(const std::string& name, const std::string& value, const PropertyState state) override {
        // Handle the status of a property change after a call to setProperty().
    }
              
    void propertyChanged(const std::string& name, const std::string& newValue) override {
        // Handle the property value change initiated by the Engine.
        // For example, if the user sets the TIMEZONE to "Pacific Standard Time - Vancouver"
        // in the companion app, your application gets a call to:
        // propertyChanged(aace::alexa::property::TIMEZONE, "America/Vancouver")
    }
}
...

// Register the platform interface with the Engine
auto m_propertyManagerHandler = std::make_shared<MyPropertyManagerHandler>();
engine->registerPlatformInterface( m_propertyManagerHandler );

// You can also set and retrieve properties in the Engine by calling the inherited
// setProperty() and getProperty() methods.

// For example, to set the LOCALE property to English-Canada:
m_propertyManagerHandler->setProperty(aace::alexa::property::LOCALE, "en-CA");

// For example, to retrieve the value of the LOCALE property:
auto locale  = m_propertyManagerHandler->getProperty(aace::alexa::property::LOCALE);

```
### Property Definitions
The definitions of the properties used with the `PropertyManager::setProperty()` and `PropertyManager::getProperty()` methods are included in the [AlexaProperties.h](../alexa/platform/include/AACE/Alexa/AlexaProperties.h) and [CoreProperties.h](./platform/include/AACE/Core/CoreProperties.h) files. For a list of the Alexa Voice Service (AVS) supported locales for the `LOCALE` property, see the [Alexa Voice Service (AVS) documentation](https://developer.amazon.com/docs/alexa-voice-service/system.html#locales).


## Managing Authorization

The Auto SDK needs access to cloud services and resources to function. Gaining access requires that the device be authorized with an authorization service such as Login With Amazon (LWA), which provides the access token. The Engine uses the token to access cloud services and resources. For example, to access Alexa APIs, the device must be authorized with LWA to obtain the access token.

The Auto SDK Authorization module is responsible for managing authorizations for different cloud services. For example, to use Alexa, your device must be authorized with LWA. The module provides a single platform interface for all authorizations and communicates with the engine services (referred to here as authorization services). The authorization service is responsible for carrying out the authorization method you choose. For example, for Alexa, you can use the CBL authorization or Auth Provider authorization method. The CBL or Auth Provider authorization service carries out the actual authorization process or flow.

For information on how to use the Authorization module with different authorization methods, see the Alexa module [README](../alexa/README.md#handling-authorization) and the CBL module [README](../cbl/README.md).

### Authorization Sequence Diagrams

#### Starting the Authorization Process

The following sequence diagram shows the typical call sequences between platform implementation and Auto SDK to start an authorization process.

![Starting_Authorization](./assets/Authorization_start.png)

#### Canceling the Authorization Process

The following sequence diagram shows the typical call sequence between the platform implementation and Auto SDK to cancel an authorization process.

![Cancel_Authorization](./assets/Authorization_cancel.png)

#### Logging out the Authorization

The following sequence diagram shows the typical call sequence between the platform implementation and Auto SDK to log out of an authorization.

![Logout_Authorization](./assets/Authorization_logout.png)

### Using the Authorization Module

To implement the custom `Authorization`  handler, extend the `Authorization` class as follows:

```cpp
#include <AACE/Authorization/Authorization.h>

class MyAuthorizationHandler : public aace::authorization::Authorization {     
    // There is an event from the requested authorization service.
    void eventReceived(const std::string& service, const std::string& event) override {
        // Take the necessary action as defined by the service protocol.
    }

    // Authorization service notifying the platform implementation of the state change.
    void authorizationStateChanged(const std::string& service, AuthorizationState state) override{
        // Handle the authorization state change as required by your application.
    }
    
    // Authorization service notifies an error in the process.
    void authorizationError(const std::string& service, const std::string& error, const std::string& message) override {
        // Handle the authorization error as required by your application.
    }

    // Authorization service needs to get the authorization-related data from the platform implementation.
    std::string getAuthorizationData(const std::string& service, const std::string& key) override {
        // Return the data identified by key.
    }
    
    // Authorization service requires the platform implementation to store the authorization-related data.
    void setAuthorizationData(const std::string& service, const std::string& key, const std::string& data) override {
        // Store/Clear the data identified by the key securely on the device.
    }
}

// Register the platform interface with the Engine
auto m_authorizationHandler = std::make_shared<MyAuthorizationHandler>();
engine->registerPlatformInterface( m_authorizationHandler );

// To notify the Engine to start authorization process represented by `service-name`
m_authorizationHandler->startAuthorization("service-name", "data-as-defined-by-service");

// To notify the Engine to cancel authorization process represented by `service-name`, which is already in progress
m_authorizationHandler->cancelAuthorization("service-name");

// To notify the Engine to log out from the authorization for the service represented by `service-name`.
m_authorizationHandler->logout("service-name");

// To send events from the platform implementation to the authorization service.
m_authorizationHandler->sendEvent("service-name", "event-data-as-defined-by-service");

```
