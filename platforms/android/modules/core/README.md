# Core Module

The Alexa Auto SDK module contains the Engine base classes and the abstract platform interfaces that can be utilized by the platform and/or other modules.

**Table of Contents**

* [Overview](#overview)
* [Creating the Engine](#creating-the-engine)
* [Configuring the Engine](#configuring-the-engine)
* [Extending the Default Platform Implementation](#extending-the-default-platform-implementation)
* [Starting the Engine](#starting-the-engine)
* [Stopping the Engine](#stopping-the-engine)
* [Getting and Setting Core Engine Properties](#getting-and-setting-core-engine-properties)

## Overview <a id="overview"></a>
The Core module provides an easy way to integrate Alexa Auto SDK into an application or a framework. To do this, follow these steps:

1. [Create](#creating-the-engine) and [configure](#configuring-the-engine) an instance of `aace::core::Engine`.
2. [Override default platform implementation classes](#extending-the-default-platform-implementation) to extend the default Alexa Auto SDK platform implementation and register the platform interface handlers with the instantiated Engine.
4. [Start the Engine](#starting-the-engine).
5. [Change the runtime settings](#getting-and-setting-core-engine-properties) if desired.

## Creating the Engine <a id="creating-the-engine"></a>

To create an instance of the Engine, call the static function `Engine.create()`:

    m_engine = Engine.create();

## Configuring the Engine <a id= "configuring-the-engine"></a>

Before you can start the Engine, you must configure it using the required `aace.core.config.EngineConfiguration` object(s) for the services you will be using:

1. Generate the `EngineConfiguration` object(s). You can do this [using a JSON configuration file](#specifying-configuration-data-using-a-json-file), [programmatically (using factory methods)](#specifying-configuration-data-programmatically), or using a combination of both approaches; however, for Android certpath and data path runtime resolution, we recommend [configuring the engine programmatically](#specifying-configuration-data-programmatically).

    >**Note:** You can generate a single `EngineConfiguration` object that includes all configuration data for the services you will be using, or you can break the configuration data into logical sections and generate multiple `EngineConfiguration` objects. For example, you might generate one `EngineConfiguration` object for each module.
  
2. Call the Engine's `configure()` function, passing in the `EngineConfiguration` object(s): 

  * For a single `EngineConfiguration` object, use:

      `engine->configure( config );`
  
 *  For multiple `EngineConfiguration` objects, use:

      `engine->configure( { xConfig, yConfig, zConfig, ... } );`
      
      replacing `xConfig, yConfig, zConfig` with logical names to identify the `EngineConfiguration` objects you generated; for example: `coreConfig, alexaConfig, navigationConfig`

    > **Note**: You can call the Engine's `configure()` method only once, and you must call it before you register any platform interfaces or start the Engine.

### Configuration Database Files

Some values in the configuration, such as `"defaultlocale"`, are used only to configure the Engine the first time it is started. After the first start, the Auto SDK engine creates configuration database files (also referred to as SQLite database files) so that these settings are preserved the next time you start the application. You can change the default settings if desired.

By default, the Auto SDK stores the configuration database files in the `/opt/AAC/data/` directory, but you have the option to change the path to the configuration database files as part of your Engine configuration. If you delete the database files, the Auto SDK will create new ones the next time you run the application.

### Specifying Configuration Data Using a JSON File <a id = "specifying-configuration-data-using-a-json-file"></a>

The Auto SDK provides a class in [`EngineConfiguration.h`](../../../../modules/core/platform/include/AACE/Core/EngineConfiguration.h) that reads the configuration from a specified JSON file and creates an `EngineConfiguration` object from that configuration:

`aace::core::config::ConfigurationFile::create( “<filename.json>” )`
 
You can include all the configuration data in a single JSON file to create a single `EngineConfiguration` object; for example:

`auto config = aace::core::config::ConfigurationFile::create( “config.json” );`

or break the configuration data into multiple JSON files to create multiple `EngineConfiguration` objects; for example:

```
auto coreConfig = aace::core::config::ConfigurationFile::create( “core-config.json” );
auto alexaConfig = aace::core::config::ConfigurationFile::create( “alexa-config.json” );
auto navigationConfig = aace::core::config::ConfigurationFile::create( “navigation-config.json” );
```

The [config.json.in](../../../../samples/cpp/assets/config.json.in) file provides an example of a JSON configuration file. If desired, you can use this file as a starting point for customizing the Engine configuration to suit your needs.

### Specifying Configuration Data Programmatically <a id ="specifying-configuration-data-programmatically"></a>

You can also specify the configuration data programmatically by using the configuration factory methods provided in the library. The following code sample provides an example of using factory methods to instantiate an `EngineConfiguration` object:

```
m_engine.configure( new EngineConfiguration[]{
    AlexaConfiguration.createCurlConfig( "<CERTS_PATH>" ),
    AlexaConfiguration.createDeviceInfoConfig(  "<DEVICE_SERIAL_NUMBER>", "<CLIENT_ID>", "<PRODUCT_ID>" ),
    AlexaConfiguration.createMiscStorageConfig( "<SQLITE_DATABASE_FILE_PATH>" ),
    AlexaConfiguration.createCertifiedSenderConfig( "<SQLITE_DATABASE_FILE_PATH>" ),
    AlexaConfiguration.createAlertsConfig( "<SQLITE_DATABASE_FILE_PATH>" ),
    AlexaConfiguration.createSettingsConfig( "<SQLITE_DATABASE_FILE_PATH>" ),
    AlexaConfiguration.createNotificationsConfig( "<SQLITE_DATABASE_FILE_PATH>" ),
    LoggerConfiguration.createSyslogSinkConfig( "syslog", "<LOGGER_LEVEL>" )
});
```

See the API reference documentation for the [`AlexaConfiguration Class`](https://alexa.github.io/alexa-auto-sdk/docs/android/classcom_1_1amazon_1_1aace_1_1alexa_1_1config_1_1_alexa_configuration.html) for details about the configurable methods used to generate the `EngineConfiguration` object.

### Vehicle Information Requirements

You must configure vehicle information in the Engine configuration. A sample configuration is detailed below. You can generate the `EngineConfiguration` object including this information by using this schema in a `.json` config file or programmatically using the `VehicleConfiguration.createVehicleInfoConfig()` factory method.

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
>**Important!** To pass the certification process, the vehicle information that you provide in the Engine configuration must include a `"vehicleIdentifier"` that is NOT the vehicle identification number (VIN). See the [platform API reference documentation] (https://alexa.github.io/alexa-auto-sdk/docs/android/classcom_1_1amazon_1_1aace_1_1vehicle_1_1config_1_1_vehicle_configuration.html) for more information about the [`VehicleConfiguration`](src/main/java/com/amazon/aace/vehicle/config/VehicleConfiguration.java) class.

## Extending the Default Platform Implementation <a id="extending-the-default-platform-implementation"></a>

To extend each Auto SDK interface you will use in your platform implementation:

1. Create a handler for the interface by overriding the various classes in the library that, when registered with the Engine, allow your application to interact with Amazon services.
2. Register the handler with the Engine by passing an instance to `registerPlatformInterface`.

The functions that you override in the interface handlers are typically associated with directives from Alexa Voice Service (AVS). The functions that are made available by the interfaces are typically associated with events or context sent to AVS. It is not always a one-to-one mapping however, because the Alexa Auto SDK attempts to simplify the interaction with AVS.

The code sample below provides an example of creating and registering platform interface handlers with the Engine.

```
// LoggerHandler.java
	public class LoggerHandler extends Logger {
	...
	// AlexaClientHandler.java
	public class AlexaClientHandler extends AlexaClient {
	...
	// MainActivity.java
	m_engine.registerPlatformInterface( m_logger = new LoggerHandler( getApplicationContext(), Logger.Level.INFO ) );
	m_engine.registerPlatformInterface( m_alexaClient = new AlexaClientHandler( getApplicationContext(), m_logger ) );
	...
```
The sections below provide information about and examples for creating [logging](#implementing-log-events) and [audio](#implementing-audio) interface handlers with the Engine. For details about creating handlers for the various Auto SDK modules, see the README files for those modules.

### Implementing Log Events<a id="implementing-log-events"></a>

The Engine provides a callback for implementing log events from the AVS SDK. This is optional, but useful for the platform implementation.

To implement a custom log event handler for logging events from AVS using the default engine Logger class, extend the `Logger` class:

```
public class LoggerHandler extends Logger
{
	@Override
	public boolean logEvent( Level level, long time, String source, String message )
	{
		//handle the log events from Alexa Auto SDK, AVS, or other source
	...
		//notify the Engine of a new log event
		log((Level) level, (String) tag, (String) message );

```    

#### Source Tags
There are generally three different log `tag` values, depending on the `source`.

* `AVS` refers to logs being passed from the AVS Device SDK.
* `AAC` refers to logs being passed from the Alexa Auto SDK.
* `CLI` refers to logs coming from the client itself, by convention.

### Implementing Audio<a id="implementing-audio"></a>

The platform should implement audio input and audio output handling. Other Auto SDK components will make use of the provided implementation to provision audio input and output channels. 
  
The `AudioInputProvider` should provide a platform specific implementation of the `AudioInput` interface, for the type of input specified by the `AudioInputType` parameter, when its `openChannel()` method is called. There should be only one instance of `AudioInput` per `AudioInputType`. For example, the `SpeechRecognizer` Engine implementation requests an audio channel for type `VOICE`, while AlexaComms requests a channel for type `COMMUNICATION` - it's up to the implementation to determine if these are shared or separate input channels. If it is a shared input channel, then whenever the platform implementation writes data to the interface, any instance in the Engine that has opened that channel will receive a callback with the audio data.
  
```
public class AudioInputProviderHandler extends AudioInputProvider
{
...
    @Override
    public AudioInput openChannel( String name, AudioInputType type )
    {
        // Create a shared AudioInput for VOICE and COMMUNICATION input channels. In this
        // use case both SpeechRecognizer and AlexaComms are sharing a single microphone.
        if( type == AudioInputType.VOICE || type == AudioInputType.COMMUNICATION ) {
            return getDefaultAudioInput();
        }
        // If a loopback audio can be provided then return the AudioInput implementation
        // for the loopback audio source.
        else if( type == AudioInputType.LOOPBACK ) {    
            return getAudioLoopbackSource();
        }
        else {
            return null;
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
public class AudioInputHandler extends AudioInput
{
    ...
    @Override
    public boolean startAudioInput() {
        ...
        // start receiving audio data from the platform specific input device
        return startRecording();
    }
    
    @Override
    public boolean stopAudioInput() {
        ...
        try {
            // stop requesting audio data from the platform specific input device
            mAudioInput.stop();
        } catch (IllegalStateException e) {
            return false;
        }

        return true;
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
 
It's up to the `AudioOutputProvider` implementation to determine how to handle each different audio type. The simplest implementation, for example, may ignore the audio type and return the same `AudioOutput` implementation for all channels. 

A more sophisticated implementation may provide completely different `AudioOutput` implementations depending on the audio type - for example, providing a low level audio implementation for `NOTIFICATION` and `EARCON` types, and a high level implementation (such as ExoPlayer) for `TTS` and `MUSIC`. The best approach is highly dependent on your system specific use case.

```
public class AudioOutputProviderHandler extends AudioOutputProvider
{
    ...
    @Override
    public AudioOutput openChannel( String name, AudioOutputType type )
    {
        AudioOutput audioOutputChannel = null;
        // Simple implementation which returns a new instance of AudioOutputHandler
        // whenever openChannel() is called. 
        switch( type )
        {
            case COMMUNICATION:
                audioOutputChannel = new RawAudioOutputHandler( mActivity, mLogger, name );
                break;

            default:
                audioOutputChannel = new AudioOutputHandler( mActivity, mLogger, name );
                break;
        }

        mAudioOutputMap.put( name, audioOutputChannel );

        return audioOutputChannel;
    }
    ...
};
```

The `AudioOutput` describes a platform-specific implementation of an audio output channel. The platform should have one or more implementations of it, depending on on the desired behavior. In addition to implementing the callback methods, each `AudioOutput` channel implementation should report the state of its media, when appropriate. 

The full `AudioOutput` API is described below. 

```
public class AudioOutputHandler extends AudioOutput implements AuthStateObserver
{ 
    ...
    @Override
    public boolean prepare( AudioStream stream, boolean repeating ) {   
        ... // tell the platform media player to prepare audio stream
        mPlayer.prepare( mediaSource );
        return true;
    ...
    
    @Override
    public boolean prepare( String utl, boolean repeating ) {   
        ... // tell the platform media player to prepare the url
        mPlayer.prepare( url );
    ...
    
    @Override
    public boolean play() {
        ... // tell the platform media player to start playing
        mPlayer.setPlayWhenReady( true );
    ...
 
    @Override
    public boolean stop() {
        ... // tell the platform media player to stop playing
        mPlayer.setPlayWhenReady( false );
    ...
 
    @Override
    public boolean pause() {
        ... // tell the platform media player to pause playback
        mPlayer.setPlayWhenReady( false );
    ...
 
    @Override
    public boolean resume() {
        ... // tell the platform media player to resume playback
        mPlayer.setPlayWhenReady( true );
    ...
 
    @Override
    public boolean setPosition( long position ) {
        ... // set the current media position of the platform media player
        mPlayer.seekTo( position );
    ...
 
    @Override
    public long getPosition() {
        ... // return the current media position of the platform media player        
        return mPosition;
    ...
    
    @Override
    public long getDuration() {
        ... // return the current media duration of the platform media player        
        return mPlayer.getDuration();
    ...
    
    @Override
    public boolean volumeChanged( float volume ) {
        ... // set the current media volume of the platform media player
        mPlayer.setVolume( volume );
    ...
    
    @Override
    public boolean mutedStateChanged( MutedState state ) {
        ... // set the current media mute state of the platform media player   
        mPlayer.setVolume( state == MutedState.MUTED ? 0 : mVolume );
    ...
        // on playback started notification from the platform media player  
        mediaStateChanged( MediaState.PLAYING );
        ...
        // on playback stopped notification from the platform media player  
        mediaStateChanged( MediaState.STOPPED );
        ...
        // on playback buffering notification from the platform media player  
        mediaStateChanged( MediaState.BUFFERING );    
    ...
};
```

## Starting the Engine<a id="starting-the-engine"></a>

After creating and registering handlers for all required platform interfaces, you can start the Engine. The Engine will first attempt to register all listed interface handlers, and then attempt to establish a connection with the given authorization implementation.

```
m_engine.start();
```

## Stopping the Engine<a id="stopping-the-engine"></a>

If you need to stop the engine for any reason, use the Engine's `stop()` method. You can then restart the Engine by calling `start()` again.

```
if( m_engine != null ) {
    m_engine.stop();
}
```

You should call `dispose()` on the Engine when the app is being destroyed.

```
if( m_engine != null ) {
    m_engine.dispose();
}
```

## Getting and Setting Core Engine Properties <a id="getting and-setting-core-engine-properties"></a>

The Core module defines one or more constants (such as `VERSION`) that are used to get and set runtime properties in the Engine. To use these properties, call the Engine's `getProperty()` and `setProperty()` methods.

```
// get the SDK version from the Engine
String version = m_engine.getProperty( com.amazon.aace.core.CoreProperties.VERSION );
```

>**Note:** The `setProperty()` method returns `true` if the the property value was successfully updated and `false` if the update failed.

The [`CoreProperties`](src/main/java/com/amazon/aace/core/CoreProperties.java) class includes details about the Engine properties defined in the Core module.
