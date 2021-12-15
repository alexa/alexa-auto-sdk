# Core Module <!-- omit in toc -->

**Table of Contents** <!-- omit in toc -->

- [Core module overview](#core-module-overview)
- [Understand the Auto SDK API](#understand-the-auto-sdk-api)
  - [The Engine](#the-engine)
  - [MessageBroker](#messagebroker)
  - [AASB message interfaces](#aasb-message-interfaces)
- [Manage the Engine lifecycle in your application](#manage-the-engine-lifecycle-in-your-application)
  - [Create the Engine](#create-the-engine)
  - [Configure the Engine](#configure-the-engine)
    - [Specify configuration in a file](#specify-configuration-in-a-file)
    - [Specify configuration programatically](#specify-configuration-programatically)
  - [Subscribe to AASB messages](#subscribe-to-aasb-messages)
  - [Start the Engine](#start-the-engine)
  - [Stop the Engine](#stop-the-engine)
  - [Shut down the Engine](#shut-down-the-engine)
- [Configure the Core module](#configure-the-core-module)
  - [(Required) Vehicle info configuration](#required-vehicle-info-configuration)
  - [(Required) Storage configuration](#required-storage-configuration)
  - [(Optional) Logger configuration](#optional-logger-configuration)
  - [(Optional) cURL configuration](#optional-curl-configuration)
  - [(Optional) AASB and MessageBroker configuration](#optional-aasb-and-messagebroker-configuration)
    - [(Optional) Configure enabled interfaces](#optional-configure-enabled-interfaces)
    - [(Optional) Configure the synchronous message timeout](#optional-configure-the-synchronous-message-timeout)
- [Understand the key core Engine services](#understand-the-key-core-engine-services)
  - [Authorization](#authorization)
  - [Audio input and output](#audio-input-and-output)
  - [Runtime properties](#runtime-properties)
- [Provide core device status](#provide-core-device-status)
  - [Report location with LocationProvider](#report-location-with-locationprovider)
  - [Report network status changes with NetworkInfoProvider](#report-network-status-changes-with-networkinfoprovider)
  - [Report data usage with DeviceUsage](#report-data-usage-with-deviceusage)

## Core module overview

The Alexa Auto SDK `Core` module is the heart of the SDK. The `Core` module acts as the foundation for all Auto SDK features by contributing the following essential elements to the SDK:
- **Defining the Auto SDK API for your application—** `Core` defines the `Engine` and `MessageBroker` components. Alongside the Alexa Auto Services Bridge (AASB) messages defined by each Auto SDK module, these components comprise the core API for your application to access the features of Auto SDK. To learn about the API, see [Understand the Auto SDK API](#understand-the-auto-sdk-api).

- **Providing an infrastructure to other modules—** `Core` provides the base infrastructure of the Engine, which each Auto SDK module extends to add module-specific features. `Core` also defines the common Engine services for logging, audio I/O, authorization, and device settings. Each module uses these Engine services to provide its own module-specific features. To learn about the primary core services, see [Understand the key core Engine services](#understand-the-key-core-engine-services).

- **Receiving core device statuses from your application—** `Core` includes AASB message interfaces for your application to report essential information such as device location and network connection status. The Engine uses this information to provide a reliable experience to the user. To learn about the core device status interfaces, see [Provide core device status](#provide-core-device-status). 

> **Important!**: If you are an Android developer, your application will use the [**Alexa Auto Client Service (AACS)**](../aacs/android/README.md) as its foundation. AACS implements much of the core Auto SDK setup, abstracting it from your application and exposing only a necessary subset of the Auto SDK API in an Android-specific way. Some of the information presented in this documentation and documentation for other SDK modules might not pertain to your application exactly as written for cases in which AACS provides the implementation or further abstracts it, so keep this in mind while reading. Use the module documentation to understand the underlying layers of Auto SDK, if interested, and to reference the Engine configuration and AASB message definitions for the features you do need to build into your application yourself. 


## Understand the Auto SDK API

The `Engine`, `MessageBroker`, and `AASB message interfaces` comprise the Auto SDK API. Your application uses these three components to build a complete Alexa client implementation for your vehicle. 

### The Engine

The Auto SDK [Engine](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1core_1_1_engine.html) is a system of components that provide the core implementation of all Auto SDK features. With respect to Alexa, your application's Alexa client stack uses the Engine as the layer that sets up the connection to Alexa, publishes device capabilities, sends Alexa events, sequences Alexa directives, and more.

Your application creates an instance of the Engine and uses a simple interface to [manage the Engine lifecycle](manage-the-engine-lifecycle-in-your-application) for the duration of the application run time. Aside from setting up the Engine, the primary responsibility of your application is to provide the platform-specific, customizable integration details that make Alexa and other core SDK features work for *your* vehicle, in particular. Platform-specific integration might include interacting with external libraries or the underlying software frameworks of your operating system to complete the Auto SDK client stack with deep integration into the applications on your system.

The Engine implements as much of the general functionality as possible; for integration details that it can't implement, the Engine delegates responsibility to your application via [AASB messages](#aasb-message-interfaces) published through the [MessageBroker](#messageBroker).

### MessageBroker

The [MessageBroker](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1core_1_1_message_broker.html) is the bridge in the Alexa Auto Services Bridge (AASB). MessageBroker provides a publish and subscribe API to the Engine and your application for communication with each other. In order to consume a message that the Engine publishes to your application, your application uses MessageBroker to subscribe to the message by specifying the message `topic` and `action` as well as a function for MessageBroker to invoke to deliver the message. Similarly, the Engine uses MessageBroker to subscribe to messages published by your application.

### AASB message interfaces

A typical Auto SDK module defines one or more AASB message interfaces. An interface groups logically related messages together with a `topic`. Within the `topic`, each interface has one or more `actions` to represent individual messages. I.e., a `topic` + `action` combination identifies a single message.

For instance, the `Alexa` module defines many interfaces related to standard Alexa features. The `Alexa` module `SpeechRecogizer` interface defines messages for your application to publish to the Engine when the user invokes Alexa. It also defines messages the Engine publishes to your application to convey key events in the user speech stream. 

For example, your application publishes a `SpeechRecognizer.StartCapture` message when the user taps the Alexa invocation button:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "12345",
        "messageDescription": {
            "topic": "SpeechRecognizer",
            "action": "StartCapture"
        }
    },
    "payload": {
        "initiator": "TAP_TO_TALK"
    }
}
```

The Engine subscribes to this message at startup time, so it is ready to consume the message when published by your application. In response to the message, the Engine sends the user speech audio to Alexa. Alexa processes the speech, and when she detects the user has finished speaking, the Engine publishes a `SpeechRecognizer.EndOfSpeechDetected` message to your application:

```json
{
    "header": {
        "version": "4.0",
        "messageType": "Publish",
        "id": "00876",
        "messageDescription": {
            "topic": "SpeechRecognizer",
            "action": "EndOfSpeechDetected"
        }
    }
}
```
Your application receives this message from MessageBroker if it subscribed to the `SpeechRecognizer` topic and `EndOfSpeechDetected` action.

MessageBroker uses AASB messages as serialized JSON strings; however, for convenience, Auto SDK provides C++ wrapper classes for each message that help with the serialization and deserialization. The Auto SDK build system generates these wrapper classes as part of the build. 


## Manage the Engine lifecycle in your application

To use Auto SDK features, your application must instantiate and manage the lifecycle of the Engine.

### Create the Engine

During the launch sequence of your application, create one instance of the Engine using the static function `aace::core::Engine::create()`:

```cpp
std::shared_ptr<aace::core::Engine> engine = aace::core::Engine::create();
```
A typical application creates the Engine once when the user turns on the vehicle ignition and uses the instance until the user turns off the vehicle ignition.

### Configure the Engine

After creating the Engine instance, configure it with the required Engine configurations. Engine configuration uses JSON serialized as strings, but you pass the configurations to the Engine in one or more `aace::core::config::EngineConfiguration` wrapper objects. Auto SDK provides two options to generate `EngineConfiguration` objects:
- Specify your Engine configuration in a [JSON file](#specify-configuration-in-a-file) and construct an `EngineConfiguration` from a path to the file
- Build the configuration [programatically](#specify-configuration-programatically) using one of the configuration factory functions.

You can choose either option or a combination of both. I.e., you can generate a single `EngineConfiguration` object that includes all configuration data for the Engine components you use, or you can break up the configuration data into logical sections and generate multiple `EngineConfiguration` objects. For example, you might generate one `EngineConfiguration` object for each module.

To configure the Engine, call the Engine's `configure()` function, passing in the `EngineConfiguration` object(s): 

* For a single `EngineConfiguration` object:

    ~~~
    engine->configure( config );
    ~~~
  
* For multiple `EngineConfiguration` objects:
    ~~~
      engine->configure( { xConfig, yConfig, zConfig } );
    ~~~
    replacing `xConfig, yConfig, zConfig` with logical names to identify the `EngineConfiguration` objects you generated.

> **Note**: For one Engine instance, you can call the `configure()` function only once, and you must call it before you subscribe to AASB messages with `MessageBroker` and before you start the Engine.

#### Specify configuration in a file

Auto SDK provides the [`ConfigurationFile` class](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1core_1_1config_1_1_configuration_file.html#a8ee151fa389256ca8e2df8626e76407e) that reads the configuration from a specified JSON file path and creates an `EngineConfiguration` object from that configuration:

```cpp
aace::core::config::ConfigurationFile::create( “</path/to/filename.json>” )
```
 
You can include all the configuration data in a single JSON file to create a single `EngineConfiguration` object. For example,

```cpp
auto config = aace::core::config::ConfigurationFile::create( “/opt/AAC/config/config.json” );
```

Alternatively, you can break the configuration data into multiple JSON files to create multiple `EngineConfiguration` objects. For example,

```cpp
auto coreConfig = aace::core::config::ConfigurationFile::create( “/opt/AAC/data/core-config.json” );
auto alexaConfig = aace::core::config::ConfigurationFile::create( “/opt/AAC/data/alexa-config.json” );
auto navigationConfig = aace::core::config::ConfigurationFile::create( “/opt/AAC/data/navigation-config.json” );
```

See documentation for individual module features to see the format of their respective JSON configuration. For example, `Core` outlines its required configurations in [Configure the Core module](#configure-the-core-module).

#### Specify configuration programatically

Each Auto SDK module that defines configuration provides a factory class with functions that return `EngineConfiguration` objects. The values a function puts in the configuration it creates correspond to the function parameters. For example, you can configure the `Alexa` module's `alertsCapabilityAgent` settings by using the [`AlexaConfiguration::createAlertsConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html#af7a3007198c8d8e47ce33edbf5c902a7) function:

```cpp
auto alertsConfig = aace::alexa::config::AlexaConfiguration::createAlertsConfig("</some/directory/path/for/databases>" );
```

###  Subscribe to AASB messages

After you configure the Engine, use MessageBroker to subscribe to any AASB interface messages that your application will handle. For example, the following code uses the AASB message wrapper classes for the `SpeechRecognizer` interface to subscribe to messages from the Engine with `SpeechRecognizer` topic:

```cpp
#include <AASB/Message/Alexa/SpeechRecognizer/StopCaptureMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/EndOfSpeechDetectedMessage.h>
#include <AASB/Message/Alexa/SpeechRecognizer/WakewordDetectedMessage.h>

void SpeechRecognizerHandler::subscribeToAASBMessages() {
    m_messageBroker->subscribe(
        [=](const std::string& message) { handleEndOfSpeechDetectedMessage(message); },
        EndOfSpeechDetectedMessage::topic(),
        EndOfSpeechDetectedMessage::action());

    m_messageBroker->subscribe(
        [=](const std::string& message) { handleWakewordDetectedMessage(message); },
        WakewordDetectedMessage::topic(),
        WakewordDetectedMessage::action());
}

void SpeechRecognizerHandler::handleEndOfSpeechDetectedMessage(const std::string& message) {
    // MessageBroker invokes this function when the Engine publishes a SpeechRecognizer.EndOfSpeechDetected message
    // Do something here!
    // Return quickly to avoid blocking MessageBroker's outgoing thread
}

void SpeechRecognizerHandler::handleWakewordDetectedMessage(const std::string& message) {
    // MessageBroker invokes this function when the Engine publishes a SpeechRecognizer.WakewordDetected message
    // Do something here!
    // Return quickly to avoid blocking MessageBroker's outgoing thread
}

```

> **Note**: For one Engine instance, you must subscribe to messages after configuring the Engine and before starting the Engine.

### Start the Engine

After configuring the Engine and subscribing to AASB messages, start the Engine by calling the Engine's `start()` function:

```cpp
engine->start();
```
Engine start initializes the internal Engine components for each Engine component your application uses. With respect to Alexa, it starts the connection to Alexa if there is an internet connection and an Alexa access token. Wait to publish messages to the Engine until after `start()` completes.

Your application can start the Engine more than once in its lifetime, if needed, by stopping the Engine and starting it again. However, you cannot start the Engine again after shutting it down.

### Stop the Engine

When your application needs to halt the operations of the Engine, stop the Engine by calling the Engine's `stop()` function:
```cpp
engine->stop();
```
With respect to Alexa, stopping the Engine tears down the Alexa connection.
Typically, Engine stop is a cleanup step before Engine shutdown. However, if you stopped the Engine at runtime and need to start it again, calling `start()` resumes Engine operations. With respect to Alexa, this includes reestablishing the Alexa connection.

### Shut down the Engine

When your application is ready to exit, shut down the Engine by calling the Engine's `shutdown()` function.

```cpp
engine->shutdown();
```

Make sure you also stop the Engine prior to shutting it down. After shutdown completes, you can safely dispose of the pointer to your Engine instance. You cannot use this instance of the Engine again.


## Configure the Core module

The `Core` module defines required and optional configuration objects that you include in the Engine configuration for your application. You can define the configuration objects in a file or construct them programatically with the relevant configuration factory functions.

### (Required) Vehicle info configuration

Your application must provide the `aace.vehicle` configuration specified below. The properties of the vehicle configuration are used for analytics.

```
{
  "aace.vehicle":
  {
     "info": {
         "make": {{STRING}},
         "model": {{STRING}},
         "year": {{STRING}},
         "trim": {{STRING}},
         "geography": {{STRING}},
         "version": {{STRING}},
         "os": {{STRING}},
         "arch": {{STRING}},
         "language": {{STRING}},
         "microphone": {{STRING}},
         "vehicleIdentifier": {{STRING}}
     }
  }
}
```
The following table describes the properties in the configuration:

| Property          | Type                | Required | Description                                                                                       | Example                          |
| ----------------- | ------------------- | -------- | ------------------------------------------------------------------------------------------------- | -------------------------------- |
| make              | string              | Yes      | The make of the vehicle                                                                           | —                                |
| model             | string              | Yes      | The model of the vehicle                                                                          | —                                |
| year              | integer as a string | Yes      | The model year of the vehicle. The value must be an integer in the range 1900-2100.               | "2019"                           |
| trim              | string              | No       | The trim package of the vehicle, identifying the vehicle's level of equipment or special features | "Sport"                          |
| geography         | string              | No       | The location of the vehicle                                                                       | "US",<br>"US-North",<br> "WA"    |
| version           | string              | No       | The client software version                                                                       | "4.0"                            |
| os                | string              | No       | The operating system used by the head unit                                                        | "AndroidOreo_8.1"                |
| arch              | string              | No       | The hardware architecture used by the head unit                                                   | "x86_64"                         |
| language          | string              | No       | The language or locale selected for Alexa by the vehicle owner                                    | "en-US",<br>"fr-CA"              |
| microphone        | string              | No       | The type and arrangement of microphone used in the vehicle                                        | "7 mic array, centrally mounted" |
| vehicleIdentifier | string              | Yes      | An identifier for the vehicle                                                                     | "1234abcd"                       |

Auto SDK provides the [`VehicleConfiguration::createVehicleInfoConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1vehicle_1_1config_1_1_vehicle_configuration.html) factory function to generate the configuration programatically. 

>**Important!** To pass the certification process, the `vehicleIdentifier` value you provide must NOT be the vehicle identification number (VIN).

### (Required) Storage configuration

Your application must provide the `aace.storage` configuration specified below. The Engine uses the configured path to create a database to persist data across device reboots.

```
{
    "aace.storage": {
        "localStoragePath": {{STRING}},
        "storageType": "sqlite"
    }
}
```

The following table describes the properties in the configuration:

| Property         | Type   | Required | Description                                                                              | Example                         |
| ---------------- | ------ | -------- | ---------------------------------------------------------------------------------------- | ------------------------------- |
| localStoragePath | string | Yes      | The path to a directory for the Engine to create a database, including the database name | "/opt/AAC/data/aace-storage.db" |
| storageType      | string | Yes      | The type of storage to use                                                               | "sqlite"                        |

>**Note:** This database is not the only one used by the Engine. Components in the `Alexa` module have similar configuration to store feature-specific settings and data. See [Configure the Alexa module](../alexa/README.md#configure-the-alexa-module) for details.


### (Optional) Logger configuration

By default, the Engine writes logs to the console. You can configure the Engine to save logs to a file with the `aace.logger` configuration:

```
{
  "aace.logger": {
    "sinks": [
        {
            "id": {{STRING}},
            "type": "aace.logger.sink.file",
            "config": {
                "path": {{STRING}},
                "prefix": {{STRING}},
                "maxSize": {{INTEGER}},
                "maxFiles": {{INTEGER},
                "append": {{BOOLEAN}}
            },
            "rules": [
                {
                    "level": {{STRING}}
                }
            ]
        }
    ]
}
```

The following table describes the properties in the configuration:

| Property                                         | Type             | Required | Description                                                                                                                                                                                                  | Example                 |
| ------------------------------------------------ | ---------------- | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------- |
| aace.logger.<br>sinks[i].<br>id                  | string           | Yes      | A unique identifier for the log sink.                                                                                                                                                                        | "debug-logs"            |
| aace.logger.<br>sinks[i].<br>type                | string           | Yes      | The type of the log sink. Use "aace.logger.sink.file" to write logs to a file.                                                                                                                               | "aace.logger.sink.file" |
| aace.logger.<br>sinks[i].<br>config.<br>path     | string           | Yes      | An absolute path to a directory where the Engine creates the log file.                                                                                                                                                      | "/opt/AAC/data"         |
| aace.logger.<br>sinks[i].<br>config.<br>prefix   | string           | Yes      | The prefix for the log file.                                                                                                                                                                                 | "auto-sdk"              |
| aace.logger.<br>sinks[i].<br>config.<br>maxSize  | integer          | Yes      | The maximum size of the log file in bytes.                                                                                                                                                                   | 5242880                 |
| aace.logger.<br>sinks[i].<br>config.<br>maxFiles | integer          | Yes      | The maximum number of logs files.                                                                                                                                                                            | 5                       |
| aace.logger.<br>sinks[i].<br>config.<br>append   | boolean          | Yes      | Use true to append logs to the existing file. Use false to overwrite the log files.                                                                                                                          | false                   |
| aace.logger.<br>sinks[i].<br>rules[j].<br>level  | enum string | Yes      | The log level used to filter logs written to the sink. <br><br>**Accepted values:**<ul><li>`"VERBOSE"`</li><li>`"INFO"`</li><li>`"WARN"`</li><li>`"ERROR"`</li><li>`"CRITICAL"`</li><li>`"METRIC"`</li></ul> | "VERBOSE"               |

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programatically with the C++ factory function</summary>
<br/>

Auto SDK provides the [`LoggerConfiguration::createFileSinkConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1logger_1_1config_1_1_logger_configuration.html#ab58d322f01f350cf87c89c41e8257037) factory function to generate the configuration programatically. 

```c++
#include "AACE/Logger/Logger.h"
#include "AACE/Logger/LoggerConfiguration.h"


auto fileSinkConfig = aace::logger::config::LoggerConfiguration::createFileSinkConfig(
    "debug-logs",
    aace::logger::LoggerEngineInterface::Level::VERBOSE,
    "opt/AAC/data",
    "auto-sdk",
    5242880,
    5,
    false);

engine->configure(
    {
        // ...other config objects...,
        fileSinkConfig
    }
);

```

</details>
<br/>

### (Optional) cURL configuration

The Auto SDK uses cURL for network connections. Your application can provide configuration to specify the cURL configuration:

```
{
    "aace.alexa": {
        "avsDeviceSDK": {
            "libcurlUtils" {
                "CURLOPT_CAPATH": {{STRING}},
                "CURLOPT_INTERFACE": {{STRING}},
                "CURLOPT_PROXY": {{STRING}}

            }
        }
    }
}
```

The following table describes the properties in the configuration:

| Property          | Type   | Required | Description                                                                                    | Example                 |
| ----------------- | ------ | -------- | ---------------------------------------------------------------------------------------------- | ----------------------- |
| CURLOPT_CAPATH    | string | Yes      | The path to the directory containing the CA certificates                                       | "/opt/AAC/certs"        |
| CURLOPT_INTERFACE | string | Yes      | The outgoing network interface. Can be a network interface name, an IP address, or a host name | "wlan0"                 |
| CURLOPT_PROXY     | string | No       | The address of the HTTP proxy                                                                  | "http://127.0.0.1:8888" |

> **Note:** If the HTTP proxy requires credentials in HTTP headers to authenticate a user agent, you can [specify the header with the `PropertyManager` interface](#runtime-properties) by using the `aace.network.httpProxyHeaders` property name. You can also change the network interface at runtime with the `aace.network.networkInterface` property name.

Auto SDK provides the [`AlexaConfiguration::createCurlConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/cpp/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html#a2012a3f45f54d16f5d8f260bb16c49c1) factory function to generate the configuration programatically. 

### (Optional) AASB and MessageBroker configuration

#### (Optional) Configure enabled interfaces

When you use a module, the Engine services of that module enable every interface defined in the module. This means that for every interface, if your application does not subscribe to the AASB messages of the interface, the Engine performs default handling for the messages you do not handle.

To disable this setting, provide the following `aace.messageBroker` configuration object in your Engine configuration:

```
{
    "aace.messageBroker": {
        "autoEnableInterfaces": false
    }
}
```

You can also change this enablement on a per-interface basis. The `enabled` setting tells the Engine service whether to enable the interface. If you don't want the Engine to provide a default handler for a particular interface, you can disable the interface using the following configuration:

```
{
    "aasb.<message_handler_engine_service_name>": {
        "<interface_name>": {
            "enabled": false
        }
    }
}
```
For example, the below configuration disables the `TemplateRuntime` interface from the `Alexa` module and the `LocationProvider` interface from `Core` module. 

```
{
    "aasb.alexa": {
        "TemplateRuntime": {
            "enabled": false
        }
    },
   "aasb.location": {
      "LocationProvider": {
         "enabled": false
      }
   }
}
```

#### (Optional) Configure the synchronous message timeout

All the messages published by the Engine through MessageBroker are asynchronous; however, certain messages require your application to respond with a special synchronous-style `Reply` message. Your application must publish the reply quickly because the Engine blocks its execution thread while waiting for the response, and the MessageBroker cannot dispatch more messages while waiting. The following is an example of the `Reply` message for the [`LocationProvider.GetLocation` message](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/LocationProvider/index.html#getlocation):
```
{
  "header": {
    "id": "4c4d13b6-6a8d-445b-931a-a3feb0878311",
    "messageType": "Reply",
    "version": "1.0",
    "messageDescription": {
      "replyToId": "23b578ed-6dc3-460a-998e-1647ba6cde42"
    }
  },
  "payload": {
    "location": {
      "latitude": 37.410,
      "longitude": -122.025
    }
  }
}
```
If your application does not publish the reply before the message timeout expires, the relevant Engine operation won't execute properly. The AASB message documentation for each interface specifies whether the interface requires any reply messages. 

The default timeout value for these messages is 500 milliseconds. In a busy system, the default timeout might not be long enough. You can configure this value by adding the optional field `defaultMessageTimeout` to the `aace.messageBroker` JSON object. The following example shows how to change the timeout to 1000 ms:
```
{
    "aace.messageBroker": {
        "defaultMessageTimeout": 1000
    }
}
```
> **Important!** Since increasing the timeout increases the Engine's message processing time, use this configuration carefully. Consult with your Amazon Solutions Architect (SA) as needed.

## Understand the key core Engine services

The `Core` module defines several Engine services for common functionality used by multiple Auto SDK modules. For example, the `Core`, `Alexa`, and `Alexa Comms` modules all require the application to play audio through platform-specific audio ouput channels. The `Core` module defines the core audio Engine service and corresponding `AudioOutput` AASB message interface, and all three modules use `AudioOutput` messages to request the application to play audio for their respective audio channels.

The following list describes the primary core Engine services provided by the `Core` module. Each service defines to one or more important AASB message interfaces that your application is required to use.

> **Note:** This is not a list of every Engine service in the `Core` module. For the most part, every AASB interface defined by `Core` also corresponds to a `Core` module Engine service that other modules might care about. The following lists the foundational core services that matter most to your application; the Engine cannot function without these.

### Authorization

The `Authorization` interface specifies messages for your application to initiate device authorization, terminate device authorization, or provide authorization data, such as Alexa access tokens, to the Engine. See [Alexa Auto SDK Authorization](./AUTHORIZATION.md) for more information.

### Audio input and output

The core audio Engine service provides a mechanism for Engine components of any module to open audio input and output channels in your application. Each component that requests an audio channel specifies its audio channel type so your application can provide different microphone and media player implementations for each channel. See [Alexa Auto SDK Audio Channels](./AUDIO.md) for more information.

### Runtime properties

Different Auto SDK modules define properties based on their supported features. For example, the `Alexa` module requires a locale setting to notify Alexa which language to use when interacting with the user. The `Core` module provides a mechanism for Engine services to register properties they manage and listen to changes in properties managed by other modules. The `PropertyManager` interface specifies messages for your application and the Engine to query and update these properties. See [Alexa Auto SDK Property Manager](./RUNTIME_PROPERTIES.md) for more information.


## Provide core device status

The `Core` module defines interfaces for your application to provide key "core" information about the status of the application or its runtime environment.

### Report location with LocationProvider

Sometimes the user asks Alexa a question that requires she know the location in order to answer properly. For example, a user in San Francisco, California might say *"Alexa, what's the weather?"*. This user probably wants to hear Alexa say something like *"The weather in San Francisco is sixty-five degrees and overcast..."* rather than something like *"I can't find your exact location right now..."*. Similarly, the user might say *"Alexa, take me to the nearest Whole Foods"* and wants Alexa to start navigation to a Whole Foods that is actually nearby.

To provide the user with accurate responses to local search commands, weather questions, and more, obtain the user's consent to share their location with Alexa and use the [`LocationProvider`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/LocationProvider/index.html) interface.

> **Note:** For Android applications, AACS provides a default implementation of `LocationProvider`. See the [AACS Default Implementation documentation](https://alexa.github.io/alexa-auto-sdk/aacs/android#default-platform-implementation) for more information.

Your application should subscribe to the [`LocationProvider.GetLocation`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/LocationProvider/index.html#getlocation) and [`LocationProvider.GetCountry`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/LocationProvider/index.html#getcountry) messages to provide location data, such as geocoordinates and vehicle operating country, when the Engine requests it. These messages are synchronous-style and require your application to send the corresponding reply messages right away. To avoid blocking the MessageBroker outgoing thread and delaying user requests to Alexa, your application should keep the location data in a cache that you update frequently. Pull the location from the cache when the Engine requests it.

The Engine won't publish the `GetLocation` message if it knows your application has lost access to the location data. Keep the Engine in sync with the state of your application's location provider availability by proactively publishing the [`LocationServiceAccessChanged`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/LocationProvider/index.html#locationserviceaccesschanged) message at startup and each time the state changes. For example, your application might publish this message with `access` set to `DISABLED` if the system revokes your application's access to location or if GPS turns off.

> **Note:** The Engine does not persist this state across device reboots. To ensure the Engine always knows the initial state of location availability, publish a `LocationServiceAccessChanged` message each time you start the Engine. This includes notifying the Engine that `access` is `ENABLED`.

<details markdown="1">
<summary>Click to expand or collapse C++ example code</summary>

```cpp
#include <AACE/Core/MessageBroker.h>

#include <AASB/Message/Location/LocationProvider/GetCountryMessage.h>
#include <AASB/Message/Location/LocationProvider/GetLocationMessage.h>
#include <AASB/Message/Location/LocationProvider/LocationServiceAccessChangedMessage.h>

class MyLocationProviderHandler {

    // Call before you start the Engine
    void MyLocationProviderHandler::subscribeToAASBMessages() {
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleGetCountryMessage(message); },
            GetCountryMessage::topic(),
            GetCountryMessage::action());
        m_messageBroker->subscribe(
            [=](const std::string& message) { handleGetLocationMessage(message); },
            GetLocationMessage::topic(),
            GetLocationMessage::action());
    }

    void MyLocationProviderHandler::handleGetCountryMessage(const std::string& message) {
        GetCountryMessage msg = json::parse(message);

        // Quickly publish the GetCountry reply message
        auto country = getCountryFromCache(); // implement this stub
        GetCountryMessageReply replyMsg;
        replyMsg.header.messageDescription.replyToId = msg.header.id;
        replyMsg.payload.country = country;
        m_messageBroker->publish(replyMsg.toString());
    }

    void MyLocationProviderHandler::handleGetLocationMessage(const std::string& message) {
        GetLocationMessage msg = json::parse(message);

        // Quickly publish the GetCountry reply message
        auto location = getLocationFromCache(); // implement this stub
        GetLocationMessageReply replyMsg;
        replyMsg.header.messageDescription.replyToId = msg.header.id;

        // parse "location" and populate the fields of the reply message
        aasb::message::location::locationProvider::Location replyLocation;
        replyLocation.latitude = ... ; // the latitude from "location";
        replyLocation.longitude =  ... ; // the longitude from "location";
        replyMsg.payload.location = replyLocation;
        m_messageBroker->publish(replyMsg.toString());
    }

    // Call when the application access to location data changes
    // and after starting the Engine
    void MyLocationProviderHandler::locationServiceAccessChanged(bool hasAccess) {
        LocationServiceAccessChangedMessage msg;
        if (hasAccess) {
             msg.payload.access = aasb::message::location::locationProvider::LocationServiceAccess::ENABLED;
        } else {
            msg.payload.access = aasb::message::location::locationProvider::LocationServiceAccess::DISABLED;
        }
        m_messageBroker->publish(msg.toString());
    }
}

```
</details>

### Report network status changes with NetworkInfoProvider

Your application should monitor the network connection and notify the Engine of changes in the status using the [`NetworkInfoProvider`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/NetworkInfoProvider/index.html) interface. The Engine uses this information to adjust its behavior, including tearing down the connection to Alexa cloud when your application reports that there is no internet connection. Although using `NetworkInfoProvider` is optional, you should use it so the Engine can avoid undesirable behavior; for instance, attempting to send events to Alexa when the lack of connectivity means the events are bound to fail.

>**Note:** You must use the `NetworkInfoProvider` interface if your application uses the Local Voice Control (LVC) extension.

> **Note:** For Android applications, AACS provides a default implementation of `NetworkInfoProvider`. See the [AACS Default Implementation documentation](https://alexa.github.io/alexa-auto-sdk/aacs/android#default-platform-implementation) for more information.

Various Engine components want the initial network status at startup so they can adapt their initial behavior accordingly. Your application should subscribe to the [`NetworkInfoProvider.GetNetworkStatus`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/NetworkInfoProvider/index.html#getnetworkstatus) and [`NetworkInfoProvider.GetWifiSignalStrength`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/NetworkInfoProvider/index.html#getwifisignalstrength) to answer the inital query from the Engine. These messages are synchronous-style and require your application to send the corresponding reply messages right away. 

At runtime, publish the [`NetworkInfoProvider.NetworkStatusChanged`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/NetworkInfoProvider/index.html#networkstatuschanged) message to notify the Engine of any status changes.


### Report data usage with DeviceUsage

Periodically publish the [`DeviceUsage.ReportNetworkDataUsage`](https://alexa.github.io/alexa-auto-sdk/docs/sdk-docs/modules/core/aasb-docs/DeviceUsage/index.html#reportnetworkdatausage) message (for example, at five minute intervals) to report network data usage to the Engine. If your application uses the `Device Client Metrics (DCM)` extension, the Engine tracks metrics with this information.

The `usage` field in the payload is a JSON object as a string. The format of the JSON is the following:

```
{
  "startTimeStamp" : {{LONG}},
  "endTimeStamp" : {{LONG}},
  "networkInterfaceType": "{{STRING}}",
  "dataPlanType" : "{{STRING}}",
  "bytesUsage" :{
       "rxBytes" : {{LONG}},
       "txBytes" : {{LONG}}
   }
}
```

The following table describes the properties in the JSON:

| Property               | Type   | Required | Description                                                                                                                                                                  | Example                                   |
| ---------------------- | ------ | -------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| startTimeStamp         | long   | Yes      | The starting timestamp in milliseconds for this network usage datapoint                                                                                         | —                                         |
| endTimeStamp           | long   | Yes     | The ending timestamp in milliseconds for this network usage datapoint                                                       | —                                         |
| networkInterfaceType   | string | Yes      | The network interface name over which the data is recorded                                                                                                                   | "WIFI",<br>"MOBILE"                       |
| dataPlanType           | string | No       | The type of data plan the device is subscribed to. This is an optional field and should be provided if your application uses the `AlexaConnectivity` module. | See [`AlexaConnectivity`](../connectivity/README.md) |
| bytesUsage<br>.rxBytes | long   | Yes      | The bytes received over the network interface                                                                                                                                | —                                         |
| bytesUsage<br>.txBytes | long   | Yes      | The bytes transmitted over the network interface                                                                                                                                | —                                         |
