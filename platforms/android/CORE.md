## Core API

### Overview

The module contains the Engine base classes and the abstract platform interfaces that can be utilized by the platform and/or other modules. It also provides an easy way to integrate AAC SDK into an application or a framework. This involves configuring and creating an instance of `Engine`, overriding default platform implementation classes, and registering the custom interface handlers with the instantiated Engine.

### Creating the Engine

You create an instance of the Engine by calling the static function `Engine.create()`.

    m_engine = Engine.create();

### Configuring the Engine

Before the Engine can be started, you must configure it using the required `EngineConfiguration` object(s) for the services you will be using. The SDK provides classes for reading the configuration data from a JSON file, as well as programmatically configuring the services.

> **NOTE**: For Android certpath, and data path runtime resolution, we recommend configuring the engine programmitcally as demonstrated in the sample code below.  

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

**NOTE:** The Engine's `configure()` method can only be called once and must be called before registering any platform interfaces or starting the Engine.

### Registering Platform Interface Handlers

A platform implementation should extend each interface it will use by creating an interface handler for it. Each handler will then be registered with the Engine by passing an instance to `registerPlatformInterface()`.

The functions that are overriden in the interface handlers are typically associated with directives from AVS. The functions that are made available by the interfaces, are typically associated with events or context sent to AVS. It is not always a one to one mapping however, because the AAC SDK attempts to simplify the platform's interaction with AVS.

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

#### Starting the Engine 

Finally, the Engine can be started after calling configure and registering all required platform interfaces. The Engine will attempt to register all listed interface handlers, and then attempt to establish a connection with the given authorization implementation.

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