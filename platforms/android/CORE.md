## Core API

### Overview

The module contains the Engine base classes and the abstract platform interfaces that can be utilized by the platform and/or other modules. It also provides an easy way to integrate AAC SDK into an application or a framework. This involves configuring and creating an instance of `Engine`, overriding default platform implementation classes, and registering the custom interface handlers with the instantiated Engine.

#### Creating and configuring the Engine

The Engine is created with the necessary configuration options passed to Engine's `configure()` method. 

The configuration from the AAC sample is given below:

```
m_engine.configure( new EngineConfiguration[] {
	AlexaConfiguration.createCurlConfig( certsDir.getPath() ),
	AlexaConfiguration.createCertifiedSenderConfig( appDataDir.getPath() + "/certifiedSender.sqlite" ),
	AlexaConfiguration.createAlertsConfig( appDataDir.getPath() + "/alerts.sqlite" ),
	AlexaConfiguration.createSettingsConfig( appDataDir.getPath() + "/settings.sqlite" ),
	AlexaConfiguration.createNotificationsConfig( appDataDir.getPath() + "/notifications.sqlite" ),
});
```

#### Registering platform interface handlers 

The functions that are overriden in the interface handlers are typically associated with directives from AVS. The functions that are made available by the interfaces, are typically associated with events sent to AVS. It is not a one to one mapping, because the AAC SDK attempts to simplify the platform's interaction with AVS.

Each platform interface is registered with the Engine by passing an instance to `registerPlatformInterface()`.

An example of registering the platform interfaces with the Engine is shown below. 

```
	m_engine.registerPlatformInterface( m_logger = new LoggerHandler( getApplicationContext(), Logger.Level.INFO ) );
	m_engine.registerPlatformInterface( m_alexaClient = new AlexaClientHandler( getApplicationContext(), m_logger ) );
	...
```

#### Starting the Engine 

Finally, the Engine can be started after `configure()` and each `registerPlatformInterface` has been called. The Engine will attempt to register all listed interface handlers, and then attempt to establish a connection with the given authorization implementation.

```
    m_engine.start();
```

#### Stopping the Engine

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

### Implementing log events

The Engine provides a callback for implementing log events from the AVS SDK. This is optional, but useful for the platform implementation.

To implement a custom log event handler for logging events from AVS using the default engine Logger class, the `Logger` class should be extended:

```
public class LoggerHandler extends Logger
{
	@Override
	public boolean logEvent( Level level, long time, String source, String message )
	{
		//handle the log events from AAC, AVS, or other source
	...
		//notify the Engine of a new log event
		log((Level) level, (String) tag, (String) message );
	
```    

### Source Tags
There are generally three different log `tag` values, depending on the `source`.

- `AVS` refers to logs being passed from the AVS Device SDK.
- `AAC` refers to logs being passed from the AAC SDK.
- `CLI` refers to logs coming from the client itself, by convention.