# Core Module

The Alexa Auto SDK module contains the Engine base classes and the abstract platform interfaces that can be utilized by the platform and/or other modules. It also provides an easy way to integrate Alexa Auto SDK into an application or a framework. This involves configuring and creating an instance of `Engine`, overriding default platform implementation classes, and registering the custom interface handlers with the instantiated Engine.

**Table of Contents**

* [Creating the Engine](#creating-the-engine)
* [Configuring the Engine](#configuring-the-engine)
* [Registering Platform Interface Handlers](#registering-platform-interface-handlers)
* [Starting the Engine](#starting-the-engine)
* [Stopping the Engine](#stopping-the-engine)
* [Implementing Log Events](#implementing-log-events)
* [Implementing Audio](#implementing-audio)
* [Core Engine Properties](#core-engine-properties)

## Creating the Engine<a id="creating-the-engine"></a>

You create an instance of the Engine by calling the static function `Engine.create()`:

    m_engine = Engine.create();

## Configuring the Engine<a id= "configuring-the-engine"></a>

Before you can start the Engine, you must configure it using the required `aace.core.config.EngineConfiguration` object(s) for the services you will be using. The Alexa Auto SDK provides classes for reading the configuration data from a JSON file, as well as programmatically configuring the services.

> **Note**: For Android certpath, and data path runtime resolution, we recommend configuring the engine programmatically as demonstrated in the sample code below.  

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

Read about the `AlexaConfiguration Class` for more information about configurable methods.

* [Alexa Auto SDK for Android](https://alexa.github.io/alexa-auto-sdk/docs/android/)
* [Alexa Auto SDK for C++](https://alexa.github.io/alexa-auto-sdk/docs/cpp/)

**NOTE:** You can call the Engine's `configure()` method only be once, and you must call it before registering any platform interfaces or starting the Engine.

### Vehicle Information Requirements

You should supply vehicle configuration information to the Alexa Auto SDK through regular Engine configuration. A sample configuration is detailed below. You can generate the `EngineConfiguration` object including this information by using this schema in a `.json` config file or programmatically through the `VehicleConfiguration.createVehicleInfoConfig()` factory method.

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
The vehicle information should be provided with a unique vehicle identifier that is not the vehicle identification number (VIN). This information is required to pass the certification process. Read detailed information about the [vehicle configuration class](src/main/java/com/amazon/aace/vehicle/config/VehicleConfiguration.java) in the API reference documentation.

## Registering Platform Interface Handlers<a id="registering-platform-interface-handlers"></a>

A platform implementation should extend each interface it will use by creating an interface handler for it. Each handler will then be registered with the Engine by passing an instance to `registerPlatformInterface()`.

The functions that are overridden in the interface handlers are typically associated with directives from AVS. The functions that are made available by the interfaces, are typically associated with events or context sent to AVS. It is not always a one-to-one mapping however, because the Alexa Auto SDK attempts to simplify the platform's interaction with AVS.

An example of creating and registering platform interface handlers with the Engine is shown below.

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

## Starting the Engine<a id="starting-the-engine"></a>

After calling configure and registering all required platform interfaces, you can start the Engine. The Engine will first attempt to register all listed interface handlers, and then attempt to establish a connection with the given authorization implementation.

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

## Implementing Log Events<a id="implementing-log-events"></a>

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

### Source Tags
There are generally three different log `tag` values, depending on the `source`.

* `AVS` refers to logs being passed from the AVS Device SDK.
* `AAC` refers to logs being passed from the Alexa Auto SDK.
* `CLI` refers to logs coming from the client itself, by convention.

## Implementing Audio<a id="implementing-audio"></a>

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

## Core Engine Properties<a id="core-engine-properties"></a>

The Core module defines several constants that are used to get and set runtime properties in the Engine. To use these properties call the Engine's `getProperty()` and `setProperty()` methods.

```
// get the SDK version from the Engine
String version = m_engine.getProperty( com.amazon.aace.core.CoreProperties.VERSION );
```

The following constants are defined in the Core module:

<table>
<tr>
<th>Property</th>
<th>Description</th>
</tr>
<tr>
<td><code>com.amazon.aace.core.CoreProperties.VERSION</code>
</td>
<td>The Alexa Auto SDK version.
<p>
<b>Note:</b> This is a read-only property.</td>

</tr>
</table>


See the API reference documentation for [CoreProperties](src/main/java/com/amazon/aace/core/CoreProperties.java) for more information.
